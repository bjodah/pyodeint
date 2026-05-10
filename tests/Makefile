CXX ?= g++
EXTRA_LIBS ?=-lm
WARNINGS ?= \
-Wall \
-Wextra \
-Wredundant-decls \
-Wcast-align \
-Wmissing-include-dirs \
-Wswitch-enum \
-Wswitch-default \
-Winvalid-pch \
-Wredundant-decls \
-Wformat=2 \
-Wmissing-format-attribute \
-Wformat-nonliteral \
-Wodr
CXXFLAGS ?= -std=c++20 $(WARNINGS) -Werror -pedantic -g -ggdb -O0
#-D_GLIBCXX_DEBUG
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../pyodeint/include -I../external/anyode/include
DEFINES ?=
OPENMP_FLAG ?= -fopenmp
OPENMP_LIB ?= -lgomp


.PHONY: test clean

test: test_odeint_anyode test_odeint_anyode_parallel test_odeint_anyode_autorestart
	env DISTUTILS_DEBUG=1 CC=$(CXX) CFLAGS="$(EXTRA_FLAGS)" LDFLAGS="$(LDFLAGS)" LD_PRELOAD="$(PY_LD_PRELOAD)" ASAN_OPTIONS=detect_leaks=0 python3 ./_test_odeint_anyode.py
	./test_odeint_anyode --abortx 1
	./test_odeint_anyode_parallel --abortx 1
	./test_odeint_anyode_autorestart --abortx 1

clean:
	rm -f doctest.h
	rm -f test_odeint_anyode
	rm -f test_odeint_anyode_parallel
	rm -f test_odeint_anyode_autorestart

test_%: test_%.cpp ../pyodeint/include/odeint_anyode.hpp doctest.h testing_utils.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LDLIBS) $(EXTRA_LIBS)

test_odeint_anyode_parallel: test_odeint_anyode_parallel.cpp doctest.h ../pyodeint/include/odeint_*.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OPENMP_FLAG) $(INCLUDE) $(DEFINES) -o $@ $< $(LDLIBS) $(EXTRA_LIBS) $(OPENMP_LIB)

doctest.h: doctest.h.bz2
	bunzip2 -k -f $<
