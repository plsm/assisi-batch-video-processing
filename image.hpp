#ifndef __IMAGE__
#define __IMAGE__

#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

extern const unsigned int NUMBER_COLOUR_LEVELS;

typedef cv::Mat Image;

inline Image read_image (const std::string &filename)
{
	if (access (filename.c_str (), F_OK) != 0) {
		fprintf (stderr, "There is no such image: %s\n", filename.c_str ());
		exit (EXIT_FAILURE);
	}
	return cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
}

#endif
