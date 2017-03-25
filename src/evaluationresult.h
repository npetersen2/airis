#ifndef EVALUATIONRESULT_H
#define EVALUATIONRESULT_H

class EvaluationResult {
public:
	double totalPercentGain;
	double avgPercentGainPerTotalTime;
	double avgPercentGainPerHold;

	int numTransactions;

	double minHoldPeriod;
	double maxHoldPeriod;
	double avgHoldPeriod;


	friend std::ostream& operator<< (std::ostream &os, const EvaluationResult &r) {
		os << "(tPG:" << r.totalPercentGain << ", "
			<< "aPGPTT:" << r.avgPercentGainPerTotalTime << ", " 
			<< "aPGPH:" << r.avgPercentGainPerHold << ", "
			<< "nT:" << r.numTransactions << ", "
			<< "minHP:" << r.minHoldPeriod << ", "
			<< "maxHP:" << r.maxHoldPeriod << ", "
			<< "avgHP:" << r.avgHoldPeriod << ")";

		return os;
	}
};


#endif // EVALUATIONRESULT_H
