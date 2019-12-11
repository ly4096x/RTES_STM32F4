//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// These functions are redefined locally, to avoid references to some
// heavy implementations in the standard C++ library.

// ----------------------------------------------------------------------------

#include <cstdlib>
#include <sys/types.h>

extern "C"
void FailAndInfiniteLoop(void);

// ----------------------------------------------------------------------------

namespace __gnu_cxx {
void __attribute__((noreturn)) __verbose_terminate_handler();

void __verbose_terminate_handler() { FailAndInfiniteLoop(); }
}  // namespace __gnu_cxx

// ----------------------------------------------------------------------------

extern "C" {
void __attribute__((noreturn)) __cxa_pure_virtual();

void __cxa_pure_virtual() { FailAndInfiniteLoop(); }
}

// ----------------------------------------------------------------------------

