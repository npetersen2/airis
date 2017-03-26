#ifndef DATETIME_H
#define DATETIME_H

#include <math.h>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

class DateTime {

public:
	DateTime() {
		set(0, 0, 0, 0, 0, 0);
	}

	DateTime(int year, int month, int day, int hours, int minutes, int seconds) {
		set(year, month, day, hours, minutes, seconds);
	}

	DateTime(const std::string d) {
		set(0, 0, 0, 0, 0, 0);

		size_t numSpaces = std::count(d.begin(), d.end(), ' ');

		// assume "YYYY-MM-DD" format
		if (numSpaces == 0) {
			setYMD(d);
			return;
		}

		if (numSpaces == 1) {
			// assume "YYYY-MM-DD HH:MM:SS" format
			std::istringstream iss(d);

			std::vector<std::string> tokens;
			std::copy(std::istream_iterator<std::string>(iss),
				std::istream_iterator<std::string>(),
				std::back_inserter(tokens));

			if (tokens.size() == 2) {
				setYMD(tokens.at(0));
				setHMS(tokens.at(1));
				return;
			}

			throw std::invalid_argument("date must be of format YYYY-MM-DD HH:MM:SS");
		}
	}

	void set(int year, int month, int day, int hours, int minutes, int seconds) {
		if (year != -1 && month != -1 && day != -1) {
			int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

			// account for leap year
			if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) {
				leap = true;
				days_in_month[1] = 29; // adjust february days
			} else {
				leap = false;
			}

			this->dt = 1000 * year;
			for (int i = 0; i < month - 1; i++) {
				this->dt += days_in_month[i];
			}
			this->dt += day;

			this->year = year;
			this->month = month;
			this->day = day;
		}

		if (hours != -1 && minutes != -1 && seconds != -1) {
			int seconds_of_day = (60 * 60 * hours) + (60 * minutes) + seconds;
			int total_seconds = 24 * 60 * 60;

			this->dt += (double)seconds_of_day / (double)total_seconds;

			this->hours = hours;
			this->minutes = minutes;
			this->seconds = seconds;
		}
	}

	DateTime subtractOneDay() {
		int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (leap) days_in_month[1] = 29;

		if (month == 1 && day == 1) {
			// go to prev year
			DateTime ret(year - 1, 12, 31, hours, minutes, seconds);
			return ret;
		} else {
			double prevDt = dt - 1; // prev day in year
			
			int doy = floor(prevDt - (year * 1000));
			int newMonth = 0;
			while (doy > 31) {
				doy -= days_in_month[newMonth++];
			}

			int newDay = doy;

			DateTime ret(year, newMonth, newDay, hours, minutes, seconds);
			return ret;
		}
	}

	int getYear()    { return this->year;    }
	int getMonth()   { return this->month;   }
	int getDay()     { return this->day;     }
	int getHours()   { return this->hours;   }
	int getMinutes() { return this->minutes; }
	int getSeconds() { return this->seconds; }


	std::string to_string() const {
		std::string ret;
		ret += prependZero(year) + std::string("-") + prependZero(month) + std::string("-") + prependZero(day);
		ret += " ";
		ret += prependZero(hours) + std::string(":") + prependZero(minutes) + std::string(":") + prependZero(seconds);
		return ret;
	
	}

	friend std::ostream& operator<< (std::ostream &os, const DateTime &dt) {
		os << dt.to_string();
		return os;
	}


	friend bool operator== (const DateTime &d1, const DateTime &d2) { return d1.dt == d2.dt; }
	friend bool operator!= (const DateTime &d1, const DateTime &d2) { return d1.dt != d2.dt; }
	friend bool operator>= (const DateTime &d1, const DateTime &d2) { return d1.dt >= d2.dt; }
	friend bool operator<= (const DateTime &d1, const DateTime &d2) { return d1.dt <= d2.dt; }
	friend bool operator> (const DateTime &d1, const DateTime &d2) { return d1.dt > d2.dt; }
	friend bool operator< (const DateTime &d1, const DateTime &d2) { return d1.dt < d2.dt; }

private:
	// assume ymd is in form "YYYY-MM-DD"
	void setYMD(const std::string &ymd) {
		std::istringstream ss(ymd);

		std::string token;
		std::vector<int> values;
		while (std::getline(ss, token, '-')) {
			values.push_back(std::stoi(token));
		}

		if (values.size() == 3) {
			// assume "YYYY-MM-DD" format
			set(values[0], values[1], values[2], -1, -1, -1);
			return;
		}

		throw std::invalid_argument("date must be of format YYYY-MM-DD");
	}

	// assume hms is in form HH:MM:SS
	void setHMS(const std::string &hms) {			
		std::istringstream ss(hms);

		std::string token;
		std::vector<int> values;
		while (std::getline(ss, token, ':')) {
			values.push_back(std::stoi(token));
		}

		if (values.size() == 3) {
			// assume "HH:MM:SS" format
			set(-1, -1, -1, values[0], values[1], values[2]);
			return;
		}

		throw std::invalid_argument("date must be of format HH:MM:SS");
	}

	std::string prependZero(int num) const {
		std::string ret;
		if (0 <= num && num < 10) {
			ret += "0";
		}

		ret += std::to_string(num);
		return ret;
	}

	double dt;

	bool leap;

	int year;
	int month;
	int day;
	int hours;
	int minutes;
	int seconds;
};

#endif // DATETIME_H
