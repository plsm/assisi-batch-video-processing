#include "experiment.hpp"

int main(int argc, char *argv[])
{
	Experiment experiment (argc, argv);
	experiment.process_data_plots_file ();
	return 0;
}
