#ifndef __PARAMETERS__
#define __PARAMETERS__

#include <stdio.h>
#include <string>
#include <opencv2/core/core.hpp>

#include "image.hpp"

/**
 * @brief The RunParameters class represents parameters used in an experiment
 * and in an analysis of the experiment results.
 */
class RunParameters
{
public:
	const std::string csv_filename;
	const std::string frame_file_type;
	const unsigned int number_ROIs;
	const unsigned int delta_frame;
	const unsigned int number_frames;
	const unsigned int same_colour_threshold;
	const unsigned int same_colour_level;
	RunParameters (const std::string csv_filename, const std::string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int number_frames, unsigned same_colour_threshold);
	static RunParameters parse (int argc, char *argv[]);
	template<typename A, typename B>
	inline void fold2_frames_ROIs (void (*func) (unsigned int, unsigned int, A, B), A acc1, B acc2) const
	{
		for (unsigned int index_frame = 0; index_frame < this->number_frames; index_frame++) {
			for (unsigned int index_mask = 0; index_mask < this->number_ROIs; index_mask++) {
				func (index_frame, index_mask, acc1, acc2);
			}
			fprintf (stdout, "\r      %d", index_frame + 1);
			fflush (stdout);
		}
		fprintf (stdout, "\n");
	}
	template<typename A, typename B, typename C>
	inline void fold3_frames_ROIs (void (*func) (unsigned int, unsigned int, A, B, C), A acc1, B acc2, C acc3) const
	{
		for (unsigned int index_frame = 0; index_frame < this->number_frames; index_frame++) {
			for (unsigned int index_mask = 0; index_mask < this->number_ROIs; index_mask++) {
				func (index_frame, index_mask, acc1, acc2, acc3);
			}
			fprintf (stdout, "\r      %d", index_frame + 1);
			fflush (stdout);
		}
		fprintf (stdout, "\n");
	}
};

/**
 * @brief The UserParameters class represents parameters and data specific to a
 * particular run of an experiment.
 *
 * This class contains attributes that represent the folder where the data is
 * stored and parameters used in specific analysis.
 *
 * This class contains methods the return the filenames of CSV files that contain
 * analysis results.
 *
 * Filenames of files that contain histograms have the following rule:
 *
 * histograms-frames_(raw|cropped)_(PRE-PROCESS)_(OPERATION)_(PARAMETERS).csv
 */
