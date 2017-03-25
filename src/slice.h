#ifndef SLICE_H
#define SLICE_H

class Slice {
public:
	int index;

	double open;
	double close;

	double high;
	double low;

	int volume;

	friend std::ostream& operator<< (std::ostream &os, const Slice &s) {
		os << "index:" << s.index << ", "
			<< "open:" << s.open << ", " 
			<< "high:" << s.high << ", "
			<< "low:" << s.low << ", "
			<< "close:" << s.close << ", "
			<< "volume:" << s.volume << ", ";

		return os;
	}



};


#endif // SLICE_H
