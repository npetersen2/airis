#ifndef ALGO_SIDEWAYS_H
#define ALGO_SIDEWAYS_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"
#include "slices.h"

class Algo_Sideways : public Algo {

public:
	Algo_Sideways(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "Sideways", "sideways") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_SIDEWAYS_H
