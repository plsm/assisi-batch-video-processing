#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <limits>
#include <getopt.h>
#include <sys/stat.h>

#include <opencv2/imgproc/imgproc.hpp>

#include "experiment.hpp"

using namespace std;
namespace po = boost::program_options;

typedef std::vector<double> DoubleSeries;
typedef std::vector<DoubleSeries> VectorDoubleSeries;

void check_ROI_pair (unsigned int roi1_number, const Image &roi1_image, unsigned int roi2_number, const Image &roi2_image);

void compute_histograms_bee_speed_1 (const Image &current_frame, const Experiment *experiment, queue<Image> *cache, VectorHistograms *result);
void compute_histograms_bee_speed_2 (const Image &ROI_mask, bool enough_frames, Image *bee_speed, VectorHistograms *result);

void compute_histograms_number_bees_1 (const Image &current_frame_raw, const Experiment *experiment, Image *background, VectorHistograms *result);
void compute_histograms_number_bees_raw_1 (const Image &current_frame_raw, const Experiment *experiment, VectorHistograms *result);
void compute_histograms_number_bees_2 (const Image &ROI_mask, Image *number_bees, VectorHistograms *result);

static void compute_features_number_bees_bee_speed_1 (unsigned int index_frame, const Experiment *experiment, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result);
static void compute_features_number_bees_bee_speed_2 (unsigned int index_ROI, unsigned int index_frame, const RunParameters *run, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result);

static Series *read_series (const string &filename, size_t series_length);
static void write_series (const string &filename, const Series &s);

static VectorSeries *read_series (const string &filename, size_t number_series, size_t series_length);
static void write_series (const string &filename, const VectorSeries &vs);

static void write_series (const string &filename, const VectorDoubleSeries &vs);

static bool exists (const string &filename);

#define PO_CHECK_ROI "check-ROIs"
#define PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_RAW "histograms-frames-masked-ORed-ROIs-number-bees-raw"
#define PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_HE "histograms-frames-masked-ORed-ROIs-number-bees-HE"
#define PO_FEATURES_NUMBER_BEES_AND_BEE_SPEED "features-number-bees-AND-bee-speed"
#define PO_FEATURE_AVERAGE_BEE_SPEED "feature-average-bee-speed"
#define PO_FEATURE_TOTAL_BEE_ACCELERATION "feature-total-bee-acceleration"
#define PO_TOTAL_NUMBER_BEES_IN_ROIS_RAW "total-number-bees-in-ROIs-raw"
#define PO_TOTAL_NUMBER_BEES_IN_ROIS_HE "total-number-bees-in-ROIs-HE"

Experiment::Experiment (const po::variables_map &vm):
   run (vm),
   user (NULL),
   flag_check_ROIs (vm.count (PO_CHECK_ROI) > 0),
   flag_histograms_frames_masked_ORed_ROIs_number_bees_raw (vm.count (PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_RAW) > 0),
   flag_histograms_frames_masked_ORed_ROIs_number_bees (vm.count (PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_HE) > 0),
   flag_features_number_bees_AND_bee_speed (vm.count (PO_FEATURES_NUMBER_BEES_AND_BEE_SPEED) > 0),
   flag_feature_average_bee_speed (vm.count (PO_FEATURE_AVERAGE_BEE_SPEED ) > 0),
   flag_feature_total_bee_acceleration (vm.count (PO_FEATURE_TOTAL_BEE_ACCELERATION) > 0),
   flag_total_number_bees_in_ROIs_raw (vm.count (PO_TOTAL_NUMBER_BEES_IN_ROIS_RAW) > 0),
   flag_total_number_bees_in_ROIs_HE (vm.count (PO_TOTAL_NUMBER_BEES_IN_ROIS_HE) > 0)
{
}

