# -*- compile_command: "make runtests" -*-

# Makefile
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

## Author: jll@yorel.be
## Version: $Id: Makefile,v 0.0 2013/07/06 07:34:35 jll Exp $
## Keywords: 
## X-URL: 

CXXFLAGS+=-std=c++11 -I../include -g

.PHONY: runtests runall

runtests : tests
	./tests

runall : tests order12 order21
	./tests
	./order12
	./order21

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

tests : tests.o multimethods.o
	g++ tests.o multimethods.o -o tests

tests.o : tests.cpp ../include/multimethods.hpp animals.hpp mi.hpp

order12 : order1.o order2.o multimethods.o
	g++ order1.o order2.o multimethods.o -o order12

order21 : order1.o order2.o multimethods.o
	g++ order2.o order1.o multimethods.o -o order21

order1.o : order1.cpp order.hpp ../include/multimethods.hpp

order2.o : order1.cpp order.hpp ../include/multimethods.hpp

benchmarks : benchmarks.o benchmarks_fast.o multimethods.o
	g++ benchmarks.o benchmarks_fast.o  multimethods.o -o benchmarks

benchmarks.o : benchmarks.cpp benchmarks.hpp ../include/multimethods.hpp
	$(CXX) $(CXXFLAGS) -O2 -c $<

multimethods.o : ../src/multimethods.cpp ../include/multimethods.hpp
	$(CXX) $(CXXFLAGS) -c $<

### Makefile ends here
