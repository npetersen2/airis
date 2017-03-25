#!/bin/bash
cd "$(dirname "$0")"

echo "Good morning!"
echo ""

echo "Downloading latest data from Tradier for specified tickers..."
python tradier-sqlite.py -v $@
echo ""


echo "Optimizing algorithms for specified tickers..."
./airis --tbb opt $@
echo ""


echo "Computing signals for specified tickers..."
./airis run $@