po::options_description Experiment::program_options ()
{
	po::options_description result ("Options for what analysis and checks to perform");
	result.add_options ()
	      (
	         PO_CHECK_ROI,
	         "check the masks of the regions of interest"
	         )
	      (
	         PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_RAW,
	         "create a CSV file with histograms of images that are the result of applying a mask M to a number of bees image."
	         "Uses raw backgroung image and frames."
	         "The mask M is the result of ORing all the masks of the regions of interest."
	         )
	      (
	         PO_HISTOGRAMS_FRAMES_MASKED_ORED_ROIS_NUMBER_BEES_HE,
	         "create a CSV file with histograms of images that are result of applying a mask M to a number of bee image."
	         "Uses histogram equalization to pre-process the background image and the frames"
	         "The mask M is the result of ORing all the masks of the regions of interest."
	         )
	      (
	         PO_FEATURES_NUMBER_BEES_AND_BEE_SPEED,
	         "create a CSV file with number of bees and bee speed per region of interest "
	         "using histogram equalization to pre-process the background image and the frames"
	         )
	      (
	         PO_FEATURE_AVERAGE_BEE_SPEED,
	         "create a CSV file with average bee speed per region of interest "
	         "using image data that was subject to histogram equalization"
	         )
	      (
	         PO_FEATURE_TOTAL_BEE_ACCELERATION,
	         "create a CSV file with average bee acceleration per region of interest "
	         "using image data that was subject to histogram equalization")
	      (
	         PO_TOTAL_NUMBER_BEES_IN_ROIS_RAW,
	         "create a CSV file with the total number of bees in all regions of interest "
	         "using the raw background image and frames"
	         )
	      (
	         PO_TOTAL_NUMBER_BEES_IN_ROIS_HE,
	         "create a CSV file with the total number of bees in all regions of interest "
	         "using histogram equalization to pre-process the background image and the frames"
	         )
	;
	return result;
}

void Experiment::process_data_plots_file ()
{
	if (access (this->run.csv_filename.c_str (), F_OK) != 0) {
		cerr << "CSV file with data about which folders to analyse does NOT exist!\n";
		exit (EXIT_SUCCESS);
	}
	ifstream csv_stream (this->run.csv_filename);
	string header;
	std::getline (csv_stream, header);
	while (csv_stream) {
		string csv_row;
		std::getline (csv_stream, csv_row);
		this->user = UserParameters::parse (this->run, csv_row);
		if (!this->user->use)
			continue;
		cout << "Processing folder " << this->user->folder << "...\n";
		if (this->flag_check_ROIs)
			this->check_ROIs ();
		VectorHistograms *histograms_total_number_bees =
		      this->flag_total_number_bees_in_ROIs_HE ||
		      this->flag_histograms_frames_masked_ORed_ROIs_number_bees
		      ? this->compute_histograms_frames_masked_ORed_ROIs_number_bees (
		           "Using histogram equalization to preprocess background image and frames.",
		           preprocess_histogram_equalisation,
		           this->user->histograms_frames_masked_ORed_ROIs_number_bees_histogram_equalisation_filename ()
		           ) : NULL;
		VectorHistograms *histograms_total_number_bees_raw =
		      this->flag_total_number_bees_in_ROIs_raw ||
		      this->flag_histograms_frames_masked_ORed_ROIs_number_bees_raw
		      ? this->compute_histograms_frames_masked_ORed_ROIs_number_bees (
		           "Using raw background image and frames.",
		           preprocess_raw,
		           this->user->histograms_frames_masked_ORed_ROIs_number_bees_raw_filename ()
		           ) : NULL;
		VectorHistograms *bee_speed =
		      this->flag_feature_average_bee_speed ||
		      this->flag_features_number_bees_AND_bee_speed
		      ? this->compute_histograms_frames_masked_ROIs_bee_speed () : NULL;
		VectorHistograms *number_bees =
		      this->flag_feature_average_bee_speed ||
		      this->flag_features_number_bees_AND_bee_speed
		      ? this->compute_histograms_frames_masked_ROIs_number_bees () : NULL;
		VectorHistograms *number_bees_raw =
		      false
		      ? this->compute_histograms_frames_masked_ROIs_number_bees_raw () : NULL;
		VectorSeries *features =
		      this->flag_features_number_bees_AND_bee_speed ||
		      this->flag_feature_average_bee_speed ||
		      this->flag_feature_total_bee_acceleration ||
		      this->flag_total_number_bees_in_ROIs_HE
		      ? this->compute_features_number_bees_bee_speed (*number_bees, *bee_speed) : NULL;
		if (this->flag_feature_average_bee_speed)
			this->compute_feature_average_bee_speed (*features);
		if (this->flag_total_number_bees_in_ROIs_HE)
			this->compute_total_number_bees_in_ORed_ROIs (
		         "Background image and frames were subject to histogram equalization.",
		         histograms_total_number_bees,
		         this->user->total_number_bees_in_all_ROIs_histogram_equalisation (this->run));
		if (this->flag_total_number_bees_in_ROIs_raw)
			this->compute_total_number_bees_in_ORed_ROIs (
		         "Background image and frames were used as is.",
		         histograms_total_number_bees_raw,
		         this->user->total_number_bees_in_all_ROIs_raw_filename (this->run)
		         );
		if (this->flag_feature_total_bee_acceleration)
			this->compute_feature_total_bee_acceleration (*features);
		delete histograms_total_number_bees;
		delete histograms_total_number_bees_raw;
		delete bee_speed;
		delete number_bees;
		delete number_bees_raw;
		delete features;
		delete this->user;
	}
}

