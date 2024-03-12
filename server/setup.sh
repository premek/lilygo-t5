#!/usr/bin/env bash

sudo apt install python3-pip cairosvg libopenjp2-7 black pylint

#python3 -m venv venv
#venv/bin/python -m pip install -r requirements.txt
/usr/bin/env python3 -m pip install -r requirements.txt

./test.sh &&
   echo "All tests OK" &&
   echo 'setup done, run `nohup ./server.py`'
