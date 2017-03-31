#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <functional>
#include <unordered_map>
#include <algorithm>

// signal handling
#include <cstdlib>
#include <csignal>

#include "algo_bollingerbands.h"
#include "algo_sma.h"
#include "algo_sideways.h"
#include "algo_stochasticoscillator.h"
#include "algo_rsi.h"
#include "algo_mfi.h"

#include "datetime.h"
#include "variables.h"
#include "slices.h"
#include "slice.h"
#include "clas.h"

#include "stockmarket.h"
#include "parameterstore.h"
#include "signalstore.h"

#include "portfolio.h"
#include "order.h"
#include "position.h"

double computeSignal(ParameterStore &paramStore, const std::string &ticker, const Slices &slices, size_t todayIndex, const CLAs &args, bool bestSignal);

void opt(StockMarket &market, ParameterStore &paramStore, const std::string &ticker, const CLAs &args);
void sig(SignalStore &sigStore, StockMarket &market, ParameterStore &paramStore, const std::string &ticker, const CLAs &args);

void fillAlgos(const std::string &ticker, const Slices &slices, std::vector<std::shared_ptr<Algo>> &algos);

bool MUST_EXIT = false;
void handler_SIGINT(int s) {
	MUST_EXIT = true;
}

int main(int argc, char **argv) {
	// handle Ctrl-C signal
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = handler_SIGINT;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	CLAs args(argc, argv);
	const std::string action = args.action;

	Portfolio portfolio("data/TRANSACTIONS.db", 20, .3);
	StockMarket market("data/HISTORY.db");
	ParameterStore paramStore("data/PARAMETERS.db");
	SignalStore sigStore("data/TECH-SIGNALS.db");

	// run simulation
	if (action == "sim") {
		DateTime initDt;

		// make inital deposit into portfolio
		Order dep;
		dep.ticker = "";
		dep.numShares = 0;
		dep.limitPrice = 100000;
		dep.type = "deposit";
		dep.dt = initDt;
		portfolio.execOrder(dep);

		DateTime prevDt = initDt;
		DateTime dt = market.nextDtAfter(initDt);
		while (dt != initDt) {
			if (MUST_EXIT) {
				std::cout << "Upon request, exiting..." << std::endl;
				prevDt = dt;
				break;
			}

			std::cout << "Getting orders for " << dt << "..." << std::flush;

			// SELL ORDERS
			std::vector<Order> sellOrders = portfolio.getSellOrders(market, sigStore, dt);
			int numSucc = portfolio.execOrders(sellOrders);
			std::cout << numSucc << "/" << sellOrders.size() << " sell order(s) successful, " << std::flush;

			// BUY ORDERS
			std::vector<Order> buyOrders = portfolio.getBuyOrders(market, sigStore, dt);
			numSucc = portfolio.execOrders(buyOrders);
			std::cout << numSucc << "/" << buyOrders.size() << " buy order(s) successful, " << std::flush;

			// FAIR VALUE
			double fairValue = portfolio.getFairValue();
			std::cout << "fair portfolio value: $" << std::to_string(fairValue) << std::endl;

			prevDt = dt;
			dt = market.nextDtAfter(dt);
		}

		portfolio.stopSimulation(prevDt);
		return 0;
	}

	// compute orders for latest DateTime in the StockMarket
	if (action == "run") {
		DateTime dt = market.lastDt();
		std::vector<Order> sOrders = portfolio.getSellOrders(market, sigStore, dt);
		std::vector<Order> bOrders = portfolio.getBuyOrders(market, sigStore, dt);

		// concatenate buy/sell orders
		std::vector<Order> orders;
		orders.insert(orders.end(), sOrders.begin(), sOrders.end());
		orders.insert(orders.end(), bOrders.begin(), bOrders.end());

		std::cout << "Orders for " << dt << ":" << std::endl;
		for (auto it = orders.begin(); it != orders.end(); it++) {
			std::cout << *it << std::endl;
		}

		return 0;
	}

	if (args.tickers.size() == 0) {
		std::cout << "Error: '" << action << "' requires at least one ticker specified..." << std::endl;
	} else {
		for (const auto &ticker : args.tickers) {
			if (MUST_EXIT) {
				std::cout << "Upon request, exiting..." << std::endl;
				break;
			}

			if (action == "opt") {
				opt(market, paramStore, ticker, args);
			}
	
			if (action == "sig") {
				try {
					sig(sigStore, market, paramStore, ticker, args);
				} catch (std::exception &e) {
					std::cout << "Error: " << e.what() << std::endl;
					continue;
				}
			}
		}

		// compress db to save space
		if (action == "opt") {
			paramStore.vacuum();
		}
	}
}

