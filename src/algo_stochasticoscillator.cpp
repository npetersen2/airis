#include "algo_stochasticoscillator.h"

int Algo_StochasticOscillator::run(size_t todayIndex, const Variables &v) {
	int var_periodhl = v.at("periodhl");
	int var_periodk = v.at("periodk");
	double var_limit = v.at("limit");

	double result = TechnicalIndicators::stochastic_oscillator(slices, todayIndex, var_periodhl, var_periodk);

	if (result == -1) {
		// error finding stochastic oscillator value
		return 0;
	}

	if (result >= 100 - var_limit) {
		// SELL
		return 100;
	}

	if (result <= var_limit) {
		// BUY
		return -100;
	}

	// no answer
	return 0;
}

void Algo_StochasticOscillator::populateVariables() {
	std::vector<int> periodhl = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	std::vector<int> periodk = {2, 3, 4, 5};

	std::vector<double> limit;
	for (int i = 1; i <= 45; i++) limit.push_back(i);

	for (auto a = periodhl.begin(); a != periodhl.end(); a++) {
		for (auto b = periodk.begin(); b != periodk.end(); b++) {
			for (auto c = limit.begin(); c != limit.end(); c++) {
				Variables v;
				v.insert({"periodhl", *a});
				v.insert({"periodk", *b});
				v.insert({"limit", *c});

				vars.push(v);
			}
		}
	}
}

Variables Algo_StochasticOscillator::getDefaultVariables() {
	Variables ret;
	ret.emplace("periodhl", 14);
	ret.emplace("periodk", 3);
	ret.emplace("limit", 30);
	return ret;
}
