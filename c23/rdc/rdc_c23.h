#ifndef RDC_C23_H
#define RDC_C23_H

/*
 * Language gate for Register Drop Counter.
 * Target: ISO/IEC 9899:2024 (C23). GCC 12 accepts the same dialect as -std=c2x.
 */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "Register Drop Counter C requires ISO C11 or later; build with -std=c23, -std=c2x, or /std:clatest"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __has_c_attribute
#  if __has_c_attribute(nodiscard)
#    define RDC_NODISCARD [[nodiscard]]
#  endif
#  if __has_c_attribute(maybe_unused)
#    define RDC_UNUSED [[maybe_unused]]
#  endif
#endif
#ifndef RDC_NODISCARD
#  define RDC_NODISCARD
#endif
#ifndef RDC_UNUSED
#  define RDC_UNUSED
#endif

#ifndef RDC_NULL
#  define RDC_NULL ((void *)0)
#endif

#endif /* RDC_C23_H */
