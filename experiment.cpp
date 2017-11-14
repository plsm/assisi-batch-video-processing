#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include <opencv2/imgproc/imgproc.hpp>

#include "experiment.hpp"

using namespace std;

void compute_histograms_bee_speed_1 (const Image &current_frame, const Experiment *experiment, queue<Image> *cache, VectorHistograms *result);
void compute_histograms_bee_speed_2 (const Image &ROI_mask, bool enough_frames, Image *bee_speed, VectorHistograms *result);

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
		delete this->user;
		this->user = UserParameters::parse (this->run, csv_row);
		cout << "Processing folder " << this->user->folder << "...\n";
		VectorHistograms *bee_speed = this->compute_histograms_frames_masked_ROIs_bee_speed ();
		delete bee_speed;
	}
}

VectorHistograms *Experiment::compute_histograms_frames_masked_ROIs_bee_speed () const
{
	VectorHistograms *result;
	cout << "  Computing the histograms of bee speed images filtered with ROI masks...\n";
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

void compute_histograms_number_bees (const Image &current_frame_raw, const Experiment *experiment, const Image &background, VectorHistograms *result)
{
	static Histogram histogram;
	static Image number_bees;
	static Image current_frame_HE;
	cv::equalizeHist (current_frame_raw, current_frame_HE);
	cv::absdiff (background, current_frame_HE, number_bees);
	for (unsigned int index_mask = 0; index_mask < experiment->run.number_ROIs; index_mask++) {
		Image diff = number_bees & experiment->user->masks [index_mask];
		compute_histogram (diff, histogram);
		result->push_back (histogram);
	}
}

Image pre_process_histogram_equalisation (const Image &image)
{
	Image result;
	cv::equalizeHist (image, result);
	return result;
}