void Experiment::check_ROIs () const
{
	cout << "  Checking masks of regions of interest.\n";
	this->user->fold0_ROI_pairs (check_ROI_pair);
}

void compute_histograms_number_bees_ORed_ROI_masks_1 (
      const Image &current_frame_raw,
      const Image *preprocessed_background, PreprocessImage func, const Image *ORed_ROI_masks,
      VectorHistograms *result)
{
	static Image number_bees;
	static const Image *preprocessed_current_frame;
	preprocessed_current_frame = func (&current_frame_raw);
	cv::absdiff (*preprocessed_background, *preprocessed_current_frame, number_bees);
	static Histogram histogram;
#ifdef DEBUG
	cv::imshow ("pre-processed current frame", *preprocessed_current_frame);
	cv::imshow ("number of bees", number_bees);
	cv::imshow ("diff", diff);
	cv::waitKey (0);
#endif
	compute_histogram (number_bees, *ORed_ROI_masks, histogram);
	result->push_back (histogram);
}

VectorHistograms *Experiment::compute_histograms_frames_masked_ORed_ROIs_number_bees (
      const string &preprocess_treatment, PreprocessImage preprocess_func, const string &filename) const
{
	VectorHistograms *result;
	cout << "  Computing the histograms of number of bees images filtered with ORed ROIs mask. " << preprocess_treatment << "\n";
	if (exists (filename)) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_vector_histograms (filename, this->run.number_frames);
	}
	else {
		cout << "    Processing frames...\n";
		result = new VectorHistograms ();
		result->reserve (this->run.number_frames);
		const Image *preprocessed_background = preprocess_func (&this->user->background);
		Image ORed_ROI_masks;
		typedef void (*fold1_ROIs) (const Image &, Image *);
		fold1_ROIs func_or_ROIs = [] (const Image &ROI_mask, Image *_ORed_ROI_masks) {
			if (_ORed_ROI_masks->size ().width == 0)
				*_ORed_ROI_masks = ROI_mask;
			else
				*_ORed_ROI_masks = *_ORed_ROI_masks | ROI_mask;
		};
		this->user->fold1_ROIs (func_or_ROIs, &ORed_ROI_masks);
#ifdef DEBUG
		cv::imshow ("ORed masks", ORed_ROI_masks);
		cv::imshow ("pre-processed background", *preprocessed_background);
#endif
		this->user->fold4_frames (this->run, compute_histograms_number_bees_ORed_ROI_masks_1,
		                          preprocessed_background, preprocess_func, (const Image *) &ORed_ROI_masks, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_vector_histograms (filename, result);
	}
	return result;
}

VectorHistograms *Experiment::compute_histograms_frames_masked_ROIs_number_bees_raw () const
{
	VectorHistograms *result;
	cout << "  Computing the histograms of number of bees images filtered with ROI masks - images are not treated\n";
	string filename = this->user->histograms_frames_masked_ROIs_number_bees_raw_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_vector_histograms (filename, this->run.number_frames * this->run.number_ROIs);
	}
	else {
		cout << "    Processing frames...\n";
		result = new VectorHistograms ();
		result->reserve (this->run.number_frames * this->run.number_ROIs);
		this->user->fold2_frames (this->run, compute_histograms_number_bees_raw_1, this, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_vector_histograms (filename, result);
	}
	return result;
}

VectorHistograms *Experiment::compute_histograms_frames_masked_ROIs_bee_speed () const
{
	VectorHistograms *result;
	cout << "  Computing the histograms of bee movement images filtered with ROI masks...\n";
	string filename = this->user->histograms_frames_masked_ROIs_bee_speed_histogram_equalisation_filename (this->run);
	if (access (filename.c_str (), F_OK) == 0) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_vector_histograms (filename, this->run.number_frames * this->run.number_ROIs);
	}
	else {
		cout << "    Processing frames...\n";
		result = new VectorHistograms ();
		result->reserve (this->run.number_frames * this->run.number_ROIs);
		queue<Image> cache;
		this->user->fold3_frames (this->run, compute_histograms_bee_speed_1, this, &cache, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_vector_histograms (filename, result);
	}
	return result;
}

