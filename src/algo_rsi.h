#ifndef ALGO_RSI_H
#define ALGO_RSI_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"
#include "slices.h"

class Algo_RSI : public Algo {

public:
	Algo_RSI(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "RSI", "rsi") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_RSI_H
