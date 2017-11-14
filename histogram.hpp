#ifndef __HISTOGRAM__
#define __HISTOGRAM__

#include <stdio.h>
#include <vector>
#include <string>

class Histogram:
      public std::vector<double>
{
public:
	Histogram ();
	/**
	 * @brief read Read an histogram from the given file.  The data format is CSV.
	 * @param file
	 */
	void read (FILE *file);
	/**
	 * @brief write Write a histogram to the given file.  The data format is CSV.
	 * @param file
	 */
	void write (FILE *file) const;
	/**
	 * Return the most common colour in this histogram.
	 */
	int most_common_colour () const;
};

typedef std::vector<Histogram> VectorHistograms;

void write_vector_histograms (const std::string &filename, const VectorHistograms *vh);
VectorHistograms *read_vector_histograms (const std::string &filename, size_t size);

#endif
