# YOMM11 IS DEPRECATED

Thank you for the stars and the feedback!

In 2017 I learned the D language, and I
implemented
[an open method library for D](https://github.com/jll63/openmethods.d).  In the
process I had a few fresh ideas, some of which are applicable to C++. Also, my
colleague David Goffredo educated me on the power of modern preprocessor
macros.

Consequently, I set out to re-implement Yomm11 from scratch. The result
is [Yomm2](https://github.com/jll63/yomm2), which I feel is a much better
library. Read about the
improvements
[here](https://github.com/jll63/yomm2/blob/master/yomm11-yomm2.md).

I will no longer actively develop Yomm11 but I will consider PRs. I strongly
recommend switching to Yomm2 though.

## Old content:

This library implements open multi-methods for C++11.

Salient features are:

* syntax: is relatively uncluttered. There are no limitations on the
  number of virtual arguments. Virtual and non-virtual arguments can
  be arbitrarily mixed. Virtual and multiple inheritance are
  supported.

* speed: close to a virtual function call when the hierarchies
  involved in the virtual arguments collaborate with the
  library. Calling a method that does nothing, with a single virtual
  argument in a single inheritance hierarchy is 33% slower than the
  equivalent virtual function call. The difference becomes
  unnoticeable if the functions perform a few simple maths
  operations. See tests/benchmarks.cpp.

* size: dispatch tables are constructed in terms of class groups. This
  results in a tables devoid of redundancies.

* support for "foreign" class hierarchies: the library can be used
  without modifications to existing classes, at the cost of lower
  performance. Collaborating and foreign arguments can be freely
  mixed. Performance is still quite good, see the benchmarks.

* next: a pointer to the next most specialized method is available
  inside method specializations - see examples/next.cpp.
  Alternatively, it is possible to call a specialization directly.

Documentation: http://www.yorel.be/mm/ - see also the articles on Code
Project http://tinyurl.com/m8kg2y3

Support and discussions: yomm11 on Google Groups
(https://groups.google.com/forum/#!forum/yomm11)

Author: Jean-Louis Leroy - jl@yorel.be
