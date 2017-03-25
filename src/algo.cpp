#include "algo.h"

void outputHumanTime(std::ostream &os, double seconds);

void Algo::optimize(const CLAs &args, ParameterStore &paramStore) {
	std::cout << "Optimizing " << humanName << " algorithm for $" << ticker << "..." << std::endl; 

	populateVariables();
	tbb::concurrent_vector<std::pair<Variables, EvaluationResult>> results;

	if (args.tbb) {
		int var_size = vars.unsafe_size();
		tbb::parallel_for(0, var_size, 1, [&](int i) {
			Variables v;
			if (vars.try_pop(v)) {
				results.push_back({v, evaluate(v, args)});
			}
		});
	} else {
		int i = 0;

		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();

		int vars_size = vars.unsafe_size();
		Variables v;
		while (vars.try_pop(v)) {
			results.push_back({v, evaluate(v, args)});

			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end-start;
			double seconds = elapsed_seconds.count();
			double remaining = ((1 / ((double)i / vars_size)) * seconds) - seconds;

			std::cout << std::fixed << std::setprecision(2) << "\r" << ((double)i / vars_size) * 100 << "\% done, ";
			outputHumanTime(std::cout, seconds);
			std::cout << " elapsed, ";
			outputHumanTime(std::cout, remaining);
			std::cout << " remaining       " << std::flush;

			std::cout << std::defaultfloat << std::setprecision(6);
			i++;
		}

		std::cout << std::endl;
	}

	writeResults(results, paramStore);
}

/* @brief evaluates the algo by running it over historical data
 *        and averaging the percent returns
 *
 * @return the percent gain from historical data (0=no gain, 1=100% gains)
 */
EvaluationResult Algo::evaluate(const Variables &vars, const CLAs &args) {
	static const double PERCENT_TO_KEEP_LIQUID = .25;
	static const double INITIAL_MONEY = 100000;
	
	int numStocks = 0;
	double lastBuyPrice = 0;
	double avgBuyInPrice = 0;
	double cash = INITIAL_MONEY;

	int numTransactions = 0;
	int numSells = 0;
	int daysInvested = 0;
	std::vector<int> holdPeriods;
	int currHoldPeriod = 0;

	for (size_t todayIndex = 0; todayIndex < slices.size(); todayIndex++) {
		int result = run(todayIndex, vars);

		double currPrice = slices[todayIndex].second.open;
		DateTime dt = slices[todayIndex].first;
	
		if (result == 100) {
			// SELL
			if (numStocks > 0) {
				if (args.verbose) {
					std::cout << "SELL: on " << dt << 
							", sold " << numStocks  << " share(s) for (" << 
							numStocks << "*" << currPrice << ") = " << 
							numStocks*currPrice  << std::endl;
					std::cout << std::endl; 
				}

				cash += (numStocks * currPrice);

				numStocks = 0;
				lastBuyPrice = 0;
				numSells++;
				numTransactions++;

				holdPeriods.push_back(currHoldPeriod);
				currHoldPeriod = 0;
				avgBuyInPrice = 0;
			}

		} else if (result == -100) {
			// BUY
			if (lastBuyPrice * .90 >= currPrice || lastBuyPrice == 0) {
				int numToBuy = (cash * (1 - PERCENT_TO_KEEP_LIQUID)) / currPrice;

				if (args.verbose) {
					std::cout << "BUY: on " << dt << 
							", bought " << numToBuy  << " share(s) for (" << 
							numToBuy << "*" << currPrice << ") = " << 
							numToBuy*currPrice << std::endl;  
				}

				avgBuyInPrice = ((avgBuyInPrice * numStocks) + (numToBuy * currPrice)) / (numStocks + numToBuy);

				numStocks += numToBuy;
				cash -= (currPrice * numToBuy);
				lastBuyPrice = currPrice;
				numTransactions++;
			}

		} else {
			// HOLD

			// if (args.verbose) {
			//	std::cout << "HOLD" << std::endl;
			// }
		}

		if (numStocks > 0) {
			daysInvested++;
			currHoldPeriod++;
		}
	}

	if (numStocks > 0) {
		// still own stocks after simulation... pretend we never bought them
		cash += (numStocks * avgBuyInPrice); // sell them back for the avg cost of buying them all
		numStocks = 0;
	}


	double percentMade = (cash - INITIAL_MONEY) / INITIAL_MONEY * 100;
	auto minMax = std::minmax_element(holdPeriods.begin(), holdPeriods.end());

//	if (std::isnan(percentMade)) {
//		std::cout << "NAN" << std::endl;
//		std::cout << avgBuyInPrice << std::endl;
//	}

	EvaluationResult r;
	r.totalPercentGain = percentMade;
	r.avgPercentGainPerTotalTime = daysInvested == 0 ? 0 : percentMade / daysInvested;
	r.avgPercentGainPerHold = numSells == 0 ? 0 : percentMade / numSells;
	r.numTransactions = numTransactions;
	r.minHoldPeriod = holdPeriods.size() == 0 ? 0 : *(minMax.first);
	r.maxHoldPeriod = holdPeriods.size() == 0 ? 0 : *(minMax.second);
	r.avgHoldPeriod = holdPeriods.size() == 0 ? 0 : std::accumulate(holdPeriods.begin(), holdPeriods.end(), 0.0) / holdPeriods.size();
	return r;
}

void outputHumanTime(std::ostream &os, double seconds) {
	if (seconds > 60) {
		double minutes = seconds / 60;
		if (minutes > 60) {
			double hours = minutes / 60;
			os << hours << " hour(s)";
		} else {
			os << minutes << " minute(s)";
		}
	} else {
		os << seconds << " second(s)";
	}
}


void Algo::writeResults(tbb::concurrent_vector<std::pair<Variables, EvaluationResult>> &results, ParameterStore &paramStore) {
	int algo_inst_id = paramStore.newAlgoInstance(this->ticker, this->computerName, this->slices.at(0).first, this->slices.last());

	for (auto it = results.begin(); it != results.end(); it++) {
		EvaluationResult r = it->second;
		Variables vars = it->first;

		// 1. make a new Result using r and get its id as result_id
		int result_id    = paramStore.newResult(r);

		// 2. make a new ParamSet using algo_inst_id and result_id and get its id as paramset_id
		int paramset_id  = paramStore.newParamSet(algo_inst_id, result_id);

		// 3. make new ParamValue for all variable key/value pairs from v using paramset_id
	        paramStore.newParamValues(paramset_id, vars);
	}
}