void sig(SignalStore &sigStore, StockMarket &market, ParameterStore &paramStore, const std::string &ticker, const CLAs &args) {
	Slices slices = market.loadAll(ticker);

	DateTime initDt;

	DateTime lastDt = sigStore.lastDtFor(ticker);
	unsigned int index;

	if (lastDt == initDt) {
		index = 0;
		lastDt = slices.at(0).first;
	} else {
		index = slices.indexOf(lastDt);
	}

	std::cout << "Computing " << (slices.size() - index - 1) << " signals for $" << ticker << " from " << lastDt << " to " << slices.last() << "..." << std::flush;

	sigStore.beginTransaction();

	for (unsigned int i = index; i < slices.size(); i++) {
		double bestSignal = computeSignal(paramStore, ticker, slices, i, args, true); // true for best signal
		double defaultSignal = computeSignal(paramStore, ticker, slices, i, args, false); // false for default signal 
		sigStore.insertSignal(ticker, slices.at(i).first, bestSignal, defaultSignal);

		if (MUST_EXIT) {
			std::cout << "\n\nReceived request to stop, so cleaning up database..." << std::endl;
			sigStore.rollback();
			sigStore.close();
			market.close();
			paramStore.close();
			exit(0);
		}
	}

	std::cout << " writing to db..." << std::endl;
	sigStore.endTransaction();
}

void opt(StockMarket &market, ParameterStore &paramStore, const std::string &ticker, const CLAs &args) {
	std::vector<std::shared_ptr<Algo>> algos;
	Slices slices = market.loadAll(ticker);
	fillAlgos(ticker, slices, algos);

	paramStore.beginTransaction();

	for (auto &a: algos) {
		if (paramStore.haveAlgoInstance(ticker, a->getComputerName())) {
			std::cout << "Already optimized $" << ticker << " for " << a->getComputerName() << " algorithm, skipping" << std::endl;
			continue;
		}

		try {
			a->optimize(args, paramStore);
		} catch (std::exception &e) {
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "Skipping " << a->getComputerName() << " algorithm" << std::endl;
			paramStore.rollback();
			paramStore.beginTransaction();
			continue;
		}

		if (MUST_EXIT) {
			std::cout << "\nReceived request to stop, so cleaning up database..." << std::endl;
			paramStore.rollback();
			market.close();
			paramStore.close();
			exit(0);
		}

		// optimal
		Variables bestVars = paramStore.getBestVariables(ticker, a->getComputerName());
		EvaluationResult bestResult = a->evaluate(bestVars, args);
		paramStore.assignOptimalParamSet(ticker, a->getComputerName(), bestVars, bestResult);

		// default
		Variables defaultVars = a->getDefaultVariables();
		EvaluationResult defaultResult = a->evaluate(defaultVars, args);
		paramStore.assignDefaultParamSet(ticker, a->getComputerName(), defaultVars, defaultResult);

		// set datetimes to ParamSet
		paramStore.assignDateTimesToParamSet(ticker, a->getComputerName(), slices.at(0).first, slices.last());

		// delete all extra rows in DB
		paramStore.removeNonOptimalParamSets(ticker, a->getComputerName());
	}

	std::cout << "Writing parameters to db file..." << std::endl;
	paramStore.endTransaction();

	std::cout << std::endl;
}

double computeSignal(ParameterStore &paramStore, const std::string &ticker, const Slices &slices, size_t todayIndex, const CLAs &args, bool bestSignal) {
	std::vector<std::shared_ptr<Algo>> algos;
	fillAlgos(ticker, slices, algos);

	double ret = 0;
	for (auto &a: algos) {
		int result;
		double weight;

		if (bestSignal) {
			// best
			result = a->run(todayIndex, paramStore.getOptimalVariables(ticker, a->getComputerName()));
			weight = paramStore.getBestWeight(ticker, a->getComputerName());
			ret += weight * result;
		} else {
			// default
			result = a->run(todayIndex, a->getDefaultVariables());
			weight = paramStore.getDefaultWeight(ticker, a->getComputerName());
			ret += weight * result;
		}

		if (args.verbose) {
			std::cout << (weight*100) << "\% of " << result << " - " << a->getComputerName() << " algorithm" << std::endl;
		}
	}

	return ret;
}

void fillAlgos(const std::string &ticker, const Slices &slices, std::vector<std::shared_ptr<Algo>> &algos) {
	algos.push_back(std::shared_ptr<Algo>(new Algo_Sideways(ticker, slices)));
	algos.push_back(std::shared_ptr<Algo>(new Algo_BollingerBands(ticker, slices)));
	algos.push_back(std::shared_ptr<Algo>(new Algo_SMA(ticker, slices)));
	algos.push_back(std::shared_ptr<Algo>(new Algo_StochasticOscillator(ticker, slices)));
	algos.push_back(std::shared_ptr<Algo>(new Algo_RSI(ticker, slices)));
	algos.push_back(std::shared_ptr<Algo>(new Algo_MFI(ticker, slices)));
}
