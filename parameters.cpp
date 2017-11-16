#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "parameters.hpp"

using namespace std;

static string verify_slash_at_end (const string &folder);
static vector<cv::Mat> read_masks (const RunParameters &run_parameters, const UserParameters &parameters);

RunParameters::RunParameters (const string csv_filename, const string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int number_frames, unsigned same_colour_threshold):
   csv_filename (csv_filename),
   frame_file_type (frame_file_type),
   number_ROIs (number_ROIs),
   delta_frame (delta_frame),
   number_frames (number_frames),
   same_colour_threshold (same_colour_threshold),
   same_colour_level (round ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100.0))
{
}

RunParameters RunParameters::parse (int argc, char *argv[])
{
	bool ok = true;
	const char *csv_filename = "data-plots.csv";
	const char *frame_file_type = "png";
	unsigned int number_ROIs = 3;
	unsigned int same_colour_threshold = 15;
	unsigned int delta_frame = 2;
	unsigned int number_frames = 730;
	do {
		static struct option long_options[] = {
		   {"csv-file"       , required_argument, 0, 'a'},
		        {"frame-file-type"       , required_argument, 0, 'f'},
		        {"number-ROIs"           , required_argument, 0, 'r'},
		        {"delta-frame"           , required_argument, 0, 'd'},
		        {"number-frames"         , required_argument, 0, 'n'},
		        {"same-colour-threshold" , required_argument, 0, 'c'},
		        {0,         0,                 0,  0 }
		};
		int c = getopt_long (argc, argv, "a:f:r:d:n:c:", long_options, 0);
		switch (c) {
		case '?':
			break;
		case -1:
			ok = false;
			break;
		case ':':
			fprintf (stderr, "Missing argument\n");
			exit (EXIT_FAILURE);
			break;
		case 'a':
			csv_filename = optarg;
			break;
		case 'f':
			frame_file_type = optarg;
			break;
		case 'r':
			number_ROIs = (unsigned int) atoi (optarg);
			break;
		case 'd':
			delta_frame = (unsigned int) atoi (optarg);
			break;
		case 'n':
			number_frames = (unsigned int) atoi (optarg);
		case 'c':
			same_colour_threshold = (unsigned int) atoi (optarg);
			break;
		}
	} while (ok);
	return RunParameters (csv_filename, frame_file_type, number_ROIs, delta_frame, number_frames, same_colour_threshold);
}

UserParameters::UserParameters (const RunParameters &run_parameters, const string &folder, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, bool use):
   folder (folder + verify_slash_at_end (folder)),
   x1 (x1),
   y1 (y1),
   x2 (x2),
   y2 (y2),
   use (use),
   background (read_image (this->background_filename (run_parameters))),
   masks (use ? read_masks (run_parameters, *this) : std::vector<Image> ())
{
}

UserParameters *UserParameters::parse (const RunParameters &run_parameters, const string &csv_row)
{
	std::stringstream          lineStream (csv_row);
	std::string                cell;
	vector<string> cs;
	while (std::getline (lineStream, cell, ',')) {
		cs.push_back (cell);
	}
	if (cs.size () != 6) {
		cerr << "The number of cells is different from 5!\n";
		exit (EXIT_FAILURE);
	}
	std::string folder = cs [0];
	folder = folder.substr (1, folder.size () - 2);
	return new UserParameters (run_parameters, folder, std::stoi (cs [1]), std::stoi (cs [2]), std::stoi (cs [3]), std::stoi (cs [4]), cs [5] == "1" || cs [5] == "true");
}

static string verify_slash_at_end (const string &folder)
{
	if (folder.size () == 0)
		return "";
	else if (folder [folder.size () - 1] == '/')
		return "";
	else
		return "/";
}

static vector<Image> read_masks (const RunParameters &run_parameters, const UserParameters &user_parameters)
{
	vector<Image> result (run_parameters.number_ROIs);
	for (unsigned int index_mask = 0; index_mask < run_parameters.number_ROIs; index_mask++) {
		result [index_mask] = read_image (user_parameters.mask_filename (index_mask));
	}
	return result;
}
