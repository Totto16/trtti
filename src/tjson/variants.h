#pragma once

#if !defined(_TJSON_USE_VARIANT_IMPL)
	#error "'_TJSON_USE_VARIANT_IMPL' needs to be defined"
#elif _TJSON_USE_VARIANT_IMPL == 1
	#if !defined(_TJSON_IMPL_NEED_VARIANTS)
		#error "'_TJSON_IMPL_NEED_VARIANTS' needs to be defined"
	#elif _TJSON_IMPL_NEED_VARIANTS == 2
		#include "json_variants.h"
	#elif _TJSON_IMPL_NEED_VARIANTS == 3
		#include "json_schema_variants.h"
	#else
		#error "_TJSON_USE_VARIANT_IMPL has to be 2 or 3"
	#endif
#elif _TJSON_USE_VARIANT_IMPL == 0
	#ifndef TJSON_ALLOW_BOOTSTRAP
		#error "tjson bootstrap not allowed"
	#else
		#if !defined(_TJSON_IMPL_NEED_VARIANTS)
			#error "'_TJSON_IMPL_NEED_VARIANTS' needs to be defined"
		#elif _TJSON_IMPL_NEED_VARIANTS == 2
			#include "./bootstrap_variants_json.h"
		#elif _TJSON_IMPL_NEED_VARIANTS == 3
			#include "./bootstrap_variants_schema.h"
		#else
			#error "_TJSON_USE_VARIANT_IMPL has to be 2 or 3"
		#endif
	#endif
#else
	#error "_TJSON_USE_VARIANT_IMPL has to be 0 or 1"
#endif
