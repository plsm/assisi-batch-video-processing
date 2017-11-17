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
	bool flag_features_number_bees_AND_bee_speed;
	bool flag_total_number_bees_in_ROIs_raw;
	bool flag_total_number_bees_in_ROIs_HE;
	void parse (int argc, char *argv[]);
	VectorHistograms *compute_histograms_frames_masked_ROIs_bee_speed () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees_raw () const;
	VectorSeries *compute_features_number_bees_bee_speed (const VectorHistograms &histograms_number_bees, const VectorHistograms &histograms_bee_speed) const;
	/**
	 * @brief compute_total_number_bees_in_ROIs_raw Computes the number of bees in
	 * all region of interest per video frame. This method uses the histograms of
	 * ROI masked number of bees images.
	 *
	 * @param histograms_number_bees The histograms of ROI masked number of bees
	 * images.
	 *
	 * @return A series with the number of bees in all region of interest per video
	 * frame.
	 */
	Series *compute_total_number_bees_in_ROIs_raw (const VectorHistograms &histograms_number_bees) const;
	Series *compute_total_number_bees_in_ROIs (const VectorSeries *features_number_bees_bee_speed);
};

#endif // EXPERIMENT_HPP
