#ifndef ALGO_SMA_H
#define ALGO_SMA_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"

class Algo_SMA : public Algo {

public:
	Algo_SMA(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "SMA", "sma") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_SMA_H
