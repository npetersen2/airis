#ifndef ALGO_BOLLINGERBANDS_H
#define ALGO_BOLLINGERBANDS_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"
#include "slices.h"

class Algo_BollingerBands : public Algo {

public:
	Algo_BollingerBands(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "Bollinger Bands", "bollingerbands") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_BOLLINGERBANDS_H
