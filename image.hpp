#ifndef __IMAGE__
#define __IMAGE__

#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

extern const unsigned int NUMBER_COLOUR_LEVELS;

typedef cv::Mat Image;

class Histogram;

inline Image read_image (const std::string &filename)
{
	if (access (filename.c_str (), F_OK) != 0) {
		fprintf (stderr, "There is no such image: %s\n", filename.c_str ());
		exit (EXIT_FAILURE);
	}
	return cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
}

inline const Image *preprocess_histogram_equalisation (const Image *image)
{
	static Image result;
	cv::equalizeHist (*image, result);
	return &result;
}

inline const Image *preprocess_raw (const Image *image)
{
	return image;
}

void compute_histogram (const Image &image, Histogram &histogram);

#endif
