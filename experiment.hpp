#ifndef EXPERIMENT_HPP
#define EXPERIMENT_HPP

#include "parameters.hpp"
#include "histogram.hpp"

class Experiment
{
public:
	const RunParameters run;
	UserParameters *user;
	Experiment (int argc, char *argv[]);
	void process_data_plots_file ();
private:
	VectorHistograms *compute_histograms_frames_masked_ROIs_bee_speed () const;
};

#endif // EXPERIMENT_HPP
