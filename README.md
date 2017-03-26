Project Airis
=============

Algorithmic Trading Engine
----------------------------

Algorithmic trading using both technical and fundamental analysis with stock-specific optimized parameters obtained via machine learning.

History
-------

In October of 2016, Nathan Petersen started this project. On March 25, 2017, he deleted the history of this repository because of space issues due to poor `.gitignore` rules early on. For record keeping, note that the initital commit date was originally Oct 19, 2016.

Requirements
-------------

Compiling the CPP program:
* [TBB](https://www.threadingbuildingblocks.org/) (On Ubuntu, install TBB using `sudo apt install libtbb-dev`)
* [SQLite](https://www.sqlite.org/) (On Ubuntu, install SQLite3 using `sudo apt install libsqlite3-dev`)

Running the script to download stock data:
* [Python](https://www.python.org/)
* [Requests](http://python-requests.org/)

Installation
------------

    git clone https://github.com/npetersen2/airis.git
    cd airis
    make

Running
-------

To run all aspects of program, run the following script. If no tickers are specified, the entire S&P500 list will be used.
`./driver.sh [TICKER LIST]`

Downloading stock history:
* `python tradier-sqlite.py [-v] <TICKERS LIST>`

Optimizing, running, and computing signals:
* `./airis --help`
* `./airis [--tbb] opt <TICKER LIST>`
* `./airis [--verbose] run <TICKER LIST>`
* `./airis sig <TICKER LIST>`
