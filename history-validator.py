from __future__ import print_function
import sys
import sqlite3

def main():
	if len(sys.argv) is 1:
		print("Usage: python history-validator.py <HISTORY-DB-FILE.db>")
		quit()

	con = sqlite3.connect(sys.argv[1])
	c = con.cursor()

	sql = "SELECT DISTINCT `ticker` FROM `History` ORDER BY `ticker` ASC"
	c.execute(sql)

	numErrors = 0
	cleanTickers = []

	tickers = c.fetchall()
	for t in tickers:
		# check validity of history
		sql = "SELECT * FROM `HISTORY` WHERE `ticker` = ? ORDER BY `datetime` ASC"
		c.execute(sql, t)
		history = c.fetchall()
		e = validate(t[0], history)
		if e is True:
			numErrors += 1
		else:
			cleanTickers.append(t[0])

	print(str(numErrors) + " error stock(s) out of " + str(len(tickers)) + " total stocks")
	print("CLEAN TICKERS:")
	print(" ".join(cleanTickers))


def validate(ticker, history):
	print("Validating: " + ticker)

	errorFound = False

	# check for $0 or NaN entries
	for day in history:
		day = rowToDict(day)

		# NaN entries
		if (day["open"] == "NaN") or (day["high"] == "NaN") or (day["low"] == "NaN") or (day["close"] == "NaN") or (day["volume"] == "NaN"):
			print("\tError: NaN value on " + day["datetime"])
			errorFound = True

		# $0 entries
		if (day["open"] == 0) or (day["high"] == 0) or (day["low"] == 0) or (day["close"] == 0):
			print("\tError: $0 value on " + day["datetime"])
			errorFound = True



	# check for price changing by a lot in one day
	for i in range(1, len(history) - 1):
		prev = rowToDict(history[i - 1])
		curr = rowToDict(history[i])

		factor = 1.9

		eO = checkDiffFor("open", factor, prev, curr)
		eH = checkDiffFor("high", factor, prev, curr)
		eL = checkDiffFor("low", factor, prev, curr)
		eC = checkDiffFor("close", factor, prev, curr)

		if eO or eH or eL or eC:
			print("\tError: >= " + str(factor) + "x diff for", end='')
			if eO: print(" 'open'", end='')
			if eH: print(" 'high'", end='')
			if eL: print(" 'low'", end='')
			if eC: print(" 'close'", end='')
			print(" going to " + curr["datetime"])
			errorFound = True

	return errorFound


def checkDiffFor(key, factor, prev, curr):
	"more than [factor]x smaller/larger today => BAD"

	# ignore cases of NaN... they are caught elsewhere
	if prev[key] == "NaN" or curr[key] == "NaN":
		return False

	if abs(prev[key] / curr[key]) > factor or abs(curr[key] / prev[key]) > factor:
		return True
	else:
		return False


def rowToDict(row):
	ret = {}
	ret["ticker"] = row[0]
	ret["datetime"] = row[1]
	ret["open"] = row[2]
	ret["high"] = row[3]
	ret["low"] = row[4]
	ret["close"] = row[5]
	ret["volume"] = row[6]
	return ret

if __name__ == "__main__":
	main()
