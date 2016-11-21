#!/bin/bash -xeu
PKG_NAME=${1:-${CI_REPO##*/}}
if [[ "$CI_BRANCH" =~ ^v[0-9]+.[0-9]?* ]]; then
    eval export ${PKG_NAME^^}_RELEASE_VERSION=\$CI_BRANCH
    echo ${CI_BRANCH} | tail -c +2 > __conda_version__.txt
fi

(cd tests/; make)

python2 setup.py sdist
python2 -m pip install dist/*.tar.gz
(cd /; python2 -m pytest --pyargs $PKG_NAME)
python3 -m pip install dist/*.tar.gz
(cd /; python3 -m pytest --pyargs $PKG_NAME)
PYTHONPATH=$(pwd) PYTHON=python3 ./scripts/run_tests.sh --cov $PKG_NAME --cov-report html
./scripts/coverage_badge.py htmlcov/ htmlcov/coverage.svg

# Make sure repo is pip installable from git-archive zip
git archive -o /tmp/$PKG_NAME.zip HEAD
python3 -m pip install --force-reinstall /tmp/$PKG_NAME.zip
(cd /; python3 -c "from ${PKG_NAME} import get_include as gi; import os; assert 'odeint_anyode_nogil.pxd' in os.listdir(gi())")

! grep "DO-NOT-MERGE!" -R . --exclude ci.sh
