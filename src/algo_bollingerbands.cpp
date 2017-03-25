#include "algo_bollingerbands.h"

int Algo_BollingerBands::run(size_t todayIndex, const Variables &v) {
	int var_period = v.at("period");
	double var_percent = v.at("percent");

	std::tuple<double, double, double> bands = TechnicalIndicators::bollingerbands(slices, todayIndex, var_period);

	double low = std::get<0>(bands);
	double mid = std::get<1>(bands);
	double high = std::get<2>(bands);

	if (low == -1 && mid == -1 && high == -1) {
		// error find bands
		return 0;
	}

	double currPrice = slices[todayIndex].second.open;

	if (currPrice * (1 + (var_percent / 100)) >= high) {
		// SELL
		return 100;
	}

	if (currPrice * (1 - (var_percent / 100)) <= low) {
		// BUY
		return -100;
	}

	// no answer
	return 0;
}

void Algo_BollingerBands::populateVariables() {
	std::vector<int> period = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};

	std::vector<double> percent;
	for (double i = 0; i <= 30; i += .1) percent.push_back(i);

	for (auto a = period.begin(); a != period.end(); a++) {
		for (auto b = percent.begin(); b != percent.end(); b++) {
			Variables v;
			v.insert({"period", *a});
			v.insert({"percent", *b});

			vars.push(v);
		}
	}
}

Variables Algo_BollingerBands::getDefaultVariables() {
	Variables ret;
	ret.emplace("period", 14);
	ret.emplace("percent", 15);
	return ret;
}
