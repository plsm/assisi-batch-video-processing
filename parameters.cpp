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
#define PO_MASK_FILE_TYPE "mask-file-type"
#define PO_BACKGROUND_FILENAME "background-filename"
#define PO_NUMBER_ROIs "number-ROIs"
#define PO_DELTA_FRAME "delta-frame"
#define PO_NUMBER_FRAMES "number-frames"
#define PO_SAME_COLOUR_THRESHOLD "same-colour-threshold"
#define PO_MASK_NUMBER_STARTS_AT_0 "mask-number-starts-at-1"
#define PO_FRAME_FILENAME_PREFIX "frame-filename-prefix"
#define PO_SUBFOLDER_FRAMES "subfolder-frames"
#define PO_SUBFOLDER_BACKGROUND "subfolder-background"
#define PO_SUBFOLDER_MASK "subfolder-mask"


RunParameters::RunParameters (const po::variables_map &vm):
   csv_filename (vm [PO_CSV_FILENAME].as<string> ()),
   frame_file_type (vm [PO_FRAME_FILE_TYPE].as<string> ()),
   mask_file_type (vm [PO_MASK_FILE_TYPE].as<string> ()),
   background_filename (vm [PO_BACKGROUND_FILENAME].as<string> ()),
   number_ROIs (vm [PO_NUMBER_ROIs].as<unsigned int> ()),
   delta_frame (vm [PO_DELTA_FRAME].as<unsigned int> ()),
   number_frames (vm [PO_NUMBER_FRAMES].as<unsigned int> ()),
   same_colour_threshold (vm [PO_SAME_COLOUR_THRESHOLD].as<unsigned int> ()),
   same_colour_level (round ((NUMBER_COLOUR_LEVELS * same_colour_threshold) / 100.0)),
   mask_number_starts_at_0 (vm.count (PO_MASK_NUMBER_STARTS_AT_0) > 0),
   frame_filename_prefix (vm [PO_FRAME_FILENAME_PREFIX].as<string> ()),
   subfolder_frames (verify_slash_at_end (vm [PO_SUBFOLDER_FRAMES].as<string> ())),
   subfolder_background (verify_slash_at_end (vm [PO_SUBFOLDER_BACKGROUND].as<string> ())),
   subfolder_mask (verify_slash_at_end (vm [PO_SUBFOLDER_MASK].as<string> ()))
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
	po::options_description logistic ("Options for describing how the files with image data are organised");
	logistic.add_options ()
	      (
	         PO_SUBFOLDER_FRAMES,
	         po::value<string> ()
	         ->default_value ("")
	         ->value_name ("PATH"),
	         "folder where frames are stored (this is added to the folder of each video to analyse)"
	         )
	      (
	         PO_FRAME_FILENAME_PREFIX,
	         po::value<string> ()
	         ->default_value ("frames-")
	         ->value_name ("NAME"),
	         "prefix of the frames filename"
	         )
	      (
	         PO_FRAME_FILE_TYPE",f",
	         po::value<string> ()
	         ->required ()
	         ->default_value ("png")
	         ->value_name ("EXTENSION"),
	         "file type of the frames"
	         )
	      (
	         PO_SUBFOLDER_MASK,
	         po::value<string> ()
	         ->default_value ("")
	         ->value_name ("PATH"),
	         "folder where masks are stored (this is added to the folder of each video to analyse)"
	         )
	      (
	         PO_MASK_FILE_TYPE,
	         po::value<string> ()
	         ->default_value ("png")
	         ->value_name ("EXTENSION"),
	         "file type of the masks"
	         )
	      (
	         PO_MASK_NUMBER_STARTS_AT_0,
	         "mask numbes start at zero (by default they start at one)"
	         )
	      (
	         PO_SUBFOLDER_BACKGROUND,
	         po::value<string> ()
	         ->default_value ("")
	         ->value_name ("PATH"),
	         "folder where the background image is stored (this is added to the folder of each video to analyse)"
	         )
	      (
	         PO_BACKGROUND_FILENAME,
	         po::value<string> ()
	         ->default_value ("background.png")
	         ->value_name ("NAME"),
	         "file name of the background image"
	         )
	      ;
	po::options_description result;
	result.add (config);
	result.add (analysis);
	result.add (logistic);
	return result;
}

UserParameters::UserParameters (const RunParameters &run_parameters, const string &folder, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, bool use):
   folder (verify_slash_at_end (folder)),
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
		return folder;
	else
		return folder + "/";
}

static vector<Image> read_masks (const RunParameters &run_parameters, const UserParameters &user_parameters)
{
	vector<Image> result (run_parameters.number_ROIs);
	for (unsigned int index_mask = 0; index_mask < run_parameters.number_ROIs; index_mask++) {
		result [index_mask] = read_image (user_parameters.mask_filename (run_parameters, index_mask));
	}
	return result;
}
