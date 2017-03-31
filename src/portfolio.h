#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <string>
#include <vector>
#include <cmath>

#include "sqlitedb.h"
#include "stockmarket.h"
#include "signalstore.h"
#include "datetime.h"
#include "order.h"
#include "position.h"

class Portfolio : public SQLiteDB {
public:
	Portfolio(const std::string &dbFile, int numStocksToHave, double liquidityPercent) : SQLiteDB(dbFile) {
		this->NUM_STOCKS_TO_HAVE = numStocksToHave;
		this->LIQUIDITY_PERCENT = liquidityPercent;

		cash = 0;
		populate_db_schema();
	}

	// TODO improve this
	std::vector<Order> getOrders(StockMarket &market, const SignalStore &sigStore, const DateTime &today) const {
		std::vector<Order> ret;

		// *********************
		// SELL
		// sell off current positions if signal is > 0
		// *********************
		std::vector<Position> currHolding = currentlyHolding();
		for (Position &p: currHolding) {
			double sig = sigStore.getBestSignal(p.ticker, today);

			if (sig > 0) {
				// some algo says to sell, so sell
				Order o;
				o.ticker = p.ticker;
				o.numShares = p.numShares;
				o.limitPrice = market.loadAll(p.ticker).atDt(today).open;
				o.type = "sell";
				o.dt = today;
				ret.push_back(o);
			}
		}



		// *********************
		// BUY
		// buy 1 share if sig <= -90
		// *********************
		std::vector<std::string> tickers = market.getAllTickers();
		for (auto &t: tickers) {
			double sig = sigStore.getBestSignal(t, today);
			if (sig <= -90) {
				double marketPrice = market.loadAll(t).atDt(today).open;

				double spendableCash = getMoneySpendableFor(t);
				int numSharesToBuy = spendableCash / marketPrice;

				if (numSharesToBuy == 0) continue;

				Order o;
				o.ticker = t;
				o.numShares = numSharesToBuy;
				o.limitPrice = marketPrice;
				o.type = "buy";
				o.dt = today;
				ret.push_back(o);
			}
		}

		return ret;
	}

	bool execOrder(const Order &o) {
		if (o.type == "deposit") {
			placeOrder(o);
			return true;
		}

		// if sell order and we have enough shares to sell
		if (o.type == "sell" && getPositionFor(o.ticker).numShares >= o.numShares) {
			placeOrder(o);
			return true;
		}

		// if buy order and have enough cash
		else if (o.type == "buy" && cash >= o.numShares * o.limitPrice && o.numShares > 0) {
			placeOrder(o);
			return true;
		}

		else if (o.type == "unload") {
			// just assume the order is correct...?
			placeOrder(o);
			return true;
		}

		std::cout << "SKIPPED BAD ORDER: " << o << std::endl;
		return false;
	}

	void execOrders(const std::vector<Order> &orders) {
		beginTransaction();

		for (auto it = orders.begin(); it != orders.end(); it++) {
			execOrder(*it);
		}

		endTransaction();
	}

	void stopSimulation(const DateTime &dt) {
		std::vector<Position> currHolding = currentlyHolding();
		for (Position &p: currHolding) {
			Order o;
			o.ticker = p.ticker;
			o.numShares = p.numShares;
			o.limitPrice = p.avgPrice;
			o.type = "unload";
			o.dt = dt;
			execOrder(o);
		}

		std::cout << "Cash after stopping simulation: $" << std::to_string(cash) << std::endl;
	}

	double getCash() const {
		return cash;
	}

private:
	double getMoneySpendableFor(const std::string &ticker) const {
		Position p = getPositionFor(ticker);
		return getAllocationAmount() - (p.numShares * p.avgPrice) - getReserveAmountFor(ticker);
	}

	double getReserveAmountFor(const std::string &ticker) const {
		return getAllocationAmount() * std::pow(LIQUIDITY_PERCENT, getNumOrdersSinceSelloutFor(ticker) + 1);
	}

	int getNumOrdersSinceSelloutFor(const std::string &ticker) const {
		// get all orders for ticker
		std::string sql = R"(
			SELECT * FROM `Order`
			WHERE (`type` = 'sell' OR `type` = 'buy') AND `ticker` = ?
			ORDER BY `dt` ASC
		)";

