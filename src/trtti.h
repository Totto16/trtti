#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

static_assert(sizeof(void*) == sizeof(uint64_t));

typedef uint64_t RTTIUniqueId;

typedef struct {
	const char* ptr;
	size_t len;
} RTTITypeName;

#ifdef __cplusplus

// close extern "C"
}

[[nodiscard]] static consteval RTTITypeName operator""_trtti_lit(const char* str, std::size_t len) {
	const RTTITypeName result = { .ptr = str, .len = len };
	return result;
}

// new extern "C"
extern "C" {

	#define TRTTI_TYPE_NAME_LIT(str) str##_trtti_lit
	#define TRTTI_TYPE_NAME_LIT_CONST(str) TRTTI_TYPE_NAME_LIT(str)

	#define TRTTI_LITERAL_IMPL(Type) Type

#else

	#define TRTTI_REQUIRE_STRING_LITERAL(str) \
		(0 * \
		 sizeof( \
		     char[1] \
		         [__builtin_types_compatible_p(__typeof__(str), __typeof__(&(str)[0])) ? -1 : 1]))

	#define TRTTI_SIZE_OF_STR_LIT(str) ((sizeof(str) - 1) + (TRTTI_REQUIRE_STRING_LITERAL(str)))

    // Macro for compile-time string literals (avoids runtime strlen).
	#define TRTTI_TYPE_NAME_LIT(str) \
		((RTTITypeName){ .ptr = (str), .len = TRTTI_SIZE_OF_STR_LIT(str) })

	#if defined(__GNUC__) && (!(defined(__clang__)))
		#define TRTTI_TYPE_NAME_LIT_CONST(str) { .ptr = (str), .len = TRTTI_SIZE_OF_STR_LIT(str) }
	#else
		#define TRTTI_TYPE_NAME_LIT_CONST(str) TRTTI_TYPE_NAME_LIT(str)
	#endif

	#define TRTTI_LITERAL_IMPL(Type) (Type)

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
#define TRTTI_PTR_IS_FN(T) __impl_fn_rtti_##T##_ptr_is
#define TRTTI_VALUE_GET_FN(T) __impl_fn_rtti_##T##_value_get
#define TRTTI_VALUE_CAST_FN(T) __impl_fn_rtti_##T##_value_cast
#define TRTTI_VALUE_IS_FN(T) __impl_fn_rtti_##T##_value_is
#define TRTTI_GET_SHADOW_DATA(T) __impl_fn_rtti_##T##_get_shadow_data
#define TRTTI_GLOBAL_ID_DATA_IMPL(T) __impl_global_data_rtti_##T##_id_data
#define TRTTI_GLOBAL_ID_DATA(T) TRTTI_GLOBAL_ID_DATA_IMPL(T)
#define TRTTI_GET_DATA(T) __impl_fn_rtti_##T##_get_data
#define TRTTI_STATIC_TYPEINFO(T) __impl_global_data_rtti_##T##_static_typeinfo
#define TRTTI_STRUCT_INFO_ENTRY(T) __impl_struct_entry_rtti_##T##_info_entry_impl
#define TRTTI_STRUCT_DATA_ENTRY(T) __impl_struct_entry_rtti_##T##_data_entry_impl
#define TRTTI_GET_TYPEINFO(T) __impl_fn_rtti_##T##_get_typeinfo_impl
#define TRTTI_ALLOC_NAME(T) rc_fn_##T##_alloc
#define TRTTI_DESTROY_NAME(T) rc_fn_##T##_destroy

#define TRTTI_MATCHES_TYPE_FN __impl_fn_rtti_matches_type_generic
#define TRTTI_PANIC_FOR_TYPE_FN __impl_fn_rtti_panic_for_type_generic

#define TRTTI_FUN_ATTRIBUTES TRTTI_MAYBE_UNUSED static inline

#define TRTTI_STR(x) #x
#define TRTTI_XSTR(x) TRTTI_STR(x)
#define TRTTI_POISON(x) _Pragma(TRTTI_XSTR(GCC poison x))

typedef void* RTTIAnnotatedPtr;

typedef struct {
	RTTITypeInfo type;
	RTTIAnnotatedPtr ptr;
} RTTIAnnotatedValue;

static_assert((sizeof(RTTIAnnotatedValue) % 8) == 0);

#if (defined(__clang__))
	#define BUILTIN_CLASSIFY_TYPE(Type) __builtin_classify_type((Type*)0)
#else
	#define BUILTIN_CLASSIFY_TYPE(Type) __builtin_classify_type(Type)

// this tests only work in GCC atm
static_assert(BUILTIN_CLASSIFY_TYPE(RTTITypeInfo) == BUILTIN_CLASSIFY_TYPE(RTTITypeName));
static_assert(BUILTIN_CLASSIFY_TYPE(RTTITypeInfo) != BUILTIN_CLASSIFY_TYPE(int));

static_assert(BUILTIN_CLASSIFY_TYPE(RTTITypeName) != BUILTIN_CLASSIFY_TYPE(int[5]));

static_assert(BUILTIN_CLASSIFY_TYPE(RTTITypeName) != BUILTIN_CLASSIFY_TYPE(int[5]));
static_assert(BUILTIN_CLASSIFY_TYPE(void*) == BUILTIN_CLASSIFY_TYPE(void*));

#endif

// the same final object?
#define TRTTI_SECTION_NAME __attribute__((section("._rtti_impl_trtti_lib_section")))

#if (defined(__clang__))
	#define TRTTI_SECTION_CUSTOM __attribute__((visibility("default")))
#else
	#define TRTTI_SECTION_CUSTOM __attribute__((visibility("default"), externally_visible))

#endif

#define TRTTI_SECTION TRTTI_SECTION_NAME TRTTI_SECTION_CUSTOM

#define TRTTI_TYPE_IDENTIFIER_DEFINTION(Type) \
	TRTTI_SECTION extern const uint8_t TRTTI_GLOBAL_ID_DATA(Type);

#define TRTTI_TYPE_IDENTIFIER_DECLARATION(Type) \
	TRTTI_SECTION const uint8_t TRTTI_GLOBAL_ID_DATA(Type) = 0;

#define TRTTI_ID_OF_TYPE(Type) (RTTIUniqueId)((uintptr_t)(&(TRTTI_GLOBAL_ID_DATA(Type))))

TRTTI_FUN_ATTRIBUTES __attribute__((noreturn)) void TRTTI_PANIC_FOR_TYPE_FN(RTTITypeInfo expected,
                                                                            RTTITypeInfo got) {
	fprintf(stderr,
	        "[%s %s:%d]: PANIC: INVALID access of rtti type " TRTTI_TYPE_NAME_FMT
	        "(%zu): got " TRTTI_TYPE_NAME_FMT "(%zu) instead\n",
	        __func__, __FILE__, __LINE__, TRTTI_TYPE_NAME_FMT_ARGS(got.name), got.id,
	        TRTTI_TYPE_NAME_FMT_ARGS(expected.name), expected.id);
	exit(EXIT_FAILURE);
}

TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES bool TRTTI_MATCHES_TYPE_FN(RTTITypeInfo expected,
                                                                RTTITypeInfo got) {
	if(expected.id != got.id) {
		// TODO(Totto): maybe check if the names would match, if they would, our id system failed
		return false;
	}
	return true;
}

#define TRTTI_DECLARE_TYPE_AS_SUPPORTED(Type) \
	/* only support structs as RTTI types*/ \
	static_assert(BUILTIN_CLASSIFY_TYPE(Type) == BUILTIN_CLASSIFY_TYPE(RTTITypeInfo)); \
	/* Define value rtti type */ \
	typedef struct { \
		RTTITypeInfo TRTTI_STRUCT_INFO_ENTRY(Type); \
		Type TRTTI_STRUCT_DATA_ENTRY(Type); \
	} TRTTI_VALUE_TYPENAME(Type); \
\
	static_assert((sizeof(TRTTI_VALUE_TYPENAME(Type)) % 8) == 0); \
\
	TRTTI_TYPE_IDENTIFIER_DEFINTION(Type) \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES RTTITypeInfo TRTTI_GET_TYPEINFO(Type)(void) { \
		static RTTITypeInfo TRTTI_STATIC_TYPEINFO( \
		    Type) = { .name = TRTTI_TYPE_NAME_LIT_CONST(#Type), .id = 0 }; \
		/* TRTTI_ID_OF_TYPE(Type)  isn't constant, as it requires final executables info alias the \
		 * address of the variable in the static memory,  so this can't be static :(*/ \
		if(TRTTI_STATIC_TYPEINFO(Type).id == 0) { \
			TRTTI_STATIC_TYPEINFO(Type).id = TRTTI_ID_OF_TYPE(Type); \
		} \
\
		return TRTTI_STATIC_TYPEINFO(Type); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES TRTTI_VALUE_TYPENAME(Type) * \
	    TRTTI_GET_SHADOW_DATA(Type)(Type * ptr) { \
		static_assert((offsetof(TRTTI_VALUE_TYPENAME(Type), TRTTI_STRUCT_DATA_ENTRY(Type)) % 8) == \
		              0); \
		return (TRTTI_VALUE_TYPENAME(Type)*)(( \
		    void*)(((uint8_t*)ptr) - \
		           offsetof(TRTTI_VALUE_TYPENAME(Type), TRTTI_STRUCT_DATA_ENTRY(Type)))); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_GET_DATA(Type)(TRTTI_VALUE_TYPENAME(Type) * \
	                                                                value) { \
		return &(value->TRTTI_STRUCT_DATA_ENTRY(Type)); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_PTR_CAST_FN(Type)(RTTIAnnotatedPtr ptr) { \
		TRTTI_VALUE_TYPENAME(Type)* const data = TRTTI_GET_SHADOW_DATA(Type)((Type*)ptr); \
		const RTTITypeInfo info = TRTTI_GET_TYPEINFO(Type)(); \
		if(!TRTTI_MATCHES_TYPE_FN(info, data->TRTTI_STRUCT_INFO_ENTRY(Type))) { \
			TRTTI_PANIC_FOR_TYPE_FN(info, data->TRTTI_STRUCT_INFO_ENTRY(Type)); \
		} \
\
		return TRTTI_GET_DATA(Type)(data); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES bool TRTTI_PTR_IS_FN(Type)(RTTIAnnotatedPtr ptr) { \
		TRTTI_VALUE_TYPENAME(Type)* const data = TRTTI_GET_SHADOW_DATA(Type)((Type*)ptr); \
		const RTTITypeInfo info = TRTTI_GET_TYPEINFO(Type)(); \
		if(TRTTI_MATCHES_TYPE_FN(info, data->TRTTI_STRUCT_INFO_ENTRY(Type))) { \
			return true; \
		} \
\
		return false; \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_VALUE_CAST_FN(Type)( \
	    RTTIAnnotatedValue value) { \
		const RTTITypeInfo info = TRTTI_GET_TYPEINFO(Type)(); \
		if(!TRTTI_MATCHES_TYPE_FN(info, value.type)) { \
			TRTTI_PANIC_FOR_TYPE_FN(info, value.type); \
		} \
\
		return (Type*)(value.ptr); \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES bool TRTTI_VALUE_IS_FN(Type)(RTTIAnnotatedValue value) { \
		const RTTITypeInfo info = TRTTI_GET_TYPEINFO(Type)(); \
		if(TRTTI_MATCHES_TYPE_FN(info, value.type)) { \
			return true; \
		} \
\
		return false; \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES RTTIAnnotatedValue TRTTI_VALUE_GET_FN(Type)(Type * ptr) { \
		const RTTITypeInfo info = TRTTI_GET_TYPEINFO(Type)(); \
\
		return TRTTI_LITERAL_IMPL(RTTIAnnotatedValue){ .type = info, .ptr = (void*)ptr }; \
	} \
\
	TRTTI_NODISCARD TRTTI_FUN_ATTRIBUTES Type* TRTTI_ALLOC_NAME(Type)(void) { \
		TRTTI_VALUE_TYPENAME(Type)* result = \
		    (TRTTI_VALUE_TYPENAME(Type)*)TRTTI_MALLOC(sizeof(TRTTI_VALUE_TYPENAME(Type))); \
		if(result == NULL) { \
			return NULL; \
		} \
		result->TRTTI_STRUCT_INFO_ENTRY(Type) = TRTTI_GET_TYPEINFO(Type)(); \
\
		Type* data = TRTTI_GET_DATA(Type)(result); \
		return data; \
	} \
	TRTTI_FUN_ATTRIBUTES void TRTTI_DESTROY_NAME(Type)(Type * value) { \
		TRTTI_VALUE_TYPENAME(Type)* data = TRTTI_GET_SHADOW_DATA(Type)(value); \
\
		TRTTI_FREE(data); \
	} \
\
	TRTTI_POISON(TRTTI_VALUE_TYPENAME(Type)) \
	TRTTI_POISON(RTTITypeInfo) \
	TRTTI_POISON(TRTTI_GET_DATA(Type)) \
	TRTTI_POISON(TRTTI_GET_SHADOW_DATA(Type)) \
	TRTTI_POISON(TRTTI_STRUCT_DATA_ENTRY(Type)) \
	TRTTI_POISON(TRTTI_STRUCT_INFO_ENTRY(Type)) \
	TRTTI_POISON(TRTTI_STATIC_TYPEINFO(Type)) \
	TRTTI_POISON(TRTTI_GET_TYPEINFO(Type))

#define TRTTI_IMPLEMENTATION_FOR_TYPE(Type) TRTTI_TYPE_IDENTIFIER_DECLARATION(Type)

#define TRTTI_DEFINE_TYPE_AS_SUPPORTED(Type) \
	TRTTI_DECLARE_TYPE_AS_SUPPORTED(Type) \
	TRTTI_IMPLEMENTATION_FOR_TYPE(Type)

#define TRTTI_ANNOTATED_PTR_CAST(Type, value) TRTTI_PTR_CAST_FN(Type)(value)
#define TRTTI_ANNOTATED_PTR_IS(Type, value) TRTTI_PTR_IS_FN(Type)(value)

#define TRTTI_ALLOC(Type) TRTTI_ALLOC_NAME(Type)()
#define TRTTI_DESTROY(Type, data) TRTTI_DESTROY_NAME(Type)(data)

#define TRTTI_ANNOTATED_VALUE_GET(Type, value) TRTTI_VALUE_GET_FN(Type)(value)
#define TRTTI_ANNOTATED_VALUE_CAST(Type, value) TRTTI_VALUE_CAST_FN(Type)(value)
#define TRTTI_ANNOTATED_VALUE_IS(Type, value) TRTTI_VALUE_IS_FN(Type)(value)

#define TRTTI_TYPE_MATCHES_OTHER_TYPE(type1, type2) TRTTI_MATCHES_TYPE_FN(type1, type2)

#ifdef __cplusplus
}
#endif
