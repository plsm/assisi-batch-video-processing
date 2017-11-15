#include <sys/stat.h>
#include <iostream>

#include "histogram.hpp"
#include "image.hpp"

using namespace std;

Histogram::Histogram ():
   vector<double> ((size_type) NUMBER_COLOUR_LEVELS, 0.0)
{
}

void Histogram::read (FILE *file)
{
	int value;
	if (fscanf (file, "%d", &value) != 1) {
		cerr << "Failed reading first value from histogram!\n";
		exit (EXIT_FAILURE);
	}
	(*this) [0] = value;
	for (unsigned int i = 1; i < NUMBER_COLOUR_LEVELS; i++) {
		if (fscanf (file, ",%d", &value) != 1) {
			cerr << "Failed reading value #" << i + 1 << " from histogram!\n";
			exit (EXIT_FAILURE);
		}
		(*this) [i] = value;
	}
}

void Histogram::write (FILE *file) const
{
	fprintf (file, "%d", (int) (*this) [0]);
	for (unsigned int i = 1; i < NUMBER_COLOUR_LEVELS; i++)
		fprintf (file, ",%d", (int) (*this) [i]);
}

int Histogram::most_common_colour () const
{
	int result = 0;
	double best = (*this) [0];
	for (unsigned int colour = 1; colour < NUMBER_COLOUR_LEVELS; colour++)
		if ((*this) [colour] > best) {
			best = (*this) [colour];
			result = colour;
		}
	return result;
}

void write_vector_histograms (const std::string &filename, const VectorHistograms *vh)
{
	FILE *f = fopen (filename.c_str (), "w");
	for (const Histogram &h : *vh) {
		h.write (f);
		fprintf (f, "\n");
	}
	fclose (f);
	chmod (filename.c_str (), S_IRUSR);
}

VectorHistograms *read_vector_histograms (const std::string &filename, size_t size)
{
	VectorHistograms *result = new VectorHistograms (size);
	FILE *f = fopen (filename.c_str (), "r");
	for (unsigned int index = 0; index < size; index++) {
		result->at (index).read (f);
	}
	fclose (f);
	return result;
}
