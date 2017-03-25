#include "algo_sma.h"

int Algo_SMA::run(size_t todayIndex, const Variables &v) {
	int var_sma10 = v.at("sma10");
	int var_sma50 = v.at("sma50");
	double var_sma10delta = v.at("sma10delta");
	double var_sma50delta = v.at("sma50delta");

	if (todayIndex == 0) {
		return 0; // no yesterday
	}

	double sma10curr = TechnicalIndicators::sma(slices, todayIndex,     var_sma10);
	double sma10prev = TechnicalIndicators::sma(slices, todayIndex - 1, var_sma10);
	double sma50curr = TechnicalIndicators::sma(slices, todayIndex,     var_sma50);
	double sma50prev = TechnicalIndicators::sma(slices, todayIndex - 1, var_sma50);

	if (sma10curr == -1 || sma10prev == -1 || sma50curr == -1 || sma50prev == -1) {
		return 0; // couldn't run algo... not enough data to calculate sma
	}

	double currPrice = slices[todayIndex].second.open;

	bool plumeted = ((((sma10curr - sma10prev) / sma10prev) < var_sma10delta) 
				&& (((sma50curr - sma50prev) / sma50prev) < var_sma50delta));

	bool jumped = ((((sma10curr - sma10prev) / sma10prev) > var_sma10delta) 
				&& (((sma50curr - sma50prev) / sma50prev) > var_sma50delta));

	bool tenLessFifty = (sma10curr <= sma50curr);
	bool currPriceLessFifty = (currPrice <= sma50curr);

	if (plumeted && tenLessFifty && currPriceLessFifty) {
		// BUY
		return -100;
	}

	else if (jumped && !tenLessFifty && !currPriceLessFifty) {
		// SELL
		return 100;
	}

	else {
		// NO ANSWER
		return 0;
	}
}

void Algo_SMA::populateVariables() {
	std::vector<int> sma10 = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
	std::vector<int> sma50 = {24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56};
	std::vector<double> sma10delta = {.0006, .0008, .0010, .0012, .0014, .0016, .0018, .0020, .0021, .0022};
	std::vector<double> sma50delta = {.0009, .0010, .0011, .0012, .0013, .0014, .0015, .0016, .0017, .0018};
	
	for (auto a = sma10.begin(); a != sma10.end(); a++) {
		for (auto b = sma50.begin(); b != sma50.end(); b++) {
			for (auto c = sma10delta.begin(); c != sma10delta.end(); c++) {
				for (auto d = sma50delta.begin(); d != sma50delta.end(); d++) {
					Variables v;
					v.insert({"sma10", *a});
					v.insert({"sma50", *b});
					v.insert({"sma10delta", *c});
					v.insert({"sma50delta", *d});

					vars.push(v);
				}
			}
		}
	}
}

Variables Algo_SMA::getDefaultVariables() {
	Variables ret;
	ret.emplace("sma10", 17);
	ret.emplace("sma50", 40);
	ret.emplace("sma10delta", .0014);
	ret.emplace("sma50delta", .0012);
	return ret;
}
