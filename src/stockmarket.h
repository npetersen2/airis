#ifndef STOCKMARKET_H
#define STOCKMARKET_H

#include <string>
#include <sqlite3.h>

#include "sqlitedb.h"
#include "datetime.h"
#include "slices.h"

class StockMarket : public SQLiteDB {
public:
	StockMarket(const std::string dbFile) : SQLiteDB(dbFile) {
	}

	DateTime nextDtAfter(const DateTime &dt) {
		std::string sql1 = "SELECT tbl_name FROM sqlite_master where type='table'";

		sqlite3_stmt *stmt1;
		sqlite3_prepare_v2(this->db, sql1.c_str(), sql1.size(), &stmt1, nullptr);
		sqlite3_step(stmt1);

		std::vector<std::string> tickers;
		int rc;
		while ((rc = sqlite3_step(stmt1)) == SQLITE_ROW) {
			std::string t = std::string((char*)sqlite3_column_text(stmt1, 0));
			tickers.push_back(t);
		}

		sqlite3_finalize(stmt1);


		std::string sql = "SELECT * FROM (";
		for (unsigned int i = 0; i < tickers.size(); i++) {
			sql += std::string("SELECT `datetime` FROM `") + tickers.at(i) + std::string("`");

			if (i + 1 < tickers.size()) {
				// not last
				sql += " UNION ";
			}
		}
		sql += ") WHERE `datetime` > '";
		sql += dt.to_string();
		sql += "' ORDER BY `datetime` ASC LIMIT 1";

		sqlite3_stmt *stmt2;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt2, nullptr);

		rc = sqlite3_step(stmt2);

		if (rc == SQLITE_DONE) {
			// no more later datetimes...
			sqlite3_finalize(stmt2);
			DateTime ret;
			return ret;
		}

		std::string text = std::string((char*)sqlite3_column_text(stmt2, 0));
		sqlite3_finalize(stmt2);

		DateTime ret(text);
		return ret;
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