VectorHistograms *Experiment::compute_histograms_frames_masked_ROIs_number_bees () const
{
	VectorHistograms *result;
	cout << "  Computing the histograms of number of bees images filtered with ROI masks...\n";
	string filename = this->user->histograms_frames_masked_ROIs_number_bees_histogram_equalisation_filename ();
	if (access (filename.c_str (), F_OK) == 0) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_vector_histograms (filename, this->run.number_frames * this->run.number_ROIs);
	}
	else {
		cout << "    Processing frames...\n";
		result = new VectorHistograms ();
		result->reserve (this->run.number_frames * this->run.number_ROIs);
		Image background_HE;
		cv::equalizeHist (this->user->background, background_HE);
		this->user->fold3_frames (this->run, compute_histograms_number_bees_1, this, &background_HE, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_vector_histograms (filename, result);
	}
	return result;
}

VectorSeries *Experiment::compute_features_number_bees_bee_speed (const VectorHistograms &histograms_number_bees, const VectorHistograms &histograms_bee_speed) const
{
	VectorSeries *result;
	cout << "  Computing number of bees and bee speed per ROI...\n";
	string filename = this->user->features_pixel_count_difference_histogram_equalization_filename (this->run);
	if (access (filename.c_str (), F_OK) == 0) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_series (filename, 2 * this->run.number_ROIs, this->run.number_frames);
	}
	else {
		result = new VectorSeries (2 * this->run.number_ROIs);
		this->user->fold4_frames_I (this->run, compute_features_number_bees_bee_speed_1, this, &histograms_number_bees, &histograms_bee_speed, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_series (filename, *result);
	}
	return result;
}

void compute_average_bee_speed_12 (unsigned int index_frame, unsigned int index_ROI, const RunParameters *parameters, const VectorSeries *features_number_bees_bee_speed, VectorDoubleSeries *result)
{
	if (index_frame > parameters->delta_frame) {
		unsigned int index_number_bees = 2 * index_ROI;
		unsigned int index_bee_speed = 2 * index_ROI + 1;
		double number_bees_value = (*features_number_bees_bee_speed) [index_number_bees][index_frame] +
		      (*features_number_bees_bee_speed) [index_number_bees][index_frame - 1 - parameters->delta_frame];
		double bee_speed_value = (*features_number_bees_bee_speed) [index_bee_speed][index_frame];
		double value = number_bees_value == 0 ? 0 : bee_speed_value / number_bees_value;
		result->at (index_ROI).push_back (value);
	}
	else {
		result->at (index_ROI).push_back (std::numeric_limits<double>::quiet_NaN ());
	}
}

void Experiment::compute_feature_average_bee_speed (const VectorSeries &features_number_bees_bee_speed) const
{
	cout << "  Computing average bee speed.\n";
	string filename = this->user->features_average_bee_speed_histogram_equalization_filename (this->run);
	if (exists (filename)) {
		cout << "    File already exists, nothing to do.\n";
	}
	else {
		VectorDoubleSeries result (this->run.number_ROIs);
		this->run.fold3_frames_ROIs (compute_average_bee_speed_12, &this->run, &features_number_bees_bee_speed, &result);
		write_series (filename, result);
	}
}

void compute_total_bee_acceleration_12 (unsigned int index_frame, unsigned int index_ROI, const RunParameters *parameters, const VectorSeries *features_number_bees_bee_speed, VectorSeries *result)
{
	if (index_frame > parameters->delta_velocity) {
		unsigned int index_bee_speed = 2 * index_ROI + 1;
		int bee_speed_value_now = (*features_number_bees_bee_speed) [index_bee_speed][index_frame];
		int bee_speed_value_then = (*features_number_bees_bee_speed) [index_bee_speed][index_frame - 1 - parameters->delta_velocity];
		int value = bee_speed_value_now - bee_speed_value_then;
		result->at (index_ROI).push_back (value);
	}
	else {
		result->at (index_ROI).push_back (std::numeric_limits<int>::quiet_NaN ());
	}
}

