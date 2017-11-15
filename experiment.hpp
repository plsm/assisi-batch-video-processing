#ifndef EXPERIMENT_HPP
#define EXPERIMENT_HPP

#include <vector>

#include "parameters.hpp"
#include "histogram.hpp"

typedef std::vector<int> Series;
typedef std::vector<Series> VectorSeries;

class Experiment
{
public:
	const RunParameters run;
	UserParameters *user;
	Experiment (int argc, char *argv[]);
	void process_data_plots_file ();
private:
	VectorHistograms *compute_histograms_frames_masked_ROIs_bee_speed () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees_raw () const;
	VectorSeries *compute_features_number_bees_bee_speed (const VectorHistograms &histograms_number_bees, const VectorHistograms &histograms_bee_speed) const;
	Series *compute_total_number_bees_in_ROIs (const VectorSeries *features_number_bees_bee_speed);
};

#endif // EXPERIMENT_HPP
