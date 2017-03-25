OLDEST_DATA_WANTED = "2005-01-01"

import requests
import json
import datetime
import time
import sys
import os
import sqlite3
import signal

if (len(sys.argv) == 1) or (len(sys.argv) == 2 and sys.argv[1] == "-v"):
	print "USAGE: python " + sys.argv[0] + " [-v] TICKERS..."
	sys.exit(1)


def signal_handler(signal, frame):
	print "\n"
	print "--- program ran for " + str(time.time() - start_time) + " seconds ---"
	print "Stopping..."
	sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)



print ""
print "Loading ticker history from Tradier into SQLite db..."
start_time = time.time()


if not os.path.exists("data"):
	os.makedirs("data")
con = sqlite3.connect("data/HISTORY.db")
c = con.cursor()



sys.argv.pop(0)

if sys.argv[0] == "-v":
	VERBOSE = True
	sys.argv.pop(0)
else:
	VERBOSE = False

for t in sys.argv:
	symbol = t

	# create the table
	c.execute("CREATE TABLE IF NOT EXISTS `" + symbol + "` (`datetime` TEXT PRIMARY KEY, `open` REAL NOT NULL, `high` REAL NOT NULL, `low` REAL NOT NULL, `close` REAL NOT NULL, `volume` REAL NOT NULL);")

	
	# calculate start date
	c.execute("SELECT `datetime` FROM `" + symbol + "` ORDER BY date(`datetime`) DESC LIMIT 1")
	result = c.fetchone()
	if result is None:
		start = OLDEST_DATA_WANTED
	else:
		start = result[0]
		c.execute("DELETE FROM `" + symbol + "` WHERE `datetime`= " + start) # make sure to update last entry... could be an old intraday from tradier


	# end is tomorrow (all data tradier has)
	end = datetime.date.today() + datetime.timedelta(days=1)

	if VERBOSE:
		print t + ": fetching from " + str(start) + " to " + str(end) + ":",
		sys.stdout.flush()

	payload = {
		"symbol" : symbol,
		"interval" : "daily",
		"start" : start,
		"end" : end
	}
	
	headers = {
		"Accept" : "application/json",
		"Authorization" : "Bearer 5LmOo1czg9tKD3EP98soVUfGiTBI"
	}
	
	r = requests.get("https://api.tradier.com/v1/markets/history", headers = headers, params = payload)
	
	if (r.status_code != 200):
		print "ERROR"
		print r.text
	else:
		j = json.loads(r.text)
		if j["history"] is None:
			if not VERBOSE:
				print ""
				
			print "ERROR: cannot load history for: " + symbol
			continue

		history = []
		days = j["history"]["day"]

		if type(days) is dict:
			# only one day returned
			if VERBOSE:
				print "1 row from Tradier,",

			dopen = days["open"]
			dclose = days["close"]
			dvol = days["volume"]
			dhigh = days["high"]
			dlow = days["low"]
			date = datetime.datetime.strptime(days["date"], "%Y-%m-%d")
			i = (date.strftime("%Y-%m-%d"), str(dopen), str(dhigh), str(dlow), str(dclose), str(dvol))
			history.append(i)
		else:
			if VERBOSE:
				print str(len(days)) + " rows from Tradier,",
			for day in days:
				dopen = day["open"]
				dclose = day["close"]
				dvol = day["volume"]
				dhigh = day["high"]
				dlow = day["low"]
				date = datetime.datetime.strptime(day["date"], "%Y-%m-%d")
	
				i = (date.strftime("%Y-%m-%d"), str(dopen), str(dhigh), str(dlow), str(dclose), str(dvol))
				history.append(i)

		sys.stdout.flush()

		# add the history
		c.executemany("INSERT OR IGNORE INTO `" + symbol + "` (`datetime`, `open`, `high`, `low`, `close`, `volume`) VALUES (?,?,?,?,?,?);", history)


		if VERBOSE:
			c.execute("SELECT COUNT(*) FROM `" + t + "`")
			result = c.fetchone()
			print "now have " + str(result[0]) + " rows"
		else:
			print symbol,
		sys.stdout.flush()

con.commit()
con.close()

if not VERBOSE:
	print ""
print "--- program ran for " + str(time.time() - start_time) + " seconds ---"
print ""
