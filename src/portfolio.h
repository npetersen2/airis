#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "sqlitedb.h"
#include "stockmarket.h"
#include "signalstore.h"
#include "datetime.h"
#include "order.h"
#include "position.h"

class Portfolio : public SQLiteDB {
public:
	Portfolio(const std::string &dbFile, int numStocksToHave, double liquidityPercent);

	std::vector<Order> getSellOrders(StockMarket &market, const SignalStore &sigStore, const DateTime &today) const;
	std::vector<Order> getBuyOrders(StockMarket &market, const SignalStore &sigStore, const DateTime &today);

	bool execOrder(const Order &o);
	int execOrders(const std::vector<Order> &orders);

	void stopSimulation(const DateTime &dt);

	double getCash() const;
	double getFairValue() const; // TODO make this

private:
	// profit caching system
	void invalidateCacheProfitFor(const std::string &ticker);
	bool validCacheProfitFor(const std::string &ticker);
	double getCacheProfitFor(const std::string &ticker);
	void setCacheProfitFor(const std::string &ticker, double profit);
	std::unordered_map<std::string, bool> cacheProfitValidity;
	std::unordered_map<std::string, double> cacheProfit;


	double getMoneySpendableFor(const std::string &ticker);
	double getReserveAmountFor(const std::string &ticker);
	int getNumOrdersSinceSelloutFor(const std::string &ticker) const;
	double getAllocationAmount();
	double getHistoricalValue();
	double getProfitFor(const std::string &ticker);

	std::vector<Position> currentlyHolding() const;
	Position getPositionFor(const std::string &ticker) const;

	void placeOrder(const Order &o);

	Order stmtToOrder(sqlite3_stmt *stmt) const;
	void populate_db_schema();


	// constants passed in via constructor
	int NUM_STOCKS_TO_HAVE;
	double LIQUIDITY_PERCENT;

	double cash;
};

#endif // PORTFOLIO_H
