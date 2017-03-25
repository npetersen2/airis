#include "algo_mfi.h"

int Algo_MFI::run(size_t todayIndex, const Variables &v) {
	int var_period = v.at("period");
	double var_limit = v.at("limit");

	double result = TechnicalIndicators::mfi(slices, todayIndex, var_period);

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

void Algo_MFI::populateVariables() {
	std::vector<int> period = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	std::vector<double> limit;
	for (int i = 1; i <= 45; i++) limit.push_back(i);

	for (auto a = period.begin(); a != period.end(); a++) {
		for (auto b = limit.begin(); b != limit.end(); b++) {
			Variables v;
			v.insert({"period", *a});
			v.insert({"limit", *b});

			vars.push(v);
		}
	}
}

Variables Algo_MFI::getDefaultVariables() {
	Variables ret;
	ret.emplace("period", 14);
	ret.emplace("limit", 30);
	return ret;
}