void Experiment::compute_feature_total_bee_acceleration (const VectorSeries &features_number_bees_bee_speed) const
{
	cout << "  Computing total bee acceleration.\n";
	string filename = this->user->features_total_bee_acceleration_histogram_equalization_filename (this->run);
	if (exists (filename)) {
		cout << "    File already exists, nothing to do.\n";
	}
	else {
		VectorSeries result (this->run.number_ROIs);
		this->run.fold3_frames_ROIs (compute_total_bee_acceleration_12, &this->run, &features_number_bees_bee_speed, &result);
		cout << "    Writing data to file " << filename << '\n';
		write_series (filename, result);
	}
}

/**
 * @brief compute_total_number_bees_in_ORed_ROIs_12 Computes how many bees are
 * in all the ROIS.
 *
 * Uses the histograms of images I. Each image is computed has given by the
 * following formula:
 *
 * I = absdiff (F, B) & (R1 | R2 | ... )
 *
 * where F is a frame, B is the background image, Ri are the ROIs masks. F and
 * B are pre-processed with one of the preprocess_X functions in module image.
 *
 * @param index_frame
 * @param run
 * @param histograms_number_bees
 * @param result
 */
void compute_total_number_bees_in_ORed_ROIs_12 (unsigned int index_frame, const RunParameters *run, const VectorHistograms *histograms_number_bees, Series *result)
{
	for (unsigned int i = run->same_colour_level; i < NUMBER_COLOUR_LEVELS; i++) {
		result->at (index_frame) += histograms_number_bees->at (index_frame).at (i);
	}
}

void Experiment::compute_total_number_bees_in_ORed_ROIs (const string &preprocess_treatment, const VectorHistograms *histograms_number_bees, const string &filename) const
{
	cout << "  Computing total number of bees in all ROIs. " << preprocess_treatment << "\n";
	if (exists (filename)) {
		cout << "    File already exists, nothing to do.\n";
	}
	else {
		cout << "    Using histograms of number bees images...\n";
		Series result (this->run.number_frames, 0);
		this->run.fold3_frames (compute_total_number_bees_in_ORed_ROIs_12, &this->run, histograms_number_bees, &result);
		cout << "    Writing data to file " << filename << "...\n";
		write_series (filename, result);
	}
}

void check_ROI_pair (unsigned int roi1_number, const Image &roi1_image, unsigned int roi2_number, const Image &roi2_image)
{
	Image common = roi1_image & roi2_image;
	static Histogram histogram;
	compute_histogram (common, histogram);
	if (histogram.at (NUMBER_COLOUR_LEVELS - 1) > 0)
		cout
		      << "    ROIs " << roi1_number
		      << " and " << roi2_number
		      << " have " << histogram.at (NUMBER_COLOUR_LEVELS - 1) << " pixels in common\n";
}

void compute_histograms_bee_speed_1 (const Image &current_frame_raw, const Experiment *experiment, queue<Image> *cache, VectorHistograms *result)
{
	static Image bee_speed;
	Image current_frame_HE;
	cv::equalizeHist (current_frame_raw, current_frame_HE);
	bool enough_frames = cache->size () > experiment->run.delta_frame;
	if (enough_frames) {
		cv::Mat previous_frame = cache->front ();
		cache->pop ();
		cv::absdiff (previous_frame, current_frame_HE, bee_speed);
	}
	experiment->user->fold3_ROIs (experiment->run, compute_histograms_bee_speed_2, enough_frames, (Image*) &bee_speed, result);
	cache->push (current_frame_HE);
}

void compute_histograms_bee_speed_2 (const Image &ROI_mask, bool enough_frames, Image *bee_speed, VectorHistograms *result)
{
	static Histogram histogram;
	if (enough_frames) {
		compute_histogram (*bee_speed, ROI_mask, histogram);
	}
	else {
		histogram.assign (NUMBER_COLOUR_LEVELS, -1);
	}
	result->push_back (histogram);
}

void compute_histograms_number_bees_1 (const Image &current_frame_raw, const Experiment *experiment, Image *background_HE, VectorHistograms *result)
{
	static Image number_bees;
	static Image current_frame_HE;
	cv::equalizeHist (current_frame_raw, current_frame_HE);
	cv::absdiff (*background_HE, current_frame_HE, number_bees);
	experiment->user->fold2_ROIs (experiment->run, compute_histograms_number_bees_2, &number_bees, result);
}

void compute_histograms_number_bees_raw_1 (const Image &current_frame_raw, const Experiment *experiment, VectorHistograms *result)
{
	static Image number_bees;
	cv::absdiff (experiment->user->background, current_frame_raw, number_bees);
	experiment->user->fold2_ROIs (experiment->run, compute_histograms_number_bees_2, &number_bees, result);
}

