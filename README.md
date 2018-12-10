
# C++17/20 solutions to Advent of Code 2018 #

This repository contains my solutions to [Advent of Code 2018](https://adventofcode.com/2018/). The code is written in C++17, making use of some libraries implementing C++20 features.

The source file(s) for each day are in their own directories. There is no build system or anything like that: just `cd` to a directory and compile using the command line. The solutions have been tested with GCC 8 and Clang 7. They may or may not work with MSVC.

To save typing, the file `common.hpp` in this directory `#include`s many headers which are re-used in most of the solutions. You may wish to precompile this file to improve your build times.

## Libraries ##

 * [**NanoRange**](https://github.com/tcbrindle/nanorange)
   - Implementation of the C++20 `std::ranges` proposals
   - Author: me
   - Licence: Boost

* [**{fmt}**](https://github.com/fmtlib/fmt)
   - String formatting and printing that's appoximately a billion times better than iostreams
   - Author: Victor Zverovich
   - Licence: BSD

* [date](https://github.com/HowardHinnant/date)
   - Extension of `std::chrono` to handle dates and time zones
   - Author: Howard Hinnant
   - Licence: MIT