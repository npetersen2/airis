#ifndef TECHNICALINDICATORS_H
#define TECHNICALINDICATORS_H

#include <tuple>

#include "datetime.h"
#include "slices.h"

class TechnicalIndicators {
public:
	static double mfi(const Slices &slices, int todayIndex, int period);
	static double rsi(const Slices &slices, int todayIndex, int period);
	static double stochastic_oscillator(const Slices &slices, int todayIndex, int periodhl, int periodk);
	static double max_open(const Slices &slices, int todayIndex, int period);
	static double min_open(const Slices &slices, int todayIndex, int period);
	static double sma(const Slices &slices, int todayIndex, int length);
	static double stddev(const Slices &slices, int todayIndex, int length);

	static std::tuple<double, double, double> bollingerbands(const Slices &slices, int todayIndex, int length);

	static void test(const Slices &slices, size_t todayIndex);

private:
	static void assertNaturalNum(const std::string &s, int input);
	static void assertValidIndex(const Slices &slices, int i);
};

#endif // TECHNICALINDICATORS_H
