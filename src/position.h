#ifndef POSITION_H
#define POSITION_H

#include <string>

class Position {
public:
	std::string ticker;
	int numShares;
	double avgPrice;

	friend std::ostream& operator<< (std::ostream &os, const Position &p) {
		os << "(ticker:" << p.ticker << ", "
			<< "numShares:" << p.numShares << ", " 
			<< "avgPrice:" << p.avgPrice << ")";

		return os;
	}
};


#endif // POSITION_H
