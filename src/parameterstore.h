#ifndef PARAMETERSTORE_H
#define PARAMETERSTORE_H

#include <string>
#include "sqlitedb.h"

class ParameterStore : public SQLiteDB {
public:
	ParameterStore(const std::string dbFile) : SQLiteDB(dbFile) {
		populate_db_schema();
	}

	void removeNonOptimalParamSets(const std::string &ticker, const std::string &algoName) {
		// remove all ParamSets and accompanying records for an AlgoInstance except the optimal ParamSet

		int algo_inst_id = get_algo_inst_id(ticker, algoName);
		int opt_paramset_id = get_opt_paramset_id(ticker, algoName);
		int default_paramset_id = get_default_paramset_id(ticker, algoName);

		std::string sql = R"(
			-- delete Results
			DELETE FROM Result
			WHERE Result.id IN 
			(
			-- all results for AlgoInstance except opt_paramset
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.algo_inst_id = %d
			AND ParamSet.id != %d
			AND ParamSet.id != %d
			);


			-- delete ParamValues
			DELETE FROM ParamValue
			WHERE ParamValue.paramset_id IN
			(
			-- all ParamSets for AlgoInstance except opt_paramset
			SELECT ParamSet.id FROM ParamSet
			WHERE ParamSet.algo_inst_id = %d
			AND ParamSet.id != %d
			AND ParamSet.id != %d
			);


			-- delete ParamSets
			DELETE FROM ParamSet
			WHERE ParamSet.algo_inst_id = %d
			AND ParamSet.id != %d
			AND ParamSet.id != %d;
		)";

		int a = algo_inst_id;
		int b = opt_paramset_id;
		int c = default_paramset_id;
		char buffer[sql.size() * 2];
		std::snprintf(buffer, sql.size() * 2, sql.c_str(), a, b, c, a, b, c, a, b, c);
		std::string s(buffer);

