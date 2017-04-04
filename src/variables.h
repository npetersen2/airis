#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <map>

class Variables : public std::map<std::string, double> {
public:
	friend std::ostream& operator<< (std::ostream &os, const Variables &v) {
		for (auto it = v.begin(); it != v.end(); it++) {
			os << (*it).first << ":" << (*it).second;

			if (++it != v.end()) {
				// not last element
				--it; // put it back
				os << ", ";
			} else {
				--it;
			}
		}

		return os;
	}


};

#endif // VARIABLES_H
