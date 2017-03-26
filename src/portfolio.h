#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <string>
#include <vector>

#include "sqlitedb.h"
#include "stockmarket.h"
#include "signalstore.h"
#include "datetime.h"
#include "order.h"
#include "position.h"

class Portfolio : public SQLiteDB {
public:
	Portfolio(const std::string &dbFile) : SQLiteDB(dbFile) {
		cash = 100000;
		populate_db_schema();
	}

	// TODO make this method
	std::vector<Order> getOrders(const StockMarket &market, const SignalStore &sigStore, const DateTime &today) {
		std::vector<Order> orders;
		return orders;
	}

	void execOrders(const std::vector<Order> &orders) {
		for (auto it = orders.begin(); it != orders.end(); it++) {
			Order o = *it;

			// if sell order and we have enough shares to sell
			if (o.type == "sell" && getPositionFor(o.ticker).numShares >= o.numShares) {
				placeOrder(o);
			}

			// if buy order and have enough cash
			else if (o.type == "buy" && cash >= o.numShares * o.limitPrice) {
				placeOrder(o);
			}

			else {
				std::cout << "SKIPPING BAD ORDER: " << o << std::endl;
			}
		}
	}


//private:

	// TODO below here should be private
	Position getPositionFor(const std::string &ticker) {
		std::string sql = "SELECT * FROM `Order` WHERE `ticker` = ? ORDER BY `dt` ASC";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);

		std::vector<Order> orders;

		int rc;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			Order o;
			o.ticker = std::string((char*)sqlite3_column_text(stmt, 1));
			o.numShares = sqlite3_column_int(stmt, 2);
			o.limitPrice = sqlite3_column_double(stmt, 3);
			o.type = std::string((char*)sqlite3_column_text(stmt, 4));

			DateTime dt(std::string((char*)sqlite3_column_text(stmt, 5)));
			o.dt = dt;

			orders.push_back(o);
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
			}
		}
	

		Position p;
		p.ticker = ticker;
		p.numShares = numShares;
		p.avgPrice = avgPrice;

		return p;
	}

	void placeOrder(const Order &o) {
		std::string sql = "INSERT INTO `Order` (`ticker`, `numShares`, `limitPrice`, `type`, `dt`) VALUES (?,?,?,?,?)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

		sqlite3_bind_text(stmt, 1, o.ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, o.numShares);
		sqlite3_bind_double(stmt, 3, o.limitPrice);
		sqlite3_bind_text(stmt, 4, o.type.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 5, o.dt.to_string().c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// update cash
		if (o.type == "sell") {
			cash += o.limitPrice * o.numShares;
		} else if (o.type == "buy") {
			cash -= o.limitPrice * o.numShares;
		}
	}

private:
	double cash;

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
};

#endif // PORTFOLIO_H
