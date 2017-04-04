#ifndef CLAS_H
#define CLAS_H

#include <tclap/CmdLine.h>

class CLAs {
public:
	CLAs(int argc, char **argv);
	void helpMessage(char *prog);

	std::vector<std::string> tickers;
	std::string action;
	bool tbb;
	bool verbose;

};

#endif // CLAS_H
