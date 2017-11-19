#ifndef EXPERIMENT_HPP
#define EXPERIMENT_HPP

#include <vector>
#include <boost/program_options.hpp>

#include "parameters.hpp"
#include "histogram.hpp"

typedef std::vector<int> Series;
typedef std::vector<Series> VectorSeries;
typedef const Image * (*PreprocessImage) (const Image *);

class Experiment
{
public:
	const RunParameters run;
	UserParameters *user;
	Experiment (const boost::program_options::variables_map &vm);
	void process_data_plots_file ();
	static boost::program_options::options_description program_options ();
private:
	const bool flag_check_ROIs;
	const bool flag_histograms_frames_masked_ORed_ROIs_number_bees_raw;
	const bool flag_histograms_frames_masked_ORed_ROIs_number_bees;
	const bool flag_features_number_bees_AND_bee_speed;
	const bool flag_total_number_bees_in_ROIs_raw;
	const bool flag_total_number_bees_in_ROIs_HE;
	void check_ROIs () const;
	/**
	 * @brief compute_histograms_frames_masked_ORed_ROIs_number_bees
	 *
	 * Compute a mask M that is the resulting of ORing all the masks of the
	 * regions of interest.
	 *
	 * For each frame F compute an image D that is the absolute difference between
	 * frame F and the background image B. Afterwards compute an image I that is
	 * the result of ANDing images D and M. Finally, compute the histogram H of
	 * image I.
	 *
	 * @param preprocess_treatment Name of the preprocess applied to the
	 * background image and frames.
	 *
	 * @param func Function to be applied to the background image and frames.
	 *
	 * @param filename Filename where the histograms are saved.
	 *
	 * @return A vector with the above described histogram H.
	 */
	VectorHistograms *compute_histograms_frames_masked_ORed_ROIs_number_bees (const std::string &preprocess_treatment, PreprocessImage func, const std::string &filename) const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_bee_speed () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees () const;
	VectorHistograms *compute_histograms_frames_masked_ROIs_number_bees_raw () const;
	VectorSeries *compute_features_number_bees_bee_speed (const VectorHistograms &histograms_number_bees, const VectorHistograms &histograms_bee_speed) const;
	/**
	 * @brief compute_total_number_bees_in_ORed_ROIs Computes the number of bees in
	 * all region of interest per video frame. This method uses the histograms of
	 * ROI masked number of bees images.
	 *
	 * The histograms are computed on an image I that is
	 *
	 * I = absdiff (B', F') & (R1 | R2 | ...)
	 *
	 * where B' and F' are the pre-processed background image and frame,
	 * respectively, Ri are the image masks for region of interest i.
	 *
	 * @param preprocess_treatment A string describing how the background image
	 * and frames where pre-processed.
	 *
	 * @param histograms_number_bees The histograms of ROI masked number of bees
	 * images.
	 *
	 * @param filename The filename where the data is stored
	 */
	void compute_total_number_bees_in_ORed_ROIs (const std::string &preprocess_treatment, const VectorHistograms *histograms_number_bees, const std::string &filename) const;
};

#endif // EXPERIMENT_HPP
