import argparse

from assisivibe.common import util

parser = argparse.ArgumentParser (
    description = "create a HTML file with a report of video analysis",
    argument_default = None
)
parser.add_argument (
    '--same-colour-threshold', '-c',
    type = int,
    required = True,
    help = "threshold used to compare two pixel intensity values"
)
parser.add_argument (
    '--delta-frame', '-d',
    type = int,
    required = True,
    help = "how many frames apart were used to compute bee speed"
)
args = parser.parse_args ()

rows_data_plots = util.load_csv ("data-plots.csv", True)
rows_data_plots_groups = util.load_csv ("data-plots-groups.csv", True)
rows_data_plots_ROIs = util.load_csv ("data-plots-ROIs.csv", True)

filename = "report_DF={delta_frame}_SCT={same_colour_threshold}.html".format (
    same_colour_threshold = args.same_colour_threshold,
    delta_frame = args.delta_frame
)
fd = open (filename, "w")
fd.write ("""<html>
  <head>
    <meta charset="UTF-8">
    <title>Vibration Pattern Analysis Report</title>
  </head>
<body>""")

previous_group = None
for a_row_data_plots in rows_data_plots:
    current_group = int (a_row_data_plots [7])
    if previous_group != current_group:
        fd.write ("""
<h1>{title}</h1>
<p>
<img src="box-plots_average-bee-speed_SCT={same_colour_threshold}_DF={delta_frame}_group={group}_all-ROIs.png">
<p>""".format (
    title = [ar [0] for ar in rows_data_plots_groups if ar [1] == current_group][0],
    same_colour_threshold = args.same_colour_threshold,
    delta_frame = args.delta_frame,
    group = current_group
))
        for a_row_data_plots_ROIs in rows_data_plots_ROIs:
            fd.write ("""
<img src="box-plots_average-bee-speed_SCT={same_colour_threshold}_DF={delta_frame}_group={group}_ROI={ROI}.png">""".format (
    same_colour_threshold = args.same_colour_threshold,
    delta_frame = args.delta_frame,
    group = current_group,
    ROI = a_row_data_plots_ROIs [0]
    ))
        previous_group = current_group
    fd.write ("""
<h2>{title}</h2>
<p>
<img src="{path}/number-bees_SCT={same_colour_threshold}.png">
<img src="{path}/bee-speed_DF={delta_frame}_SCT={same_colour_threshold}.png">
<img src="{path}/features-_DF={delta_frame}_SCT={same_colour_threshold}.png">""".format (
        title = a_row_data_plots [1],
        path = a_row_data_plots [0],
        same_colour_threshold = args.same_colour_threshold,
        delta_frame = args.delta_frame
    ))
fd.write ("""
</body>
</html>""")
fd.close ()
