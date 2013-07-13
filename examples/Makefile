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

CXXFLAGS=-std=c++11 -I../include

.PHONY: run

run : asteroids three next foreign
	./asteroids
	./three
	./next
	./foreign

asteroids : asteroids.o multimethods.o
	$(CXX) $^ -o $@

three : three.o multimethods.o
	$(CXX) $^ -o $@

next : next.o multimethods.o
	$(CXX) $^ -o $@

foreign : foreign.o multimethods.o
	$(CXX) $^ -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

multimethods.o : ../src/multimethods.cpp ../include/multimethods.hpp
	$(CXX) $(CXXFLAGS) -c $<

