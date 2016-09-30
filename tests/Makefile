CXX ?= g++
LIBS ?=-lm
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -pedantic -g -ggdb -O0
#-D_GLIBCXX_DEBUG
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../include
DEFINES ?=


.PHONY: test clean

test: test_odeint_anyode
	./test_odeint_anyode --abortx 1
	python3 ./_test_odeint_anyode.py

clean:
	rm catch.hpp
	rm test_odeint_anyode

test_%: test_%.cpp ../include/odeint_anyode.hpp catch.hpp testing_utils.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LIBS)

catch.hpp: catch.hpp.bz2
	bunzip2 -k -f $<
