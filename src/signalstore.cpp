#include "signalstore.h"

SignalStore::SignalStore(const std::string &dbFile) : SQLiteDB(dbFile) {}

double SignalStore::getDefaultSignal(const std::string &ticker, const DateTime &dt) const {
	std::string sql = std::string("SELECT `defaultSignal` FROM `") + ticker + std::string("` WHERE `datetime` = '") + dt.to_string() + std::string("' LIMIT 1");

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

	sqlite3_step(stmt);

	double sig = sqlite3_column_double(stmt, 0);
	sqlite3_finalize(stmt);

	return sig;
}

DateTime SignalStore::lastDtFor(const std::string &ticker) {
	make_table(ticker);

	std::string sql = std::string("SELECT `datetime` FROM `") + ticker + std::string("` ORDER BY `datetime` DESC LIMIT 1");

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_DONE) {
		// no dts in db
		sqlite3_finalize(stmt);
		DateTime ret;
		return ret;
	}

	std::string text = std::string((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);

	DateTime dt(text);
	return dt;
}

void SignalStore::insertSignal(const std::string &ticker, const DateTime &dt, double bestSignal, double defaultSignal) {
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

// private:

void SignalStore::make_table(const std::string &ticker) {
	std::string sql = std::string("CREATE TABLE IF NOT EXISTS `");
	sql += ticker;
	sql += "` (`datetime` TEXT PRIMARY KEY, `bestSignal` REAL NOT NULL, `defaultSignal` REAL NOT NULL)";
	exec_query(sql, nullptr, nullptr);
}
