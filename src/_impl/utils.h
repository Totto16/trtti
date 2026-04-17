
#pragma once

#ifndef _TJSON_IMPL_INTERNAL__
	#error "can only be used internally"
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if _TJSON_COMPILE_WITH_NARROWED_ENUMS
	#define C_23_NARROW_ENUM_TO(x) : x
	#define C_23_ENUM_TYPE(x) x
#else
	#define C_23_NARROW_ENUM_TO(x)
	#define C_23_ENUM_TYPE(x) int
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000) || __cplusplus
	#define NODISCARD [[nodiscard]]
	#define MAYBE_UNUSED [[maybe_unused]]
#else
    // see e.g. https://www.gnu.org/software/gnulib/manual/html_node/Attributes.html
	#define NODISCARD __attribute__((__warn_unused_result__))
	#define MAYBE_UNUSED __attribute__((__unused__))
#endif

// cool trick from here:
// https://stackoverflow.com/questions/777261/avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
#ifdef NDEBUG
	#define ASSERT(x) \
		do { \
			UNUSED((x)); \
		} while(false)

	#ifndef assert
		#define assert(x) ASSERT(x) /* NOLINT(readability-identifier-naming) */
	#endif

	#define UNREACHABLE() \
		do { \
			fprintf(stderr, "[%s %s:%d]: UNREACHABLE\n", __func__, __FILE__, __LINE__); \
			exit(EXIT_FAILURE); \
		} while(false)

	#define OOM_ASSERT(value, message) \
		do { \
			if(!(value)) { \
				fprintf(stderr, "[%s %s:%d]: OOM: %s\n", __func__, __FILE__, __LINE__, (message)); \
				exit(EXIT_FAILURE); \
			} \
		} while(false)

#else
	#include <assert.h>

	#define UNREACHABLE() \
		do { \
			assert(false && "UNREACHABLE"); /* NOLINT(cert-dcl03-c,misc-static-assert) */ \
		} while(false)

	#define OOM_ASSERT(value, message) \
		do { \
			assert((value) && (message)); /* NOLINT(cert-dcl03-c,misc-static-assert) */ \
		} while(false)

	#define ASSERT(x) assert(x)

#endif

#define UNUSED(v) ((void)(v))

// uses snprintf feature with passing NULL,0 as first two arguments to automatically determine the
// required buffer size, for more read man page
// for variadic functions its easier to use macro
// magic, attention, use this function in the right way, you have to prepare a char* that is set to
// null, then it works best! snprintf is safer then sprintf, since it guarantees some things, also
// it has a failure indicator
#define FORMAT_STRING_IMPL(toStore, statement, logger_fn, format, ...) \
	{ \
		char* internalBuffer = *toStore; \
		if(internalBuffer != NULL) { \
			free(internalBuffer); \
		} \
		const LibCInt toWrite = snprintf(NULL, 0, format, __VA_ARGS__) + 1; \
		internalBuffer = (char*)malloc(toWrite * sizeof(char)); \
		if(!internalBuffer) { \
			logger_fn("Couldn't allocate memory for %d bytes!\n", toWrite); \
			statement \
		} \
		const LibCInt written = snprintf(internalBuffer, toWrite, format, __VA_ARGS__); \
		if(written >= toWrite) { \
			logger_fn("snprintf did write more bytes then it had space in the buffer, available " \
			          "space: '%d', actually written: '%d'!\n", \
			          (toWrite) - 1, written); \
			free(internalBuffer); \
			statement \
		} \
		*toStore = internalBuffer; \
	} \
	if(*toStore == NULL) { \
		logger_fn("snprintf Macro gone wrong: '%s' is pointing to NULL!\n", #toStore); \
		statement \
	}

#define FORMAT_STRING(toStore, statement, format, ...) \
	FORMAT_STRING_IMPL(toStore, statement, IMPL_STDERR_LOGGER, format, __VA_ARGS__)

#define IMPL_STDERR_LOGGER(format, ...) fprintf(stderr, format, __VA_ARGS__)

#define ZERO_STRUCT(Type) (Type){ 0 }
#define ZERO_ARRAY() { 0 }

// NOTE: these are used for surpressing the clang-tidy error "totto-use-fixed-width-types-var"
// in places, where we interface with libc, it is intentional, that this check doesn't resolves
// aliases, as we intend to use this alias, we can use non-fixed types
typedef char LibCChar;
typedef int LibCInt;
typedef long LibCLong;

#ifdef __cplusplus
}
#endif
