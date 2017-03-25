#ifndef ALGO_MFI_H
#define ALGO_MFI_H

#include <string>

#include "algo.h"
#include "datetime.h"
#include "variables.h"
#include "technicalindicators.h"
#include "slices.h"

class Algo_MFI : public Algo {

public:
	Algo_MFI(const std::string &ticker, const Slices &slices) : Algo(ticker, slices, "MFI", "mfi") {
	}

	virtual int run(size_t todayIndex, const Variables &v) override;
	virtual void populateVariables() override;
	virtual Variables getDefaultVariables() override;
};

#endif // ALGO_MFI_H
