#pragma once

#if !defined(_TJSON_USE_VARIANT_IMPL)
	#error "'_TJSON_USE_VARIANT_IMPL' needs to be defined"
#elif _TJSON_USE_VARIANT_IMPL == 1
	#include "json_variants.h"
#elif _TJSON_USE_VARIANT_IMPL == 0
	#ifndef TJSON_ALLOW_BOOTSTRAP
		#error "tjson bootstrap not allowed"
	#else
		#include "./bootstrap_variants.h"
	#endif
#else
	#error "_TJSON_USE_VARIANT_IMPL has to be 0 or 1"
#endif
