#ifndef PARAMETERSTORE_H
#define PARAMETERSTORE_H

#include <string>

#include "sqlitedb.h"
#include "variables.h"
#include "evaluationresult.h"
#include "datetime.h"

class ParameterStore : public SQLiteDB {
public:
	ParameterStore(const std::string dbFile);

	void removeNonOptimalParamSets(const std::string &ticker, const std::string &algoName);

	void assignDateTimesToParamSet(const std::string &ticker, const std::string &algoName, const DateTime &dt_start, const DateTime &dt_end);
	void assignDefaultParamSet(const std::string &ticker, const std::string &algoName, const Variables &vars, const EvaluationResult &result);
	void assignOptimalParamSet(const std::string &ticker, const std::string &algoName, const Variables &vars, const EvaluationResult &result);

	double getDefaultWeight(const std::string &ticker, const std::string &algoName);
	double getBestWeight(const std::string &ticker, const std::string &algoName);
	Variables getOptimalVariables(const std::string &ticker, const std::string &algoName);
	Variables getBestVariables(const std::string &ticker, const std::string &algoName);
	std::vector<Variables> getTopVariables(const std::string &ticker, const std::string &algoName);
	Variables getMedianVars(std::vector<Variables> &vars);
	Variables getResultVariables(int result_id);

	double maxResultsColumn(const std::string &ticker, const std::string &algoName, const std::string &columnName);

	// checks if algo instance exists
	bool haveAlgoInstance(const std::string ticker, const std::string algoName);

	void newParamValues(int paramset_id, const Variables &vars);
	int newParamSet(int algo_inst_id, int result_id);
	int newResult(const EvaluationResult &result);
	int newAlgoInstance(const std::string &ticker, const std::string &name, const DateTime &dtFirst, const DateTime &dtLast);

private:
	void insert_ignore_ticker(const std::string &ticker);
	void insert_ignore_param_name(const std::string &name);
	void insert_ignore_algorithm(const std::string &algoName);

	int get_ticker_id(const std::string &ticker);
	int get_param_name_id(const std::string &name);
	int get_algorithm_id(const std::string &algoName);
	int get_default_paramset_id(const std::string &ticker, const std::string &algoName);
	int get_opt_paramset_id(const std::string &ticker, const std::string &algoName);
	int get_algo_inst_id(const std::string &ticker, const std::string &algoName);

	void populate_db_schema();
};

#endif // PARAMETERSTORE_H
