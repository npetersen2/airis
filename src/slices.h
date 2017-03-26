#ifndef SLICES_H
#define SLICES_H

#include <vector>

#include "slice.h"
#include "datetime.h"

class Slices : public std::vector<std::pair<DateTime, Slice>> {
public:
	unsigned int indexOf(const DateTime &dt) {
		for (unsigned int i = 0; i < size(); i++) {
			if (this->at(i).first == dt) {
				return i;
			}
		}

		return -1;
	}

	DateTime last() const {
		if (size() == 0) {
			throw std::out_of_range("Slices has size of 0. Cannot get last DateTime.");
		}

		return rbegin()->first;
	}
};

#endif // SLICES_H