		exec_query(s, nullptr, nullptr);
	}


	void assignDateTimesToParamSet(const std::string &ticker, const std::string &algoName, const DateTime &dt_start, const DateTime &dt_end) {
		int algo_inst_id = get_algo_inst_id(ticker, algoName);

		// set the opt_paramset_id field of AlgoInstance
		std::string sql = "UPDATE `AlgoInstance` SET dt_first = '%s', dt_last = '%s' WHERE AlgoInstance.id = %d";
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);

		char buffer[sql.size() * 2];
		std::snprintf(buffer, sql.size() * 2, sql.c_str(), dt_start.to_string().c_str(), dt_end.to_string().c_str(), algo_inst_id);
		std::string s(buffer);

		exec_query(s, nullptr, nullptr);
	}


	void assignDefaultParamSet(const std::string &ticker, const std::string &algoName, const Variables &vars, const EvaluationResult &result) {
		int algo_inst_id = get_algo_inst_id(ticker, algoName);
		int result_id    = newResult(result);
		int paramset_id  = newParamSet(algo_inst_id, result_id);
	        newParamValues(paramset_id, vars);

		// set the opt_paramset_id field of AlgoInstance
		std::string sql = "UPDATE `AlgoInstance` SET default_paramset_id=? WHERE AlgoInstance.id=?";
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_int(stmt, 1, paramset_id);
		sqlite3_bind_int(stmt, 2, algo_inst_id);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}


	void assignOptimalParamSet(const std::string &ticker, const std::string &algoName, const Variables &vars, const EvaluationResult &result) {
		int algo_inst_id = get_algo_inst_id(ticker, algoName);
		int result_id    = newResult(result);
		int paramset_id  = newParamSet(algo_inst_id, result_id);
	        newParamValues(paramset_id, vars);

		// set the opt_paramset_id field of AlgoInstance
		std::string sql = "UPDATE `AlgoInstance` SET opt_paramset_id=? WHERE AlgoInstance.id=?";
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_int(stmt, 1, paramset_id);
		sqlite3_bind_int(stmt, 2, algo_inst_id);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}


	double getDefaultWeight(const std::string &ticker, const std::string &algoName) {
		std::string sql1 = R"(
			SELECT SUM(Result.totalPercentGain) FROM Result
			WHERE Result.id IN
			(
			-- list of all default results for a ticker
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.id IN
			(
			-- list of default_paramset_ids for a ticker
			SELECT AlgoInstance.default_paramset_id FROM AlgoInstance
			INNER JOIN Ticker ON Ticker.id=AlgoInstance.ticker_id
			WHERE Ticker.symbol = ?
			)
			)
		)";

		sqlite3_stmt *stmt1;
		sqlite3_prepare_v2(this->db, sql1.c_str(), sql1.size(), &stmt1, nullptr);
		sqlite3_bind_text(stmt1, 1, ticker.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt1);
		double sum = sqlite3_column_double(stmt1, 0);
		sqlite3_finalize(stmt1);




		std::string sql2 = R"(
			SELECT Result.totalPercentGain FROM Result
			WHERE Result.id = 
			(
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.id =
			(
			-- default_paramset_id for AAPL/rsi
			SELECT AlgoInstance.default_paramset_id FROM AlgoInstance
			INNER JOIN Ticker ON Ticker.id=AlgoInstance.ticker_id
			INNER JOIN Algorithm ON Algorithm.id=AlgoInstance.algorithm_id
			WHERE Ticker.symbol = ? AND Algorithm.name = ?
			)
			)
		)";
		sqlite3_stmt *stmt2;
		sqlite3_prepare_v2(this->db, sql2.c_str(), sql2.size(), &stmt2, nullptr);
		sqlite3_bind_text(stmt2, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt2, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt2);
		double percentReturn = sqlite3_column_double(stmt2, 0);
		sqlite3_finalize(stmt2);


		double ret = percentReturn / sum;
		return ret < 0 ? 0 : ret;
	}



	double getBestWeight(const std::string &ticker, const std::string &algoName) {
		std::string sql1 = R"(
			SELECT SUM(Result.totalPercentGain) FROM Result
			WHERE Result.id IN
			(
			-- list of all optimal results for a ticker
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.id IN
			(
			-- list of opt_paramset_ids for a ticker
			SELECT AlgoInstance.opt_paramset_id FROM AlgoInstance
			INNER JOIN Ticker ON Ticker.id=AlgoInstance.ticker_id
			WHERE Ticker.symbol = ?
			)
			)
		)";

		sqlite3_stmt *stmt1;
		sqlite3_prepare_v2(this->db, sql1.c_str(), sql1.size(), &stmt1, nullptr);
		sqlite3_bind_text(stmt1, 1, ticker.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt1);
		double sum = sqlite3_column_double(stmt1, 0);
		sqlite3_finalize(stmt1);




		std::string sql2 = R"(
			SELECT Result.totalPercentGain FROM Result
			WHERE Result.id = 
			(
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.id =
			(
			-- opt_paramset_id for AAPL/rsi
			SELECT AlgoInstance.opt_paramset_id FROM AlgoInstance
			INNER JOIN Ticker ON Ticker.id=AlgoInstance.ticker_id
			INNER JOIN Algorithm ON Algorithm.id=AlgoInstance.algorithm_id
			WHERE Ticker.symbol = ? AND Algorithm.name = ?
			)
			)
		)";
		sqlite3_stmt *stmt2;
		sqlite3_prepare_v2(this->db, sql2.c_str(), sql2.size(), &stmt2, nullptr);
		sqlite3_bind_text(stmt2, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt2, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt2);
		double percentReturn = sqlite3_column_double(stmt2, 0);
		sqlite3_finalize(stmt2);


		double ret = percentReturn / sum;
		return ret < 0 ? 0 : ret;
	}



	Variables getOptimalVariables(const std::string &ticker, const std::string &algoName) {
		if (!haveAlgoInstance(ticker, algoName)) {
			std::cout << "Can't get optimal variables for $" << ticker << " and " << algoName << " algorithm." << std::endl;
			std::cout << "Please run the parameter optimizer on $" << ticker << " first." << std::endl;
			throw std::invalid_argument("no optimal variables stored");
		}

		std::string sql = R"(
			SELECT ParamSet.result_id FROM ParamSet
			WHERE ParamSet.id =
			(
			-- opt_paramset_id for AAPL/rsi
			SELECT AlgoInstance.opt_paramset_id FROM AlgoInstance
			INNER JOIN Ticker ON Ticker.id=AlgoInstance.ticker_id
			INNER JOIN Algorithm ON Algorithm.id=AlgoInstance.algorithm_id
			WHERE Ticker.symbol = ? AND Algorithm.name = ?
			)
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		int result_id = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);

		return getResultVariables(result_id);
	}


	Variables getBestVariables(const std::string &ticker, const std::string &algoName) {
		std::vector<Variables> vars = getTopVariables(ticker, algoName);
		return getMedianVars(vars);
	}

	std::vector<Variables> getTopVariables(const std::string &ticker, const std::string &algoName) {
		// sorted list of results
		std::string sql = R"(
			SELECT Result.* FROM
				(
				-- list of results that go with AlgoInstance query result
				SELECT ParamSet.result_id FROM ParamSet
				WHERE ParamSet.algo_inst_id = 
					(
						-- id of AlgoInstance for AAPL/sma
						-- assuming only one AlgoInstance is stored for AAPL/sma
						SELECT AlgoInstance.id FROM AlgoInstance
						INNER JOIN Ticker ON AlgoInstance.ticker_id=Ticker.id
						INNER JOIN Algorithm ON AlgoInstance.algorithm_id=Algorithm.id
						WHERE Ticker.symbol=? AND Algorithm.name=?
						LIMIT 1
					)
				) AS results
			INNER JOIN Result ON results.result_id=Result.id
			ORDER BY (30 * (totalPercentGain / ?) + 70 * (1 - (avgHoldPeriod / ?))) DESC
			LIMIT 100
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, maxResultsColumn(ticker, algoName, "totalPercentGain"));
		sqlite3_bind_double(stmt, 4, maxResultsColumn(ticker, algoName, "avgHoldPeriod"));

		std::vector<Variables> vars;

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int id = sqlite3_column_int(stmt, 0);
			Variables v = getResultVariables(id);
			vars.push_back(v);
		}

		sqlite3_finalize(stmt);

		return vars;
	}


	Variables getMedianVars(std::vector<Variables> &vars) {
		// get variable names
		std::vector<std::string> keys;
		Variables ex = vars.at(0);
		for (auto it = ex.begin(); it != ex.end(); it++) {
			std::string key = it->first;
			keys.push_back(key);
		}

		Variables ret;
		for (auto it = keys.begin(); it != keys.end(); it++) { 
			std::string key = *it;
	
			std::sort(vars.begin(), vars.end(), [key](const Variables &v1, const Variables &v2) {
				return v1.at(key) < v2.at(key);
			});
	
			double value = vars.at(vars.size() / 2).at(key); // middle value (median)
			ret.emplace(key, value);
		}
	
		return ret;
	}



	Variables getResultVariables(int result_id) {
		std::string sql = R"(
			SELECT ParamName.name, ParamValue.value FROM ParamValue
			INNER JOIN ParamName ON ParamName.id=ParamValue.paramname_id
			WHERE ParamValue.paramset_id=
			(
				SELECT id FROM ParamSet
				WHERE ParamSet.result_id=?
			)
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_int(stmt, 1, result_id);

		Variables vars;
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			std::string key = std::string((char*)sqlite3_column_text(stmt, 0));
			double value = sqlite3_column_double(stmt, 1);
			vars.emplace(key, value);
		}

		sqlite3_finalize(stmt);

		return vars;
	}

	double maxResultsColumn(const std::string &ticker, const std::string &algoName, const std::string &columnName) {
		if (!haveAlgoInstance(ticker, algoName)) {
			return -1;
		}

		std::string a = std::string("SELECT MAX(") + columnName + std::string(") FROM "); // sqlite won't bind to column names...
		std::string b = R"(
				(
					SELECT Result.* FROM
						(
						-- list of results that go with AlgoInstance query result
						SELECT ParamSet.result_id FROM ParamSet
						WHERE ParamSet.algo_inst_id = 
							(
								-- id of AlgoInstance for AAPL/sma
								-- assuming only one AlgoInstance is stored for AAPL/sma
								SELECT AlgoInstance.id FROM AlgoInstance
								INNER JOIN Ticker ON AlgoInstance.ticker_id=Ticker.id
								INNER JOIN Algorithm ON AlgoInstance.algorithm_id=Algorithm.id
								WHERE Ticker.symbol=? AND Algorithm.name=?
								LIMIT 1
							)
						) AS results
					INNER JOIN Result ON results.result_id=Result.id
				)
		)";
		std::string sql = a + b;

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);


		sqlite3_step(stmt);
		double ret = sqlite3_column_double(stmt, 0);
		sqlite3_finalize(stmt);
		return ret;	
	}

	// checks if algo instance exists
	bool haveAlgoInstance(const std::string ticker, const std::string algoName) {
		std::string sql = R"(
			SELECT COUNT(*) FROM AlgoInstance
			JOIN Ticker ON AlgoInstance.ticker_id = Ticker.id
			JOIN Algorithm ON AlgoInstance.algorithm_id = Algorithm.id
			WHERE Ticker.symbol=? AND Algorithm.name=?
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		int count = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return count > 0;
	}

	void newParamValues(int paramset_id, const Variables &vars) {
		for (auto it = vars.begin(); it != vars.end(); it++) {
			const std::string key = (*it).first;
			double value = (*it).second;
			
			std::string sql = "";
			sql += "INSERT INTO `ParamValue` (`paramset_id`, `paramname_id`, `value`) VALUES (";
			sql += std::to_string(paramset_id);
			sql += ",";
			sql += std::to_string(get_param_name_id(key));
			sql += ",";
			sql += std::to_string(value);
			sql += ")";

			exec_query(sql, nullptr, nullptr);
		}
	}

	int newParamSet(int algo_inst_id, int result_id) {
		std::string sql = "";
		sql += "INSERT INTO `ParamSet` (`algo_inst_id`, `result_id`) VALUES (";
		sql += std::to_string(algo_inst_id);
		sql += ",";
		sql += std::to_string(result_id);
		sql += ")";

		exec_query(sql, nullptr, nullptr);
		return sqlite3_last_insert_rowid(this->db);

	}


	int newResult(const EvaluationResult &result) {
		std::string sql = R"(
			INSERT INTO `Result` (`totalPercentGain`, `avgPercentGainPerTotalTime`, `avgPercentGainPerHold`, `numTransactions`, `minHoldPeriod`, `maxHoldPeriod`, `avgHoldPeriod`) VALUES (?,?,?,?,?,?,?)
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_double(stmt, 1, result.totalPercentGain);
		sqlite3_bind_double(stmt, 2, result.avgPercentGainPerTotalTime);
		sqlite3_bind_double(stmt, 3, result.avgPercentGainPerHold);
		sqlite3_bind_int(stmt, 4, result.numTransactions);
		sqlite3_bind_int(stmt, 5, result.minHoldPeriod);
		sqlite3_bind_int(stmt, 6, result.maxHoldPeriod);
		sqlite3_bind_int(stmt, 7, result.avgHoldPeriod);

		sqlite3_step(stmt); // exec stmt
		sqlite3_finalize(stmt);

		return sqlite3_last_insert_rowid(this->db);
	}

	int newAlgoInstance(const std::string &ticker, const std::string &name, const DateTime &dtFirst, const DateTime &dtLast) {
		std::string sql = "";
		sql += "INSERT INTO `AlgoInstance` (`ticker_id`, `algorithm_id`) VALUES (";
		sql += std::to_string(get_ticker_id(ticker)); // ticker_id
		sql += ",";
		sql += std::to_string(get_algorithm_id(name)); // algorithm_id
		sql += ")";

		exec_query(sql, nullptr, nullptr);
		return sqlite3_last_insert_rowid(this->db);
	}


