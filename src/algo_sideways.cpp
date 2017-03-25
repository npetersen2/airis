#include "algo_sideways.h"

int Algo_Sideways::run(size_t todayIndex, const Variables &v) {
	int var_lookbackperiod = v.at("lookbackperiod");
	double var_maxstddev = v.at("maxstddev");
	double var_mindrop = v.at("mindrop");
	double var_minjump = v.at("minjump");

	double stddev = TechnicalIndicators::stddev(slices, todayIndex, var_lookbackperiod);
	double sma = TechnicalIndicators::sma(slices, todayIndex, var_lookbackperiod);

	if (stddev == -1 || sma == -1) {
		// error finding tech indicators
		return 0;
	}

	double currPrice = slices[todayIndex].second.open;

	if ((stddev / sma < var_maxstddev) && ((currPrice - sma) / sma > var_minjump)) {
		// SELL
		return 100;
	}

	if ((stddev / sma < var_maxstddev) && ((sma - currPrice) / sma > var_mindrop)) {
		// BUY
		return -100;
	}

	// no answer
	return 0;
}

void Algo_Sideways::populateVariables() {
	std::vector<int> lookbackperiod = {5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29};
	std::vector<double> maxstddev = {.01, .05, .09, .13, .17, .21, .25, .29, .33, .37};
	std::vector<double> mindrop = {.05, .06, .07, .08, .09, .1, .11, .12, .13, .14, .15};
	std::vector<double> minjump = {.05, .06, .07, .08, .09, .1, .11, .12, .13, .14, .15};

	for (auto a = lookbackperiod.begin(); a != lookbackperiod.end(); a++) {
		for (auto b = maxstddev.begin(); b != maxstddev.end(); b++) {
			for (auto c = mindrop.begin(); c != mindrop.end(); c++) {
				for (auto d = minjump.begin(); d != minjump.end(); d++) {
					Variables v;
					v.insert({"lookbackperiod", *a});
					v.insert({"maxstddev", *b});
					v.insert({"mindrop", *c});
					v.insert({"minjump", *d});

					vars.push(v);
				}
			}
		}
	}
}


Variables Algo_Sideways::getDefaultVariables() {
	Variables ret;
	ret.emplace("lookbackperiod", 14);
	ret.emplace("maxstddev", .2);
	ret.emplace("mindrop", .1);
	ret.emplace("minjump", .1);
	return ret;
}
