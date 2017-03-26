#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "datetime.h"

class Order {
public:
	std::string ticker;
	int numShares;
	double limitPrice;
	std::string type;
	DateTime dt;

	friend std::ostream& operator<< (std::ostream &os, const Order &o) {
		os << "(ticker:" << o.ticker << ", "
			<< "numShares:" << o.numShares << ", " 
			<< "limitPrice:" << o.limitPrice << ", "
			<< "type:" << o.type << ", "
			<< "dt:" << o.dt << ")";

		return os;
	}
};


#endif // ORDER_H
