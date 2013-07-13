# animals.hpp
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

### Makefile ---
# -*- compile_command: "make runtests" -*-

## Author: jll@yorel.be
## Version: $Id: Makefile,v 0.0 2013/07/06 07:34:35 jll Exp $
## Keywords: 
## X-URL: 

CXXFLAGS=-std=c++11 -I../include

.PHONY: runtests

runtests : tests
	./tests

tests : tests.o multimethods.o
	g++ tests.o multimethods.o -o tests

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

tests.o : tests.cpp ../include/multimethods.hpp animals.hpp mi.hpp

multimethods.o : ../src/multimethods.cpp ../include/multimethods.hpp
	$(CXX) $(CXXFLAGS) -c $<

### Makefile ends here
