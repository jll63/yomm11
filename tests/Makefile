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

CXXFLAGS+=-std=c++11 -I../include

.PHONY: runtests runall

runtests : tests
	./tests

runall : tests order12 order21
	./tests
	./order12
	./order21

tests : tests.o multimethods.o
	g++ tests.o multimethods.o -o tests

order12 : order1.o order2.o multimethods.o
	g++ order1.o order2.o multimethods.o -o order1

order21 : order1.o order2.o multimethods.o
	g++ order2.o order1.o multimethods.o -o order2

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

tests.o : tests.cpp ../include/multimethods.hpp animals.hpp mi.hpp

multimethods.o : ../src/multimethods.cpp ../include/multimethods.hpp
	$(CXX) $(CXXFLAGS) -c $<

### Makefile ends here
