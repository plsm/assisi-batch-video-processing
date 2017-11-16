#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <sys/stat.h>

#include <opencv2/imgproc/imgproc.hpp>

#include "experiment.hpp"

using namespace std;

void compute_histograms_bee_speed_1 (const Image &current_frame, const Experiment *experiment, queue<Image> *cache, VectorHistograms *result);
void compute_histograms_bee_speed_2 (const Image &ROI_mask, bool enough_frames, Image *bee_speed, VectorHistograms *result);

void compute_histograms_number_bees_1 (const Image &current_frame_raw, const Experiment *experiment, Image *background, VectorHistograms *result);
void compute_histograms_number_bees_raw_1 (const Image &current_frame_raw, const Experiment *experiment, VectorHistograms *result);
void compute_histograms_number_bees_2 (const Image &ROI_mask, Image *number_bees, VectorHistograms *result);

static void compute_features_number_bees_bee_speed_1 (unsigned int index_frame, const Experiment *experiment, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result);
static void compute_features_number_bees_bee_speed_2 (unsigned int index_ROI, unsigned int index_frame, const RunParameters *run, const VectorHistograms *histograms_number_bees, const VectorHistograms *histograms_bee_speed, VectorSeries *result);

static void compute_total_number_bees_in_ROIs_12 (unsigned int index_frame, unsigned int index_mask, const VectorSeries *features_number_bees_bee_speed, Series *result);
static void compute_total_number_bees_in_ROIs_12 (unsigned int index_frame, unsigned int index_mask, const RunParameters *run, const VectorHistograms *histograms_number_bees, Series *result);

static Series *read_series (const string &filename, size_t series_length);
static void write_series (const string &filename, const Series &s);

static VectorSeries *read_series (const string &filename, size_t number_series, size_t series_length);
static void write_series (const string &filename, const VectorSeries &vs);

static bool exists (const string &filename);

Experiment::Experiment (int argc, char *argv[]):
   run (RunParameters::parse (argc, argv)),
   user (NULL)
{

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
		cout << "Processing folder " << this->user->folder << "...\n";
		VectorHistograms *bee_speed = this->compute_histograms_frames_masked_ROIs_bee_speed ();
		VectorHistograms *number_bees = this->compute_histograms_frames_masked_ROIs_number_bees ();
		VectorHistograms *number_bees_raw = this->compute_histograms_frames_masked_ROIs_number_bees_raw ();
		VectorSeries *features = this->compute_features_number_bees_bee_speed (*number_bees, *bee_speed);
		Series *total_bees = this->compute_total_number_bees_in_ROIs (features);
		Series *total_bees_raw = this->compute_total_number_bees_in_ROIs_raw (*number_bees_raw);
		delete bee_speed;
		delete number_bees;
		delete number_bees_raw;
		delete features;
		delete total_bees;
		delete total_bees_raw;
		delete this->user;
	}
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

Series *Experiment::compute_total_number_bees_in_ROIs_raw (const VectorHistograms &histograms_number_bees) const
{
	Series *result;
	cout << "  Computing total number of bees in all ROIs using histograms of ROI filtered number of bees images - using raw frames.\n";
	const string filename = this->user->total_number_bees_in_all_ROIs_raw_filename (this->run);
	if (exists (filename)) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_series (filename, this->run.number_frames);
	}
	else {
		cout << "    Using histograms of ROI filtered number of bees images...\n";
		result = new Series (this->run.number_frames, 0);
		this->run.fold3_frames_ROIs (compute_total_number_bees_in_ROIs_12, &this->run, &histograms_number_bees, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_series (filename, *result);
	}
	return result;
}

Series *Experiment::compute_total_number_bees_in_ROIs (const VectorSeries *features_number_bees_bee_speed)
{
	Series *result;
	cout << "  Computing total number of bees in all ROIs...\n";
	string filename = this->user->total_number_bees_in_all_ROIs_histogram_equalisation (this->run);
	if (access (filename.c_str (), F_OK) == 0) {
		cout << "    Reading data from file " << filename << "...\n";
		result = read_series (filename, this->run.number_frames);
	}
	else {
		cout << "    Using number of bees and bee movement data...\n";
		result = new Series (this->run.number_frames, 0);
		this->run.fold2_frames_ROIs (compute_total_number_bees_in_ROIs_12, features_number_bees_bee_speed, result);
		cout << "    Writing data to file " << filename << "...\n";
		write_series (filename, *result);
	}
	return result;
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
		Image diff = *bee_speed & ROI_mask;
		compute_histogram (diff, histogram);
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
	Image diff = *number_bees & ROI_mask;
	compute_histogram (diff, histogram);
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

void compute_total_number_bees_in_ROIs_12 (unsigned int index_frame, unsigned int index_ROI, const RunParameters *run, const VectorHistograms *histograms_number_bees, Series *result)
{
	unsigned int index_histogram = index_frame * run->number_ROIs + index_ROI;
	int number_bees_value = 0;
	for (unsigned int i = run->same_colour_level; i < NUMBER_COLOUR_LEVELS; i++) {
		number_bees_value += histograms_number_bees->at (index_histogram).at (i);
	}
	result->at (index_frame) += number_bees_value;
}

void compute_total_number_bees_in_ROIs_12 (unsigned int index_frame, unsigned int index_mask, const VectorSeries *features_number_bees_bee_speed, Series *result)
{
	result->at (index_frame) += features_number_bees_bee_speed->at (2 * index_mask).at (index_frame);
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

static bool exists (const string &filename)
{
	return
	      (access (filename.c_str (), F_OK) == 0) &&
	      (access (filename.c_str (), W_OK) == -1);
}

Image pre_process_histogram_equalisation (const Image &image)
{
	Image result;
	cv::equalizeHist (image, result);
	return result;
}
