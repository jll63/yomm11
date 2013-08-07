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

.PHONY: run

run : asteroids three next foreign
	./asteroids
	./three
	./next
	./foreign

asteroids : asteroids.o multi_methods.o
	$(CXX) $^ -o $@

three : three.o multi_methods.o
	$(CXX) $^ -o $@

next : next.o multi_methods.o
	$(CXX) $^ -o $@

foreign : foreign.o multi_methods.o
	$(CXX) $^ -o $@

dl_main		: dl_main.o multi_methods.o
	$(CXX) $^ -o $@ -ldl -rdynamic

dl_shared.o	: dl_shared.cpp dl.hpp ../include/multi_methods.hpp
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ dl_shared.cpp

dl_shared.so	: dl_shared.o
	$(CXX) -shared -Wl,-soname,dl_shared.so -o $@ dl_shared.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

multi_methods.o : ../src/multi_methods.cpp ../include/multi_methods.hpp
	$(CXX) $(CXXFLAGS) -c $<

