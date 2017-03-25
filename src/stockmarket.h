#ifndef STOCKMARKET_H
#define STOCKMARKET_H

#include <string>
#include <sqlite3.h>

#include "sqlitedb.h"

class StockMarket : public SQLiteDB {
public:
	StockMarket(const std::string dbFile) : SQLiteDB(dbFile) {
	}

	Slices loadAll(const std::string ticker) {
		const std::string sql = "SELECT `datetime`, `open`, `high`, `low`, `close`, `volume` FROM `" + ticker + "` ORDER BY `datetime` ASC";

		auto callback = +[](void *ptr, int numRows, char **data, char **colName) {
			Slices *slices = (Slices*)ptr;

			DateTime dt(data[0]);

			Slice s;
			s.open = std::stod(data[1]);
			s.high = std::stod(data[2]);
			s.low = std::stod(data[3]);
			s.close = std::stod(data[4]);
			s.volume = std::stod(data[5]);
			s.index = slices->size();

			slices->push_back({dt, s});

			return 0;
		};

		Slices slices;
		exec_query(sql, callback, &slices);
		return slices;
	}

	bool loaded(const std::string ticker) {
		const std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + ticker + "'";

		auto callback = +[](void *ptr, int numRows, char **data, char **colName) {
			bool *found = (bool*)ptr;
			*found = numRows > 0 ? true : false;
			return 0;
		};

		bool found;
		exec_query(sql, callback, &found);
		return found;
	}

	size_t count(const std::string ticker) {
		const std::string sql = "SELECT COUNT(*) FROM " + ticker;

		auto callback = +[](void *ptr, int numRows, char **data, char **colName) {
			size_t *count = (size_t*)ptr;
			*count = std::stoi(data[0]);
			return 0;
		};

		size_t count;
		exec_query(sql, callback, &count);
		return count;
	}
};

#endif // STOCKMARKET_H
