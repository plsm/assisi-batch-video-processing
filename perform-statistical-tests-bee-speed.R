############################################################################## #
# Perform statistical tests on the average bee speed and total bee speed

library ("data.table")

main <- function (
  same_colour_threshold = 30,
  delta_frame = 2,
  image_preprocess = "histogram-equalization",
  filename_experiment = "data-plots.csv",
  number_frames = 730,
  parts_data_frame = data.frame (),
  ROIs = data.frame (),
  groups_data_frame = data.frame ()
) {
  process_video <- function (
    a_video
  ) {
    contents <- read.csv (
      file = sprintf (
        "%s/features-average-bee-speed_SCT=%d_DF=%d_%s.csv",
        a_video ["folder"],
        same_colour_threshold,
        delta_frame,
        image_preprocess
      ),
      header = FALSE,
      col.names = sprintf ("BSA%d", seq (1, number_ROIs))
    )
    contents$time <- c (1:number_frames)
    contents$part <- apply (
      X = contents,
      MARGIN = 1,
      FUN = compute_part
    )
    for (column_name in c ("repeat.", "group")) {
      contents [column_name] <- as.integer (a_video [column_name])
    }
    return (contents)
  }
  compute_part <- function (
    row
  ) {
    time <- as.integer (row ["time"])
    part <- which (parts_data_frame$start <= time & parts_data_frame$end >= time)
    if (length (part) == 0)
      return (NA)
    else
      return (part)
  }
  perform_statistical_test <- function (
    st
  ) {
    group_i <- as.integer (st ["group_i"])
    group_j <- as.integer (st ["group_j"])
    part_i <- as.integer (st ["part_i"])
    part_j <- as.integer (st ["part_j"])
    index_repeat <- as.integer (st ["repeat."])
    index_ROI_i <- as.integer (st ["ROI_i"])
    index_ROI_j <- as.integer (st ["ROI_j"])
    ks_result <- ks.test (
      x = get_data (index_group = group_i, index_part = part_i, index_ROI = index_ROI_i, index_repeat = index_repeat),
      y = get_data (index_group = group_j, index_part = part_j, index_ROI = index_ROI_j, index_repeat = index_repeat)
    )
    cat (paste (ks_result), "", file = log_file, sep = "\n")
    return (data.frame (
      group_i = group_i,
      group_j = group_j,
      part_i = part_i,
      part_j = part_j,
      ROI_i = index_ROI_i,
      ROI_j = index_ROI_j,
      repeat. = index_repeat,
      statistic = ks_result$statistic,
      p_value = ks_result$p.value,
      label = (if (ks_result$p.value >= 0.05) "n.s" else "⁎")
    ))
  }
  get_data <- function (
    index_group,
    index_part,
    index_ROI,
    index_repeat
  ) {
    result <- all_data
    if (!is.na(index_group  )) result <- subset (x = result, subset =    group == index_group )
    if (!is.na (index_part  )) result <- subset (x = result, subset =     part == index_part  )
    if (!is.na (index_repeat)) result <- subset (x = result, subset =  repeat. == index_repeat)
    result <- result [[sprintf ("BSA%s", index_ROI)]]
    return (result)
  }
  # read CSV files ####
  video_data_to_process <- read.csv (
    file = filename_experiment
  )
  if (nrow (groups_data_frame) == 0)
    groups_data_frame <- read.csv (
      file = "data-plots-groups.csv"
    )
  if (nrow (parts_data_frame) == 0)
    parts_data_frame <- read.csv (
      file = "data-plots-parts.csv"
    )
  if (nrow (ROIs) == 0)
    ROIs <- read.csv (
      file = "data-plots-ROIs.csv"
    )
  # initialise internal variables ####
  number_groups <- nrow (groups_data_frame)
  number_ROIs <- nrow (ROIs)
  ROIs$index <- c (1:number_ROIs)
  number_parts <- nrow (parts_data_frame)
  log_file = file ("log_statistical-tests.txt", "w")
  # initialise statistical tests ####
  statistical_tests_set_0 <- subset (
    x = video_data_to_process,
    select = c ("repeat.", "group")
  )
  statistical_tests_set_0$group_i <- statistical_tests_set_0$group
  statistical_tests_set_0$group_j <- statistical_tests_set_0$group
  statistical_tests_set_0 <- subset (
    x = statistical_tests_set_0,
    select = c ("repeat.", "group_i", "group_j")
  )
  print (statistical_tests_set_0)
  statistical_tests_set_1 <- merge (
    # x = data.frame (
    #   group_i = c (1:number_groups),
    #   group_j = c (1:number_groups)
    # ),
    x = statistical_tests_set_0,
    y = merge (
      x = data.frame (
        part_i = c (1:number_parts),
        part_j = c (1:number_parts)
      ),
      y = subset (
        x = merge (
          x = data.frame (
            ROI_i = c (1:number_ROIs)
          ),
          y = data.frame (
            ROI_j = c (1:number_ROIs)
          )
        ),
        subset = ROI_i < ROI_j
      )
    )
  )
  print (statistical_tests_set_1)
  statistical_tests_set_2 <- merge (
    x = subset (
      x = merge (
        x = data.frame (
          group_i = c (1:number_groups)
        ),
        y = data.frame (
          group_j = c (1:number_groups)
        )
      ),
      subset = group_i < group_j
    ),
    y = merge (
      x = data.frame (
        repeat. = NA,
        part_i = c (1:number_parts),
        part_j = c (1:number_parts)
      ),
      y = data.frame (
        ROI_i = c (1:number_ROIs),
        ROI_j = c (1:number_ROIs)
      )
    )
  )
  print (statistical_tests_set_2)
  all_statistical_tests <- rbind.data.frame (statistical_tests_set_1, statistical_tests_set_2)
  #all_statistical_tests <- statistical_tests_set_2
  print (all_statistical_tests)
  # read video data ####
  all_data <- rbindlist (
    apply (
      X = video_data_to_process,
      MARGIN = 1,
      FUN = process_video
    )
  )
  result <- rbindlist (apply (
    X = all_statistical_tests,
    MARGIN = 1,
    FUN = perform_statistical_test
  ))
  write.csv (
    file = "statistical-tests_2.csv",
    x = result,
    row.names = FALSE
  )
  close (log_file)
}
