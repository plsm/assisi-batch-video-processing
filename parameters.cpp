#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "parameters.hpp"

using namespace std;
namespace po = boost::program_options;

static string verify_slash_at_end (const string &folder);
static vector<cv::Mat> read_masks (const RunParameters &run_parameters, const UserParameters &parameters);

#define PO_CSV_FILENAME "csv-file"
#define PO_FRAME_FILE_TYPE "frame-file-type"
#define PO_NUMBER_ROIs "number-ROIs"
#define PO_DELTA_FRAME "delta-frame"
#define PO_NUMBER_FRAMES "number-frames"
#define PO_SAME_COLOUR_THRESHOLD "same-colour-threshold"

RunParameters::RunParameters (const po::variables_map &vm):
   csv_filename (vm [PO_CSV_FILENAME].as<string> ()),
   frame_file_type (vm [PO_FRAME_FILE_TYPE].as<string> ()),
   number_ROIs (vm [PO_NUMBER_ROIs].as<unsigned int> ()),
   delta_frame (vm [PO_DELTA_FRAME].as<unsigned int> ()),
   number_frames (vm [PO_NUMBER_FRAMES].as<unsigned int> ()),
   same_colour_threshold (vm [PO_SAME_COLOUR_THRESHOLD].as<unsigned int> ()),
   same_colour_level (round ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100.0))
{
}

po::options_description RunParameters::program_options ()
{
	po::options_description config ("Options that describe how the experiment was performed");
	config.add_options ()
	      (
	         PO_CSV_FILENAME",a",
	         po::value<string> ()
	         ->default_value ("data-analyse.csv")
	         ->value_name ("FILENAME"),
	         "CSV file with data of which folders to analyse"
	         )
	      (
	         PO_FRAME_FILE_TYPE",f",
	         po::value<string> ()
	         ->required ()
	         ->default_value ("png")
	         ->value_name ("EXTENSION"),
	         "file type of the frame"
	         )
	      (
	         PO_NUMBER_ROIs",r",
	         po::value<unsigned int> ()
	         ->default_value (3)
	         ->value_name ("N"),
	         "how many regions of interest exist"
	         )
	      (
	         PO_NUMBER_FRAMES",n",
	         po::value<unsigned int> ()
	         ->required ()
	         ->value_name ("N"),
	         "how many frames the videos have"
	         )
	      ;
	po::options_description analysis ("Options for the parameters that affect the analysis");
	analysis.add_options ()
	      (
	         PO_DELTA_FRAME",d",
	         po::value<unsigned int> ()
	         ->required ()
	         ->default_value (2)
	         ->value_name ("D"),
	         "how many frames apart are used when computing bee movement"
	         )
	      (
	         PO_SAME_COLOUR_THRESHOLD",c",
	         po::value<unsigned int> ()
	         ->required ()
	         ->value_name ("PERC"),
	         "threshold value used when deciding if two colour intensities are equal, percentage value"
	         )
	      ;
	po::options_description result;
	result.add (config);
	result.add (analysis);
	return result;
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
