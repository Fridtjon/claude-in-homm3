// MinGW compatibility for MSVC-targeted HOMM3 modding headers
#ifndef MINGW_COMPAT_H
#define MINGW_COMPAT_H

#ifdef __GNUC__

// __intN must be macros (not typedefs) so "unsigned __int32" works
#ifndef __int8
#define __int8  char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int64
#define __int64 long long
#endif

// Ensure NULL is available (patcher_x86.hpp uses it in templates)
#include <cstddef>

// MSVC __asm{} blocks — replace with GCC equivalents
// patcher_x86.hpp uses __asm{__asm int 3} as a debug break
// GCC equivalent: __builtin_trap() or asm("int $3")
// We'll handle this by wrapping __asm usage. The patcher_x86.hpp
// only uses it in a couple of template methods we won't call directly.

#endif // __GNUC__

#endif // MINGW_COMPAT_H
