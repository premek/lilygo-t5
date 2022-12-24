#!/usr/bin/env bash

cd "${0%/*}" # cd to script dir

/usr/bin/env python3 -m black . &&
/usr/bin/env python3 -m pylint *.py &&

#/usr/bin/env python3 -m coverage run -m unittest &&
#/usr/bin/env python3 -m coverage html &&
#/usr/bin/env python3 -m coverage report && 

true

