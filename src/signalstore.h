#ifndef SIGNALSTORE_H
#define SIGNALSTORE_H

#include <string>

#include "sqlitedb.h"
#include "datetime.h"

class SignalStore : public SQLiteDB {
public:
	SignalStore(const std::string &dbFile) : SQLiteDB(dbFile) {}

	void insertSignal(const std::string &ticker, const DateTime &dt, double bestSignal, double defaultSignal) {
		make_table(ticker);

		std::string sql = std::string("INSERT OR IGNORE INTO `") + ticker;
		sql += "` (`datetime`, `bestSignal`, `defaultSignal`) VALUES ('";
		sql += dt.to_string();
		sql += "',";
		sql += std::to_string(bestSignal);
		sql += ",";
		sql += std::to_string(defaultSignal);
		sql += ")";

		exec_query(sql, nullptr, nullptr);
	}

private:
	void make_table(const std::string &ticker) {
		std::string sql = std::string("CREATE TABLE IF NOT EXISTS `");
		sql += ticker;
		sql += "` (`datetime` TEXT PRIMARY KEY, `bestSignal` REAL NOT NULL, `defaultSignal` REAL NOT NULL)";
		exec_query(sql, nullptr, nullptr);
	}
};

#endif // SIGNALSTORE_H
