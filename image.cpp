#include <opencv2/imgproc/imgproc.hpp>

#include "image.hpp"
#include "histogram.hpp"

const unsigned int NUMBER_COLOUR_LEVELS = 256;

void compute_histogram (const cv::Mat &image, Histogram &histogram)
{
	// Quantize the saturation to 32 levels
	int sbins = NUMBER_COLOUR_LEVELS;
	int histSize[] = {sbins};
	// saturation varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, NUMBER_COLOUR_LEVELS };
	const float* ranges[] = { sranges };
	cv::MatND hist;
	// we compute the histogram from the 0-th
	int channels[] = {0};
	cv::calcHist (&image, 1, channels, cv::Mat (), // do not use mask
	          hist, 1, histSize, ranges,
	          true, // the histogram is uniform
	          false);
	for (unsigned int i = 0; i < NUMBER_COLOUR_LEVELS; i++) {
		histogram [i] = hist.at<float> (i);
	}
}