private:
	void insert_ignore_ticker(const std::string &ticker) {
		std::string sql = "";
		sql += "INSERT OR IGNORE INTO `Ticker` (`symbol`) VALUES ('";
		sql += ticker; // symbol
		sql += "')";

		exec_query(sql, nullptr, nullptr);
	}

	int get_ticker_id(const std::string &ticker) {
		insert_ignore_ticker(ticker);

		std::string sql = "";
		sql += "SELECT id FROM `Ticker` WHERE `symbol` = '";
		sql += ticker; // symbol
		sql += "' LIMIT 1";

		auto callback = +[](void *ptr, int numRows, char **data, char **colNum) {
			int *id = (int*)ptr;
			*id = std::stoi(data[0]);
			return 0;
		};

		int id;
		exec_query(sql, callback, &id);
		return id;
	}



	void insert_ignore_param_name(const std::string &name) {
		std::string sql = "";
		sql += "INSERT OR IGNORE INTO `ParamName` (`name`) VALUES ('";
		sql += name; // name
		sql += "')";

		exec_query(sql, nullptr, nullptr);
	}

	int get_param_name_id(const std::string &name) {
		insert_ignore_param_name(name);

		std::string sql = "";
		sql += "SELECT id FROM `ParamName` WHERE `name` = '";
		sql += name; // name
		sql += "' LIMIT 1";

		auto callback = +[](void *ptr, int numRows, char **data, char **colNum) {
			int *id = (int*)ptr;
			*id = std::stoi(data[0]);
			return 0;
		};

		int id;
		exec_query(sql, callback, &id);
		return id;
	}



	void insert_ignore_algorithm(const std::string &algoName) {
		std::string sql = "";
		sql += "INSERT OR IGNORE INTO `Algorithm` (`name`) VALUES ('";
		sql += algoName; // name
		sql += "')";

		exec_query(sql, nullptr, nullptr);
	}

	int get_algorithm_id(const std::string &algoName) {
		insert_ignore_algorithm(algoName);

		std::string sql = "";
		sql += "SELECT id FROM `Algorithm` WHERE `name` = '";
		sql += algoName; // name
		sql += "' LIMIT 1";

		auto callback = +[](void *ptr, int numRows, char **data, char **colNum) {
			int *id = (int*)ptr;
			*id = std::stoi(data[0]);
			return 0;
		};

		int id;
		exec_query(sql, callback, &id);
		return id;
	}

	int get_default_paramset_id(const std::string &ticker, const std::string &algoName) {
		std::string sql = R"(
			SELECT AlgoInstance.default_paramset_id FROM AlgoInstance
			JOIN Ticker ON AlgoInstance.ticker_id = Ticker.id
			JOIN Algorithm ON AlgoInstance.algorithm_id = Algorithm.id
			WHERE Ticker.symbol=? AND Algorithm.name=?
			LIMIT 1
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		int id = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return id;
	}



	int get_opt_paramset_id(const std::string &ticker, const std::string &algoName) {
		std::string sql = R"(
			SELECT AlgoInstance.opt_paramset_id FROM AlgoInstance
			JOIN Ticker ON AlgoInstance.ticker_id = Ticker.id
			JOIN Algorithm ON AlgoInstance.algorithm_id = Algorithm.id
			WHERE Ticker.symbol=? AND Algorithm.name=?
			LIMIT 1
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		int id = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return id;
	}



	int get_algo_inst_id(const std::string &ticker, const std::string &algoName) {
		std::string sql = R"(
			SELECT AlgoInstance.id FROM AlgoInstance
			JOIN Ticker ON AlgoInstance.ticker_id = Ticker.id
			JOIN Algorithm ON AlgoInstance.algorithm_id = Algorithm.id
			WHERE Ticker.symbol=? AND Algorithm.name=?
			LIMIT 1
		)";

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(this->db, sql.c_str(), sql.size(), &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, algoName.c_str(), -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		int id = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return id;
	}


	void populate_db_schema() {
		std::string sql = "";
		sql += "BEGIN TRANSACTION;";
		sql += "CREATE TABLE IF NOT EXISTS `Ticker` (";
		sql += "	`id`		INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`symbol`	TEXT NOT NULL UNIQUE";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `Result` (";
		sql += "	`id`				INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`totalPercentGain`		REAL NOT NULL,";
		sql += "	`avgPercentGainPerTotalTime`	REAL NOT NULL,";
		sql += "	`avgPercentGainPerHold`		REAL NOT NULL,";
		sql += "	`numTransactions`		INTEGER NOT NULL,";
		sql += "	`minHoldPeriod`			INTEGER NOT NULL,";
		sql += "	`maxHoldPeriod`			INTEGER NOT NULL,";
		sql += "	`avgHoldPeriod`			INTEGER NOT NULL";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `ParamValue` (";
		sql += "	`id`		INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`paramset_id`	INTEGER,";
		sql += "	`paramname_id`	INTEGER,";
		sql += "	`value`		REAL NOT NULL";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `ParamSet` (";
		sql += "	`id`		INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`algo_inst_id`	INTEGER,";
		sql += "	`result_id`	INTEGER";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `ParamName` (";
		sql += "	`id`	INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`name`	TEXT NOT NULL UNIQUE";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `Algorithm` (";
		sql += "	`id`	INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`name`	TEXT NOT NULL UNIQUE";
		sql += ");";
		sql += "CREATE TABLE IF NOT EXISTS `AlgoInstance` (";
		sql += "	`id`			INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,";
		sql += "	`ticker_id`		INTEGER NOT NULL,";
		sql += "	`algorithm_id`		INTEGER NOT NULL,";
		sql += "	`dt_first`		TEXT,";
		sql += "	`dt_last`		TEXT,";
		sql += "	`opt_paramset_id`	INTEGER,";
		sql += "	`default_paramset_id`	INTEGER";
		sql += ");";
		sql += "COMMIT;";

		exec_query(sql, nullptr, nullptr);
	}

};

#endif // PARAMETERSTORE_H
