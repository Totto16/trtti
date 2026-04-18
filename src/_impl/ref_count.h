#pragma once

// NOTE: this RC implementation is NOT MT thread safe!

#define RC_TYPENAME(T) rc_##T
#define RC_MALLOC_NAME(T) rc_##T##_malloc
#define RC_RELEASE_NAME(T) rc_##T##_release
#define RC_FREE_NAME(T) rc_##T##_free
#define RC_ACQUIRE_NAME(T) rc_##T##_acquire
#define RC_IMPL_STRUCT_NAME(T) rc_##T##_rc_struct_impl
#define DESTRUCTOR_FN_NAME(T) rc_##T##_destructor_name
#define RC_STRUCT_RC_ENTRY_NAME(T) rc_##T##_rc_struct_entry_impl
#define RC_STRUCT_DATA_ENTRY_NAME(T) rc_##T##_data_struct_entry_impl
#define RC_GET_DATA(T) rc_##T##_get_data
#define RC_GET_RC(T) rc_##T##_get_rc

#define RC_FUN_ATTRIBUTES RC_MAYBE_UNUSED static inline

#define RC_STR(x) #x
#define RC_XSTR(x) RC_STR(x)
#define RC_POISON(x) _Pragma(RC_XSTR(GCC poison x))

#define RC_DEFINE_TYPE(T) \
	typedef void (*DESTRUCTOR_FN_NAME(T))(T * data); \
\
	typedef struct { \
		size_t count; \
		DESTRUCTOR_FN_NAME(T) destroy; \
	} RC_IMPL_STRUCT_NAME(T); \
\
	static_assert((sizeof(RC_IMPL_STRUCT_NAME(T)) % 8) == 0); \
\
	typedef struct { \
		RC_IMPL_STRUCT_NAME(T) RC_STRUCT_RC_ENTRY_NAME(T); \
		T RC_STRUCT_DATA_ENTRY_NAME(T); \
	} RC_TYPENAME(T); \
\
	static_assert((sizeof(RC_TYPENAME(T)) % 8) == 0); \
\
	RC_NODISCARD RC_FUN_ATTRIBUTES T* RC_GET_DATA(T)(RC_TYPENAME(T) * value) { \
		return &(value->RC_STRUCT_DATA_ENTRY_NAME(T)); \
	} \
\
	RC_NODISCARD RC_FUN_ATTRIBUTES RC_TYPENAME(T) * RC_GET_RC(T)(T * data) { \
		static_assert((offsetof(RC_TYPENAME(T), RC_STRUCT_DATA_ENTRY_NAME(T)) % 8) == 0); \
		return (RC_TYPENAME(T)*)(( \
		    void*)(((uint8_t*)data) - offsetof(RC_TYPENAME(T), RC_STRUCT_DATA_ENTRY_NAME(T)))); \
	} \
\
	RC_NODISCARD RC_FUN_ATTRIBUTES T* RC_MALLOC_NAME(T)(DESTRUCTOR_FN_NAME(T) destructor) { \
		RC_TYPENAME(T)* result = malloc(sizeof(RC_TYPENAME(T))); \
		if(result == NULL) { \
			return NULL; \
		} \
		result->RC_STRUCT_RC_ENTRY_NAME(T).count = 0; \
		result->RC_STRUCT_RC_ENTRY_NAME(T).destroy = destructor; \
\
		T* data = RC_GET_DATA(T)(result); \
		return data; \
	} \
\
	RC_NODISCARD RC_FUN_ATTRIBUTES T* RC_ACQUIRE_NAME(T)(T * data) { \
		RC_TYPENAME(T)* value = RC_GET_RC(T)(data); \
		value->RC_STRUCT_RC_ENTRY_NAME(T).count++; \
\
		return data; \
	} \
\
	RC_FUN_ATTRIBUTES void RC_FREE_NAME(T)(RC_TYPENAME(T) * value) { \
		T* data = RC_GET_DATA(T)(value); \
\
		value->RC_STRUCT_RC_ENTRY_NAME(T).destroy(data); \
		free(value); \
	} \
\
	RC_FUN_ATTRIBUTES void RC_RELEASE_NAME(T)(T * data) { \
		RC_TYPENAME(T)* value = RC_GET_RC(T)(data); \
		volatile size_t* count = &(value->RC_STRUCT_RC_ENTRY_NAME(T).count); \
		/* not referenced, so just free it*/ \
		if(*count == 0) { \
			RC_FREE_NAME(T)(value); \
			return; \
		} \
		*count--; \
		/* last reference released, free it */ \
		if(*count == 0) { \
			RC_FREE_NAME(T)(value); \
			return; \
		} \
	} \
\
	RC_POISON(DESTRUCTOR_FN_NAME(T)) \
	RC_POISON(RC_TYPENAME(T)) \
	RC_POISON(RC_STRUCT_RC_ENTRY_NAME(T)) \
	RC_POISON(RC_STRUCT_DATA_ENTRY_NAME(T)) \
	RC_POISON(RC_GET_DATA(T)) \
	RC_POISON(RC_GET_RC(T)) \
	RC_POISON(RC_FREE_NAME(T)) \
	RC_POISON(RC_IMPL_STRUCT_NAME(T))

#define RC_MALLOC(T, destructor) RC_MALLOC_NAME(T)(destructor)
#define RC_ACQUIRE(T, data) RC_ACQUIRE_NAME(T)(data)
#define RC_RELEASE(T, data) RC_RELEASE_NAME(T)(data)
