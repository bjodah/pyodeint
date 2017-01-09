CXX ?= g++
LIBS ?=-lm
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -pedantic -g -ggdb -O0
#-D_GLIBCXX_DEBUG
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../pyodeint/include -I../external/anyode/include
DEFINES ?=
OPENMP_FLAG ?= -fopenmp
OPENMP_LIB ?= -lgomp


.PHONY: test clean

test: test_odeint_anyode test_odeint_anyode_parallel test_odeint_anyode_autorestart
	./test_odeint_anyode --abortx 1
	./test_odeint_anyode_parallel --abortx 1
	./test_odeint_anyode_autorestart --abortx 1
	python3 ./_test_odeint_anyode.py

clean:
	rm catch.hpp
	rm test_odeint_anyode

test_%: test_%.cpp ../pyodeint/include/odeint_anyode.hpp catch.hpp testing_utils.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LIBS)

test_odeint_anyode_parallel: test_odeint_anyode_parallel.cpp catch.hpp ../pyodeint/include/odeint_*.hpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAG) $(INCLUDE) $(DEFINES) -o $@ $< $(LIBS) $(OPENMP_LIB)

catch.hpp: catch.hpp.bz2
	bunzip2 -k -f $<