void compute_histograms_number_bees_2 (const Image &ROI_mask, Image *number_bees, VectorHistograms *result)
{
	static Histogram histogram;
	compute_histogram (*number_bees, ROI_mask, histogram);
	result->push_back (histogram);
}

void compute_features_number_bees_bee_speed_1 (unsigned int index_frame, const Experiment *experiment, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result)
{
	experiment->user->fold5_ROIs_I (experiment->run, compute_features_number_bees_bee_speed_2, index_frame, &experiment->run, histograms_number_bees, histograms_bee_speed, result);
}

void compute_features_number_bees_bee_speed_2 (unsigned int index_ROI, unsigned int index_frame, const RunParameters *run, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result)
{
	unsigned int index_number_bees = 2 * index_ROI;
	unsigned int index_bee_speed = 2 * index_ROI + 1;
	unsigned int index_histogram = index_frame * run->number_ROIs + index_ROI;
	int number_bees_value = 0;
	for (unsigned int i = run->same_colour_level; i < NUMBER_COLOUR_LEVELS; i++) {
		number_bees_value += histograms_number_bees->at (index_histogram).at (i);
	}
	int bee_speed_value;
	if (histograms_bee_speed->at (index_histogram).at (0) == -1)
		bee_speed_value = -1;
	else {
		bee_speed_value = 0;
		for (unsigned int i = run->same_colour_level; i < NUMBER_COLOUR_LEVELS; i++) {
			bee_speed_value += histograms_bee_speed->at (index_histogram).at (i);
		}
	}
	result->at (index_number_bees).push_back (number_bees_value);
	result->at (index_bee_speed).push_back (bee_speed_value);
}

Series *read_series (const string &filename, size_t series_length)
{
	Series *result = new Series (series_length);
	FILE *f = fopen (filename.c_str (), "r");
	for (unsigned int index = 0; index < series_length; index++) {
		fscanf (f, "%d", &((*result) [index]));
	}
	fclose (f);
	return result;
}

void write_series (const string &filename, const Series &s)
{
	FILE *f = fopen (filename.c_str (), "w");
	for (int value : s) {
		fprintf (f, "%d\n", value);
	}
	fclose (f);
	chmod (filename.c_str (), S_IRUSR);
}

VectorSeries *read_series (const string &filename, size_t number_series, size_t series_length)
{
	VectorSeries *result = new VectorSeries (number_series);
	FILE *f = fopen (filename.c_str (), "r");
	for (unsigned int index = 0; index < series_length; index++) {
		for (size_t series = 0; series < number_series; series++) {
			int value;
			if (series == 0) {
				if (fscanf (f, "%d", &value) != 1) {
					cerr << "Failed reading value #" << index * number_series + series + 1 << " from file " << filename << "!\n";
					exit (EXIT_FAILURE);
				}
			}
			else {
				if (fscanf (f, ",%d", &value) != 1) {
					cerr << "Failed reading value #" << index * number_series + series + 1 << " from file " << filename << "!\n";
					exit (EXIT_FAILURE);
				}
			}
			result->at (series).push_back (value);
		}
	}
	fclose (f);
	return result;
}

static void write_series (const string &filename, const VectorSeries &vs)
{
	FILE *f = fopen (filename.c_str (), "w");
	size_t number_series = vs.size ();
	size_t series_length = vs [0].size ();
	for (unsigned int index = 0; index < series_length; index++) {
		for (size_t series = 0; series < number_series; series++) {
			if (series > 0)
				fprintf (f, ",");
			fprintf (f, "%d", vs [series][index]);
		}
		fprintf (f, "\n");
	}
	fclose (f);
	chmod (filename.c_str (), S_IRUSR);
}

static void write_series (const string &filename, const VectorDoubleSeries &vs)
{
	FILE *f = fopen (filename.c_str (), "w");
	size_t number_series = vs.size ();
	size_t series_length = vs [0].size ();
	for (unsigned int index = 0; index < series_length; index++) {
		for (size_t series = 0; series < number_series; series++) {
			if (series > 0)
				fprintf (f, ",");
			if (!isnan (vs [series][index]))
				fprintf (f, "%f", vs [series][index]);
		}
		fprintf (f, "\n");
	}
	fclose (f);
	chmod (filename.c_str (), S_IRUSR);
}

static bool exists (const string &filename)
{
	return
	      (access (filename.c_str (), F_OK) == 0) &&
	      (access (filename.c_str (), W_OK) == -1);
}

