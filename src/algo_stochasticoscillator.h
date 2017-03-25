#ifndef ALGO_STOCHASTICOSCILLATOR_H
#define ALGO_STOCHASTICOSCILLATOR_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"
#include "slices.h"

class Algo_StochasticOscillator : public Algo {

public:
	Algo_StochasticOscillator(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "Stochastic Oscillator", "stochasticoscillator") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_STOCHATICOSCILLATOR_H
