#ifndef TECHNICALINDICATORS_H
#define TECHNICALINDICATORS_H

#include <tuple>

#include "datetime.h"
#include "slices.h"

class TechnicalIndicators {
private:
	static void assertNaturalNum(const std::string &s, int input) {
		if (input < 0) {
			std::cout << s << " must be >= 0" << std::endl;
			exit(1);
		}
	}

	static void assertValidIndex(const Slices &slices, int i) {
		if (i < 0 || i >= (int)slices.size()) {
			std::cout << "index " << i << " must be within 0 to " << (slices.size() - 1) << ", inclusive" << std::endl;
			exit(1);
		}
	}

public:
	static double mfi(const Slices &slices, int todayIndex, int period) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("period", period);

		if (todayIndex - period - 2 <= 0) {
			return -1; // can't go back far enough
		}

		todayIndex--; // ignore today (need access to open,close,high,volume)

		double posFlow = 0;
		double negFlow = 0;

		for (int i = 0; i < period; i++) {
			Slice today = slices[todayIndex].second;
			Slice yest = slices[todayIndex - 1].second;

			double today_typ = (today.open + today.high + today.close) / 3;
			double yest_typ = (yest.open + yest.high + yest.close) / 3;

			double today_raw = today_typ * today.volume;

			if (today_typ > yest_typ) {
				posFlow += today_raw;
			}

			else if (today_typ < yest_typ) {
				negFlow += today_raw;
			}

			todayIndex--;
		}

		double mfr = posFlow / negFlow;
		double mfi = 100 - (100 / (1 + mfr));

		return mfi;
	}

	static double rsi(const Slices &slices, int todayIndex, int period) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("period", period);

		if (todayIndex <= period) {
			// can't go back far enough
			return -1;
		}

		double gains = 0;
		double losses = 0;
		int numGains = 0;
		int numLosses = 0;

		for (int i = 0; i < period; i++) {
			double today_price = slices[todayIndex].second.open;
			double yest_price = slices[todayIndex - 1].second.open;

			double tgl = today_price - yest_price;

			if (tgl > 0) {
				gains += tgl;
				numGains++;
			}

			else if (tgl < 0) {
				losses += (-1) * tgl;
				numLosses++;
			}

			todayIndex--;
		}

		double avgGain = gains / numGains;
		double avgLoss = losses / numLosses;

		double rs = avgGain / avgLoss;
		double rsi = 100 - (100 / (1 + rs));

		return rsi;
	}


	static double stochastic_oscillator(const Slices &slices, int todayIndex, int periodhl, int periodk) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("periodhl", periodhl);
		assertNaturalNum("periodk", periodk);


		if (todayIndex - periodk < 0) {
			return -1; // can't go back far enough
		}

		double result = 0;

		for (int i = 0; i < periodk; i++) {
			double min = min_open(slices, todayIndex, periodhl);
			double max = max_open(slices, todayIndex, periodhl);

			if (min == -1 || max == -1) {
				return -1;
			}

			double price = slices[todayIndex].second.open;

			double k = ((price - min) / (max - min)) * 100;
			result += k;

			todayIndex--;
		}

		return result / periodk;
	}


	static double max_open(const Slices &slices, int todayIndex, int period) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("period", period);

		if (todayIndex < period - 1) {
			return -1; // not enough data
		}

		double max = 0;
		for (int i = todayIndex - period + 1; i <= todayIndex; i++) {
			if (slices[i].second.open > max) {
				max = slices[i].second.open;
			}
		}

		return max;
	}

	static double min_open(const Slices &slices, int todayIndex, int period) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("period", period);

		if (todayIndex < period - 1) {
			return -1; // not enough data
		}

		double min = 0;
		for (int i = todayIndex - period + 1; i <= todayIndex; i++) {
			if (slices[i].second.open < min || min == 0) {
				min = slices[i].second.open;
			}
		}

		return min;
	}

	static std::tuple<double, double, double> bollingerbands(const Slices &slices, int todayIndex, int length) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("length", length);

		double stddev = TechnicalIndicators::stddev(slices, todayIndex, length);
		double sma = TechnicalIndicators::sma(slices, todayIndex, length);

		if (stddev == -1 || sma == -1) {
			// not enough data
			return std::tuple<double, double, double>(-1, -1, -1);
		}

		double low = sma - (2 * stddev);
		double mid = sma;
		double high = sma + (2 * stddev);

		return std::tuple<double, double, double>(low, mid, high);
	}

	static double sma(const Slices &slices, int todayIndex, int length) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("length", length);

		if (todayIndex < length - 1) {
			return -1; // not enough data
		}

		double sum = 0;
		for (int i = todayIndex - length + 1; i <= todayIndex; i++) {
			sum += slices[i].second.open;
		}

		return sum / (double)length;
	}

	static double stddev(const Slices &slices, int todayIndex, int length) {
		assertValidIndex(slices, todayIndex);
		assertNaturalNum("length", length);

		if (todayIndex < length - 1) {
			return -1; // not enough data
		}

		double accumulate = 0.0;
		for (int i = todayIndex - length + 1; i <= todayIndex; i++) {
			accumulate += slices[i].second.open;
		}

		double mean = accumulate / length;
		double sum = 0.0;
		for (int i = todayIndex - length + 1; i <= todayIndex; i++) {
			double price = slices[i].second.open;
			sum += (price - mean) * (price - mean);
		}

		return std::sqrt(sum / length);
	}

	static void test(const Slices &slices, size_t todayIndex) {
		assertValidIndex(slices, todayIndex);

		std::cout << "Testing TechnicalIndicators class for DateTime: " << slices[todayIndex].first << std::endl;
	
		double sma = TechnicalIndicators::sma(slices, todayIndex, 3);
		std::tuple<double, double, double> bands = TechnicalIndicators::bollingerbands(slices, todayIndex, 3);
		double stddev = TechnicalIndicators::stddev(slices, todayIndex, 3);
		double max = TechnicalIndicators::max_open(slices, todayIndex, 3);
		double min = TechnicalIndicators::min_open(slices, todayIndex, 3);
		double osc = TechnicalIndicators::stochastic_oscillator(slices, todayIndex, 14, 3);
		double rsi = TechnicalIndicators::rsi(slices, todayIndex, 2);
		double mfi = TechnicalIndicators::mfi(slices, todayIndex, 3);
	
		std::cout << "SMA: " << sma << std::endl;
		std::cout << "Bollinger Bands: low: " << std::get<0>(bands) << ", mid: " << std::get<1>(bands) << ", high: " << std::get<2>(bands) << std::endl;
		std::cout << "Stddev: " << stddev << std::endl;
		std::cout << "Max: " << max << std::endl;
		std::cout << "Min: " << min << std::endl;
		std::cout << "Stochastic Oscillator: " << osc << std::endl;
		std::cout << "RSI: " << rsi << std::endl;
		std::cout << "MFI: " << mfi << std::endl;
	}
};

#endif // TECHNICALINDICATORS_H