class UserParameters
{
	std::string rectangle () const
	{
		return
		      "_RECT=" +
		      std::to_string (this->x1) + "x" + std::to_string (this->y1) + "-" +
		      std::to_string (this->x2) + "x" + std::to_string (this->y2);
	}
	UserParameters (const RunParameters &run_parameters, const std::string &folder, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, bool use);
public:
	/**
	 * @brief folder Contains the folder where the data of a particular run of an
	 * experiment are stored.
	 *
	 * The data consists in a background image, video frames, CSV files with
	 * analysis results, properties of rectangular area used in specific analysis.
	 */
	const std::string folder;
	/**
	 * @brief x1 lowest horizontal coordinate of the rectangular area used in light calibration.
	 */
	const unsigned int x1;
	const unsigned int y1;
	const unsigned int x2;
	const unsigned int y2;
	const bool use;
	const Image background;
	const std::vector<Image> masks;
	static UserParameters *parse (const RunParameters &, const std::string &csv_row);
	inline std::string background_filename (const RunParameters &parameters) const
	{
		return folder + "background." + parameters.frame_file_type;
	}
	inline std::string frame_filename (const RunParameters &parameters, int index_frame) const
	{
		std::string result = this->folder + "frames-";
		char number[5];
		sprintf (number, "%04d", index_frame);
		result += number;
		result += ".";
		result += parameters.frame_file_type;
		return result;
	}
	inline std::string mask_filename (int index_mask) const
	{
		return this->folder + "Mask-" + std::to_string (index_mask + 1) + ".png";
	}
	/**
	 * @brief histogram_background_filename Returns the filename that contains the
	 * histogram of the background image.
	 *
	 * This file contains a single histogram that contains the count of the pixel
	 * colour intensities of to the background image.
	 *
	 * @return the filename that contains the histogram of the background image
	 */
	inline std::string histogram_background_filename () const
	{
		return this->folder + "histogram-background.csv";
	}
	/**
	 * @brief histogram_frames_all_filename Returns the filename that contains the
	 * histograms of all video frames.
	 *
	 * This file contains as many histograms as there are video frames. Each
	 * histogram contains the count of the pixel colour intensities of an entire
	 * video frame.
	 *
	 * @return the filename that contains the histograms of all video frames.
	 */
	inline std::string histogram_frames_all_filename () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_all"
		      ".csv";
	}
	/**
	 * @brief histogram_frames_rect Returns the filename that contains the
	 * histograms of all cropped video frames.
	 *
	 * This file contains as many histograms as there are video frames. Each
	 * histogram contains the count of the pixel colour intensities in rectangular
	 * area of a video frame. The coordinates of the rectangular area are stored
	 * in attributes x1, y1, x2 and y2.
	 *
	 * @return the filename that contains the histograms of all cropped video
	 * frames.
	 */
	inline std::string histogram_frames_rect () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_cropped" +
		      this->rectangle () +
		      ".csv";
	}
	inline std::string histogram_frames_light_calibrated_most_common_colour_method_PLSM_filename () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_light-calibrated-most-common-colour" +
		      this->rectangle () +
		      "_PLSM"
		      ".csv";
	}
	inline std::string histogram_frames_light_calibrated_most_common_colour_method_LC_filename () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_light-calibrated-most-common-colour" +
		      this->rectangle () +
		      "_LC"
		      ".csv";
	}
	inline std::string histograms_frames_masked_ROIs_bee_speed_raw_filename (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_masked-ROIs"
		      "_bee-speed"
		      "_raw"
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      ".csv";
	}
	inline std::string histograms_frames_masked_ROIs_bee_speed_histogram_equalisation_filename (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_masked-ROIs"
		      "_bee-speed"
		      "_histogram-equalisation-normal"
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      ".csv";
	}
	inline std::string histograms_frames_masked_ROIs_number_bees_raw_filename () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_masked-ROIs"
		      "_number-bees"
		      "_raw"
		      ".csv";
	}
	inline std::string histograms_frames_masked_ROIs_number_bees_histogram_equalisation_filename () const
	{
		return
		      this->folder +
		      "histograms-frames"
		      "_masked-ROIs"
		      "_number-bees"
		      "_histogram-equalisation-normal"
		      ".csv";
	}
	inline std::string features_pixel_count_difference_raw_filename (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      "_raw.csv";
	}
	inline std::string features_pixel_count_difference_histogram_equalization_filename (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      "_histogram-equalization.csv";
	}
	inline std::string features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_PLSM (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      "_light-calibration-most-common-colour" +
		      rectangle () +
		      "_PLSM" +
		      ".csv";
	}
	inline std::string features_pixel_count_difference_light_calibrated_most_common_colour_filename_method_LC (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "features-pixel-count-difference"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_DF=" + std::to_string (parameters.delta_frame) +
		      "_light-calibration-most-common-colour" +
		      rectangle () +
		      "_LC" +
		      ".csv";
	}
	inline std::string total_number_bees_in_all_ROIs_raw_filename (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "total-number-bees"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_raw"
		      ".csv";
	}
	inline std::string total_number_bees_in_all_ROIs_histogram_equalisation (const RunParameters &parameters) const
	{
		return
		      this->folder +
		      "total-number-bees"
		      "_SCT=" + std::to_string (parameters.same_colour_threshold) +
		      "_histogram-equalization"
		      ".csv";
	}
	inline std::string highest_colour_level_frames_rect_filename () const
	{
		return
		      this->folder +
		      "most-common-colour" +
		      rectangle () +
		      ".csv";
	}
	template<typename A, typename B>
	inline void fold2_frames (const RunParameters &parameters, void (*func) (const Image &, A, B), A acc1, B acc2) const
	{
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			std::string filename = this->frame_filename (parameters, index_frame);
			Image frame = read_image (filename);
			func (frame, acc1, acc2);
			fprintf (stdout, "\r      %d", index_frame);
			fflush (stdout);
		}
		fprintf (stdout, "\n");
	}
	template<typename A, typename B, typename C>
	inline void fold3_frames (const RunParameters &parameters, void (*func) (const Image &, A, B, C), A acc1, B acc2, C acc3) const
	{
		for (unsigned int index_frame = 1; index_frame <= parameters.number_frames; index_frame++) {
			std::string filename = this->frame_filename (parameters, index_frame);
			Image frame = read_image (filename);
			func (frame, acc1, acc2, acc3);
			fprintf (stdout, "\r      %d", index_frame);
			fflush (stdout);
		}
		fprintf (stdout, "\n");
	}
	template<typename A, typename B, typename C, typename D>
	inline void fold4_frames_I (const RunParameters &parameters, void (*func) (unsigned int, A, B, C, D), A acc1, B acc2, C acc3, D acc4) const
	{
		for (unsigned int index_frame = 0; index_frame < parameters.number_frames; index_frame++) {
			func (index_frame, acc1, acc2, acc3, acc4);
			fprintf (stdout, "\r      %d", index_frame + 1);
			fflush (stdout);
		}
		fprintf (stdout, "\n");
	}
	template<typename A, typename B, typename C, typename D, typename E>
	inline void fold5_ROIs_I (const RunParameters &parameters, void (*func) (unsigned int, A, B, C, D, E), A acc1, B acc2, C acc3, D acc4, E acc5) const
	{
		for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
			func (index_mask, acc1, acc2, acc3, acc4, acc5);
		}
	}
	template<typename A, typename B, typename C>
	inline void fold3_ROIs (const RunParameters &parameters, void (*func) (const Image &, A, B, C), A acc1, B acc2, C acc3) const
	{
		for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
			func (this->masks [index_mask], acc1, acc2, acc3);
		}
	}
	template<typename A, typename B>
	inline void fold2_ROIs (const RunParameters &parameters, void (*func) (const Image &, A, B), A acc1, B acc2) const
	{
		for (unsigned int index_mask = 0; index_mask < parameters.number_ROIs; index_mask++) {
			func (this->masks [index_mask], acc1, acc2);
		}
	}
	/**
	 * @brief rectangle_user return a string representing the rectangle to be
	 * analysed in a human readable way.
	 *
	 * @return a string representing the rectangle to be analysed in a human
	 * readable way.
	 */
	std::string rectangle_user () const
	{
		return
		    "(" + std::to_string (this->x1) + "," + std::to_string (this->y1) + ")-(" +
		    std::to_string (this->x2) + "," + std::to_string (this->y2) + ")";
	}
};

#endif
