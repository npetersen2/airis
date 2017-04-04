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

	DateTime nextDtAfter(const DateTime &dt) const; 
	DateTime lastDt() const; 

	std::vector<std::string> getAllTickers() const; 

	Slices loadAll(const std::string ticker);
	Slice loadSliceFor(const std::string &ticker, const DateTime &dt); 
	// bool loaded(const std::string ticker) {
	// size_t count(const std::string ticker) {

private:
	Slice stmtToSlice(sqlite3_stmt *stmt, int index) const;
};

#endif // STOCKMARKET_H
