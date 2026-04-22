

#pragma once

// TODO
#ifndef __IMPL_TJSON_COMPILE_LIB__INTERNAL
	#error "only use at compile time"
#endif

// make utf8proc allocating functions not allowed

#ifdef UTF8PROC_H

	#pragma GCC poison utf8proc_map_custom

#endif

#ifdef TJSON_ALLOCATOR_PREFIX

	#include TJSON_ALLOCATOR_INCLUDE

	#define TJSON_ALLOC_ALLOCATORS_PASTE(a, b) a##b
	#define TJSON_ALLOC_ALLOCATORS_EXPAND(a, b) TJSON_ALLOC_ALLOCATORS_PASTE(a, b)

	#define TJSON_MALLOC(sz) TJSON_ALLOC_ALLOCATORS_EXPAND(TJSON_ALLOCATOR_PREFIX, _malloc)(sz)
	#define TJSON_CALLOC(n, sz) \
		TJSON_ALLOC_ALLOCATORS_EXPAND(TJSON_ALLOCATOR_PREFIX, _calloc)(n, sz)
	#define TJSON_REALLOC(p, sz) \
		TJSON_ALLOC_ALLOCATORS_EXPAND(TJSON_ALLOCATOR_PREFIX, _realloc)(p, sz)
	#define TJSON_FREE(p) TJSON_ALLOC_ALLOCATORS_EXPAND(TJSON_ALLOCATOR_PREFIX, _free)(p)

#else

	#include <stdlib.h>

	#define TJSON_MALLOC(sz) malloc(sz)
	#define TJSON_CALLOC(n, sz) calloc(n, sz)
	#define TJSON_REALLOC(p, sz) realloc(p, sz)
	#define TJSON_FREE(p) free(p)

#endif

// tvec.h and tmap.h libraries
#define T_MALLOC(sz) TJSON_MALLOC(sz)
#define T_CALLOC(n, sz) ERROR_DONT_USE_CALLOC_T_GENERIC
#define T_REALLOC(p, sz) TJSON_REALLOC(p, sz)
#define T_FREE(p) TJSON_FREE(p)

#define T_VEC_MALLOC(sz) T_MALLOC(sz)
#define T_VEC_CALLOC(n, sz) T_CALLOC(n, sz)
#define T_VEC_REALLOC(p, sz) T_REALLOC(p, sz)
#define T_VEC_FREE(p) T_FREE(p)

#define T_MAP_MALLOC(sz) T_MALLOC(sz)
#define T_MAP_CALLOC(n, sz) TJSON_CALLOC(n, sz)
#define T_MAP_REALLOC(p, sz) ERROR_DONT_USE_REALLOC_TMAP
#define T_MAP_FREE(p) T_FREE(p)

// trc.h library

#define TRC_MALLOC(sz) TJSON_MALLOC(sz)
#define TRC_CALLOC(n, sz) ERROR_DONT_USE_CALLOC_TRC
#define TRC_REALLOC(p, sz) ERROR_DONT_USE_REALLOC_TRC
#define TRC_FREE(p) TJSON_FREE(p)
