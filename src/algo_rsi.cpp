#include "algo_rsi.h"

int Algo_RSI::run(size_t todayIndex, const Variables &v) {
	int var_period = v.at("period");
	double var_limit = v.at("limit");

	double result = TechnicalIndicators::rsi(slices, todayIndex, var_period);

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

void Algo_RSI::populateVariables() {
	std::vector<int> period = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

	std::vector<double> limit;

	const static int MAX_LIMIT = 40;
	const static int STEPS = 10;
	for (size_t i = 1; i <= MAX_LIMIT * STEPS; i++) {
		limit.push_back((double)i / STEPS);
	}

	for (auto a = period.begin(); a != period.end(); a++) {
		for (auto b = limit.begin(); b != limit.end(); b++) {
			Variables v;
			v.insert({"period", *a});
			v.insert({"limit", *b});

			vars.push(v);
		}
	}
}

Variables Algo_RSI::getDefaultVariables() {
	Variables ret;
	ret.emplace("period", 14);
	ret.emplace("limit", 30);
	return ret;
}
