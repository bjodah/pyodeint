#!/bin/bash -ex
# Extract absolute path of script, from:
# http://unix.stackexchange.com/a/9546
ABSOLUTE_REPO_PATH_X="$(readlink -fn -- "$(dirname $0)/.."; echo x)"
ABSOLUTE_REPO_PATH="${ABSOLUTE_REPO_PATH_X%x}"
export PYTHONPATH=$ABSOLUTE_REPO_PATH:$PYTHONPATH
cd ${ABSOLUTE_REPO_PATH}
python2 setup.py build_ext -i
python2 -m pytest --pep8 --flakes --ignore build/ --ignore doc/
python3 setup.py build_ext -i
python3 -m pytest --ignore build/ --ignore doc/
python -m doctest README.rst
