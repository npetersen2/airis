#ifndef SIGNALSTORE_H
#define SIGNALSTORE_H

#include <string>

#include "sqlitedb.h"
#include "datetime.h"

class SignalStore : public SQLiteDB {
public:
	SignalStore(const std::string &dbFile);

	double getDefaultSignal(const std::string &ticker, const DateTime &dt) const;

	DateTime lastDtFor(const std::string &ticker);
	void insertSignal(const std::string &ticker, const DateTime &dt, double bestSignal, double defaultSignal);

private:
	void make_table(const std::string &ticker);
};

#endif // SIGNALSTORE_H
