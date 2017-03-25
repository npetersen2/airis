#ifndef CLAS_H
#define CLAS_H

#include <tclap/CmdLine.h>

class CLAs {
public:
	CLAs(int argc, char **argv) {
		TCLAP::CmdLine cmd("Algoritmic trading using technical indicators with stock-specific optimized parameters obtained via machine learning. Written Fall 2016 (C) Nathan Petersen", 
					' ', "0.0");
		TCLAP::SwitchArg tbbSwitch("", "tbb", "use Intel TBB to speed up execution when available", false);
		TCLAP::SwitchArg verboseSwitch("", "verbose", "output more information to use for debugging", false);

		std::vector<std::string> allowed;
		allowed.push_back("run");
		allowed.push_back("opt");
		allowed.push_back("sig");
		TCLAP::ValuesConstraint<std::string> allowedVals(allowed);
		TCLAP::UnlabeledValueArg<std::string> actionArg("action", "specifies whether the program should run (using preoptimized parameters) or optimize",
									true, "action", &allowedVals);

		TCLAP::UnlabeledMultiArg<std::string> tickersArg("tickers", "specifies the tickers for which the program will execute",
									true, "tickers");
	
		cmd.add(tbbSwitch);
		cmd.add(verboseSwitch);
		cmd.add(actionArg);
		cmd.add(tickersArg);

		cmd.parse(argc, argv);

		action = actionArg.getValue();
		tickers = tickersArg.getValue();
		tbb = tbbSwitch.getValue();
		verbose = verboseSwitch.getValue();

		// store tickers as upper case
		for (auto &t: tickers) std::transform(t.begin(), t.end(), t.begin(), ::toupper);
	}

	std::vector<std::string> tickers;
	std::string action;
	bool tbb;
	bool verbose;

	void helpMessage(char *prog) {
		std::cout << "Usage: " << prog << " [OPTIONS] ACTION TICKER(S)..." << std::endl;
		std::cout << "                                                                   " << std::endl;
		std::cout << "Options:                                                           " << std::endl;
		std::cout << "    --tbb      use Intel TBB when available                        " << std::endl;
		std::cout << "    --verbose  output more information for debugging               " << std::endl;
		std::cout << "    --help     show this help message                              " << std::endl;
		std::cout << "                                                                   " << std::endl;
		std::cout << "Actions:                                                           " << std::endl;
		std::cout << "    opt        computes the optimized parameters for each algorithm" << std::endl;
		std::cout << "    run        computes buy/sell signal using all algorithms       " << std::endl;
		std::cout << "                                                                   " << std::endl;
		std::cout << "Example: " << prog << " --tbb opt AAPL GE FB" << std::endl;

		exit(1);
	}
};

#endif // CLAS_H
