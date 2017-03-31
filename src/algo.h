#ifndef ALGO_H
#define ALGO_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <map>
#include <chrono>
#include <ctime>
#include <atomic>
#include <mutex>

#include "datetime.h"
#include "variables.h"
#include "slices.h"
#include "evaluationresult.h"
#include "clas.h"
#include "parameterstore.h"

#include "tbb/tbb.h"

class Algo {
public:
	Algo(const std::string &ticker, const Slices &slices, const std::string &humanName, const std::string &computerName) {
		this->ticker = ticker;
		this->slices = slices;

		this->humanName = humanName;
		this->computerName = computerName;
	}


	/* @brief implemented in derived class: runs algo to determine sell/buy/undetermined
	 *
	 * @param dt the date to run the algo on (pretend that is the last data point to look at)
	 * @param vars the specific variables used for the algo
	 *
	 * @return whether to buy/sell/undetermined
	 *		100: SELL
	 *		-100: BUY
	 *		0: undetermined
	 */
	virtual int run(size_t todayIndex, const Variables &vars) = 0;
	virtual void populateVariables() = 0; 
	virtual Variables getDefaultVariables() = 0;

	/* @brief computes the compounded percent gain from using this algo for all test data
	 *
	 * @param vars the specific variables used for the algo
	 *
	 * @return the percent gained (1=100% gain)
	 */
	EvaluationResult evaluate(const Variables &vars, const CLAs &clas);


	void printProgress(int curr, int total, std::atomic<int> &dotsPrinted);


	/* @brief optimizes the algo to produce the maximum profit for the given algo
	 *        by rerunning the evaluate function for different input variables
	 *        (usually takes a long time to run)
	 */
	void optimize(const CLAs &args, ParameterStore &paramStore);

	void writeResults(tbb::concurrent_vector<std::pair<Variables, EvaluationResult>> &results, ParameterStore &paramStore);

	const std::string getHumanName()    const { return humanName;    }
	const std::string getComputerName() const { return computerName; }
	const std::string getTicker()       const { return ticker;       }
protected:
	std::string humanName;
	std::string computerName;

	std::string ticker;
	Slices slices;

	tbb::concurrent_queue<Variables> vars;

private:
	std::mutex mutex;
};

#endif // ALGO_H
