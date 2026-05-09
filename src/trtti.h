#pragma once

#include <stddef.h>
#include <stdint.h>

// allocators

#ifndef TRTTI_MALLOC
	#include <stdlib.h>
	#define TRTTI_MALLOC(sz) malloc(sz)
	#define TRTTI_FREE(p) free(p)
#endif

// util macros

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000) || __cplusplus
	#define TRTTI_NODISCARD [[nodiscard]]
	#define TRTTI_MAYBE_UNUSED [[maybe_unused]]
#else
    // see e.g. https://www.gnu.org/software/gnulib/manual/html_node/Attributes.html
	#define TRTTI_NODISCARD __attribute__((__warn_unused_result__))
	#define TRTTI_MAYBE_UNUSED __attribute__((__unused__))
#endif

// end of utils

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t RTTIUniqueId;

typedef struct {
	const char* ptr;
	size_t len;
} RTTITypeName;

#define TRTTI_REQUIRE_STRING_LITERAL(str) \
	(0 * \
	 sizeof( \
	     char[1][__builtin_types_compatible_p(__typeof__(str), __typeof__(&(str)[0])) ? -1 : 1]))

#define TRTTI_SIZE_OF_STR_LIT(str) ((sizeof(str) - 1) + (TRTTI_REQUIRE_STRING_LITERAL(str)))

// Macro for compile-time string literals (avoids runtime strlen).
#define TRTTI_TYPE_NAME_LIT(str) ((RTTITypeName){ .ptr = (str), .len = TRTTI_SIZE_OF_STR_LIT(str) })

#if defined(__GNUC__) && (!(defined(__clang__)))
	#define TRTTI_TYPE_NAME_LIT_CONST(str) { .ptr = (str), .len = TRTTI_SIZE_OF_STR_LIT(str) }
#else
	#define TRTTI_TYPE_NAME_LIT_CONST(str) TRTTI_TYPE_NAME_LIT(str)
#endif

#define TRTTI_TYPE_NAME_FMT "%.*s"
#define TRTTI_TYPE_NAME_FMT_ARGS(str) ((int)((str).len)), ((str).ptr)

typedef struct {
	RTTITypeName name;
	RTTIUniqueId id;
} RTTITypeInfo;

static_assert((sizeof(RTTITypeInfo) % 8) == 0);

#define TRTTI_VALUE_TYPENAME(T) __impl_struct_typename_rtti_##T
#define TRTTI_PTR_CAST_FN(T) __impl_fn_rtti_##T##_ptr_cast
#define TRTTI_GET_SHADOW_DATA(T) __impl_fn_rtti_##T##_get_shadow_data
#define TRTTI_GLOBAL_ID_DATA(T) __impl_global_data_rtti_##T##_id_data
#define TRTTI_GLOBAL_START_ID_NAME __impl_global_start_postion_for_rtti_ids_
#define TRTTI_GET_DATA(T) __impl_fn_rtti_##T##_get_data
#define TRTTI_MATCHES_TYPE_FN __impl_fn_rtti_matches_type_generic

#define TRTTI_FUN_ATTRIBUTES TRTTI_MAYBE_UNUSED static inline

#define TRTTI_STR(x) #x
#define TRTTI_XSTR(x) TRTTI_STR(x)
#define TRTTI_POISON(x) _Pragma(TRTTI_XSTR(GCC poison x))

typedef void* RTTIAnnotatedPtr;

static_assert(__builtin_classify_type(struct A1) == __builtin_classify_type(struct A2));
static_assert(__builtin_classify_type(struct A1) != __builtin_classify_type(int));

static_assert(__builtin_classify_type(struct Foo) != __builtin_classify_type(int[5]));

static_assert(__builtin_classify_type(struct Foo) != __builtin_classify_type(int[5]));
static_assert(__builtin_classify_type(void*) == __builtin_classify_type(void*));

#define TRTTI_TYPE_IDENTIFIER_DEFINTION(Type) \
	__attribute__((section(".rtti"))) const uint8_t TRTTI_GLOBAL_ID_DATA(Type) = 0;

TRTTI_TYPE_IDENTIFIER_DEFINTION(TRTTI_GLOBAL_START_ID_NAME)

#define TRTTI_GLOBAL_ID_DATA_START TRTTI_GLOBAL_ID_DATA(TRTTI_GLOBAL_START_ID_NAME)

#define RTTI_TYPE_ID(Type) \
	(RTTIUniqueId)(((uintptr_t)((&(TRTTI_GLOBAL_ID_DATA(Type))) - (&(TRTTI_GLOBAL_ID_DATA_START)))))

#define TRTTI_ID_OF_TYPE(Type)

#define TRTTI_MATCHES_TYPE(Type, info) TRTTI_MATCHES_TYPE_FN(#Type, info)

#define TRTTI_DECLARE_TYPE_AS_SUPPORTED(Type) \
	/* only support structs as RTTI types*/ \
	static_assert(__builtin_classify_type(Type) == __builtin_classify_type(RTTITypeInfo)); \
	/* Define value rtti type */ \
	typedef struct { \
		RTTITypeInfo info; \
		Type data; \
	} TRTTI_VALUE_TYPENAME(Type); \
\
	TRTTI_TYPE_IDENTIFIER_DEFINTION(Type) \
\
	static RTTITypeInfo TRTTI_STATIC_TYPEINFO(Type) = { .name = TRTTI_TYPE_NAME_LIT_CONST(#Type), \
		                                                .id = TRTTI_ID_OF_TYPE(Type) }; \
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES TRTTI_VALUE_TYPENAME(Type) * \
	    TRTTI_GET_SHADOW_DATA(Type)(RTTIAnnotatedPtr ptr) { \
		static_assert((offsetof(TRTTI_VALUE_TYPENAME(Type), value) % 8) == 0); \
		return (TRTTI_VALUE_TYPENAME(Type)*)(( \
		    void*)(((uint8_t*)data) - offsetof(TRTTI_VALUE_TYPENAME(Type), value))); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_GET_DATA(Type)(TRTTI_VALUE_TYPENAME(Type)*) { \
		return &(value->data); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_PTR_CAST_FN(Type)(RTTIAnnotatedPtr ptr) { \
		TRTTI_VALUE_TYPENAME(Type)* const data = TRTTI_GET_SHADOW_DATA(Type)(ptr); \
		if(!TRTTI_MATCHES_TYPE(Type, data->info)) { \
			TRTTI_PANIC_FOR_TYPE(Type, data->info); \
		} \
\
		return TRTTI_GET_DATA(Type); \
	}

#define TRTTI_ANNOTATED_PTR_CAST(Type, value) TRTTI_PTR_CAST_FN(T)(value)

#define RTTI_TYPE(T) \
	typedef T T; \
	enum { T##_has_rtti = 1 }

#ifdef __cplusplus
}
#endif
