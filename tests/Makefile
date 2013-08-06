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

.PHONY: runtests run

run : tests order12 order21
	./tests
	./order12
	./order21

runtests : tests
	./tests

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

tests : tests.o multi_methods.o
	g++ tests.o multi_methods.o -o tests

tests.o : tests.cpp ../include/multi_methods.hpp animals.hpp mi.hpp adjust.hpp

order12 : order1.o order2.o multi_methods.o
	g++ order1.o order2.o multi_methods.o -o order12

order21 : order1.o order2.o multi_methods.o
	g++ order2.o order1.o multi_methods.o -o order21

order1.o : order1.cpp order.hpp ../include/multi_methods.hpp

order2.o : order1.cpp order.hpp ../include/multi_methods.hpp

benchmarks : benchmarks.o benchmarks_fast.o multi_methods.o
	g++ benchmarks.o benchmarks_fast.o  multi_methods.o -o benchmarks

benchmarks.o : benchmarks.cpp benchmarks.hpp ../include/multi_methods.hpp
	$(CXX) $(CXXFLAGS) -O2 -c $<

multi_methods.o : ../src/multi_methods.cpp ../include/multi_methods.hpp
	$(CXX) $(CXXFLAGS) -c $<

### Makefile ends here
