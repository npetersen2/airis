#include "stockmarket.h"

Slice StockMarket::loadSliceFor(const std::string &ticker, const DateTime &dt) {
	const std::string d = dt.to_string();
	const std::string sql = std::string("SELECT * FROM `History` WHERE `ticker` = ? AND `datetime` >= (SELECT substr('") + d + std::string("', 0, instr('") + d + std::string("', ' '))) LIMIT 1");

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);

	sqlite3_step(stmt);
	Slice ret = stmtToSlice(stmt, -1);
	sqlite3_finalize(stmt);

	return ret;
}

DateTime StockMarket::nextDtAfter(const DateTime &dt) const {
	std::string sql = std::string("SELECT DISTINCT `datetime` FROM `History` WHERE `datetime` > '") + dt.to_string() + std::string("' ORDER BY `datetime` ASC LIMIT 1");

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

	int rc = sqlite3_step(stmt);

	if (rc == SQLITE_DONE) {
		// no more later datetimes...
		sqlite3_finalize(stmt);
		DateTime ret;
		return ret;
	}

	std::string text = std::string((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);

	DateTime ret(text);
	return ret;
}

DateTime StockMarket::lastDt() const {
	std::string sql = std::string("SELECT `datetime` FROM `History` ORDER BY `datetime` DESC LIMIT 1");

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

	sqlite3_step(stmt);

	std::string text = std::string((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);

	DateTime ret(text);
	return ret;

}

std::vector<std::string> StockMarket::getAllTickers() const {
	std::string sql = "SELECT DISTINCT `ticker` FROM `History` ORDER BY `ticker` ASC";

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

	std::vector<std::string> ret;

	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		std::string t = std::string((char*)sqlite3_column_text(stmt, 0));
		ret.push_back(t);
	}

	sqlite3_finalize(stmt);
	return ret;
}

// TODO cache this result...
Slices StockMarket::loadAll(const std::string ticker) {
	const std::string sql = std::string("SELECT `datetime`, `open`, `high`, `low`, `close`, `volume` FROM `History` WHERE `ticker` = '") + ticker + std::string("' ORDER BY `datetime` ASC");

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

/*
bool StockMarket::loaded(const std::string ticker) {
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

size_t StockMarket::count(const std::string ticker) {
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
*/


Slice StockMarket::stmtToSlice(sqlite3_stmt *stmt, int index) const {
	Slice s;
	s.open =   sqlite3_column_double(stmt, 2);
	s.high =   sqlite3_column_double(stmt, 3);
	s.low =    sqlite3_column_double(stmt, 4);
	s.close =  sqlite3_column_double(stmt, 5);
	s.volume = sqlite3_column_double(stmt, 6);
	s.index = index;
	return s;
}