		std::vector<Order> orders;

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);

		int rc;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			orders.push_back(stmtToOrder(stmt));
		}

		sqlite3_finalize(stmt);

		int counter = 0;
		int numShares = 0;
		for (Order &o: orders) {
			if (o.type == "buy") {
				counter++;
				numShares += o.numShares;
			}

			if (o.type == "sell") {
				numShares -= o.numShares;

				if (numShares == 0) {
					counter = 0;
				}
			}
		}

		return counter;
	}

	double getAllocationAmount() const {
		double ret = getHistoricalValue() / NUM_STOCKS_TO_HAVE;
		return ret;
	}

	double getHistoricalValue() const {
		double ret = 0;

		// add up all deposits
		std::string sql = R"(
			SELECT SUM(`limitPrice`) FROM (
				SELECT `limitPrice` FROM `Order` WHERE
				`type` = 'deposit'
			)
		)";
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_step(stmt);
		double sumOfDeposits = sqlite3_column_double(stmt, 0);
		sqlite3_finalize(stmt);

		ret += sumOfDeposits;

		// get DISTINCT list of tickers in Order table
		sql = R"(
			SELECT DISTINCT `ticker` FROM `Order`
			WHERE `ticker` != ''
			ORDER BY `ticker` ASC
		)";

		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

		int rc;
		std::vector<std::string> tickers;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			tickers.push_back(std::string((char*)sqlite3_column_text(stmt, 0)));
		}

		sqlite3_finalize(stmt);


		for (std::string &t: tickers) {
			ret += getProfitFor(t);
		}

		return ret;
	}

	double getProfitFor(const std::string &ticker) const {
		std::string sql = R"(
			SELECT * FROM `Order`
			WHERE (`type` = 'sell' OR `type` = 'buy') AND `ticker` = ?
			ORDER BY `ticker` ASC, `dt` ASC
		)";

		std::vector<Order> orders;

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);

		int rc;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			orders.push_back(stmtToOrder(stmt));
		}

		sqlite3_finalize(stmt);

		// remove orphined BUYs at the end of orders vector... i.e., it should end with a sell
		for (int i = orders.size() - 1; i >= 0; i--) {
			Order &o = orders.at(i);

			// if sell, don't delete any more!
			if (o.type == "sell") break;

			// erase buys
			if (o.type == "buy") {
				orders.erase(orders.begin() + i);
			}
		}

		double profit = 0;
		for (Order &o: orders) {
			if (o.type == "sell") {
				profit += o.numShares * o.limitPrice;
			}

			if (o.type == "buy") {
				profit -= o.numShares * o.limitPrice;
			}
		}

		return profit;
	}

	Order stmtToOrder(sqlite3_stmt *stmt) const {
		Order o;
		o.ticker = std::string((char*)sqlite3_column_text(stmt, 1));
		o.numShares = sqlite3_column_int(stmt, 2);
		o.limitPrice = sqlite3_column_double(stmt, 3);
		o.type = std::string((char*)sqlite3_column_text(stmt, 4));
		DateTime dt(std::string((char*)sqlite3_column_text(stmt, 5)));
		o.dt = dt;
		return o;
	}

	std::vector<Position> currentlyHolding() const {
		std::string sql = "SELECT DISTINCT `ticker` FROM `Order` WHERE `type` = 'buy'";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

		std::vector<std::string> tickers;

		int rc;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			std::string t = std::string((char*)sqlite3_column_text(stmt, 0));
			tickers.push_back(t);
		}

		sqlite3_finalize(stmt);

		std::vector<Position> ret;

		for (auto &t: tickers) {
			Position p = getPositionFor(t);
			if (p.numShares > 0) {
				ret.push_back(p);
			}
		}

		return ret;
	}

	Position getPositionFor(const std::string &ticker) const {
		std::string sql = "SELECT * FROM `Order` WHERE `ticker` = ? ORDER BY `dt` ASC";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);

		std::vector<Order> orders;

		int rc;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			orders.push_back(stmtToOrder(stmt));
		}

		sqlite3_finalize(stmt);

		int numShares = 0;
		double avgPrice = 0;

		for (auto it = orders.begin(); it != orders.end(); it++) {
			Order o = *it;
			if (o.type == "buy") {
				avgPrice = ((avgPrice * numShares) + (o.numShares * o.limitPrice)) / (numShares + o.numShares);
				numShares += o.numShares;
			} else if (o.type == "sell") {
				numShares -= o.numShares;

				// if you sold out everything, avgPrice is $0
				if (numShares == 0) {
					avgPrice = 0;
				}
			}
		}
	

		Position p;
		p.ticker = ticker;
		p.numShares = numShares;
		p.avgPrice = avgPrice;

		return p;
	}

	void placeOrder(const Order &o) {
		std::string sql = std::string("INSERT INTO `Order` (`ticker`, `numShares`, `limitPrice`, `type`, `dt`) VALUES (?,?,?,?,'") + o.dt.to_string() + std::string("')");

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

		sqlite3_bind_text(stmt, 1, o.ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, o.numShares);
		sqlite3_bind_double(stmt, 3, o.limitPrice);
		sqlite3_bind_text(stmt, 4, o.type.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// update cash
		if (o.type == "sell") {
			cash += o.limitPrice * o.numShares;
		} else if (o.type == "buy") {
			cash -= o.limitPrice * o.numShares;
		} else if (o.type == "deposit") {
			cash += o.limitPrice;
		} else if (o.type == "unload") {
			cash += o.limitPrice * o.numShares;
		}
	}

	void populate_db_schema() {
		std::string sql = R"(
			BEGIN TRANSACTION;
			CREATE TABLE IF NOT EXISTS `Order` (
				`id`		INTEGER PRIMARY KEY AUTOINCREMENT,
				`ticker`	TEXT NOT NULL,
				`numShares`	INTEGER NOT NULL,
				`limitPrice`	REAL NOT NULL,
				`type`		TEXT NOT NULL,
				`dt`		TEXT NOT NULL
			);
			COMMIT;
		)";

		exec_query(sql, nullptr, nullptr);
	}

	// constants passed in via constructor
	int NUM_STOCKS_TO_HAVE;
	double LIQUIDITY_PERCENT;

	double cash;
};

#endif // PORTFOLIO_H
