#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "parameters.hpp"

using namespace std;

static string verify_slash_at_end (const string &folder);

RunParameters::RunParameters (const string &frame_file_type, unsigned int number_ROIs, unsigned int delta_frame, unsigned int number_frames, unsigned same_colour_threshold):
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
	const char *frame_file_type = "png";
	unsigned int number_ROIs = 3;
	unsigned int same_colour_threshold = 15;
	unsigned int delta_frame = 2;
	unsigned int number_frames = 730;
	do {
		static struct option long_options[] = {
		        {"frame-file-type"       , required_argument, 0, 'f'},
		        {"number-ROIs"           , required_argument, 0, 'r'},
		        {"delta-frame"           , required_argument, 0, 'd'},
		        {"number-frames"         , required_argument, 0, 'n'},
		        {"same-colour-threshold" , required_argument, 0, 'c'},
		        {0,         0,                 0,  0 }
		};
		int c = getopt_long (argc, argv, "p:f:c:r:d:", long_options, 0);
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
	return RunParameters (frame_file_type, number_ROIs, delta_frame, number_frames, same_colour_threshold);
}

UserParameters::UserParameters (const string &folder, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2):
   folder (folder + verify_slash_at_end (folder)),
   x1 (x1),
   y1 (y1),
   x2 (x2),
   y2 (y2)
{
}

UserParameters UserParameters::parse (const string &csv_row)
{
	std::stringstream          lineStream (csv_row);
	std::string                cell;
	vector<string> cs;
	while (std::getline (lineStream, cell, ',')) {
	  cs.push_back (cell);
	}
	if (cs.size () != 5) {
		cerr << "The number of cells is different from 5!\n";
		exit (EXIT_FAILURE);
	}
	return UserParameters (cs [0], std::stoi (cs [1]), std::stoi (cs [2]), std::stoi (cs [3]), std::stoi (cs [4]));
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
