
#pragma once

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
	#define assert(x) /* NOLINT(readability-identifier-naming) */ \
		do { \
			UNUSED((x)); \
		} while(false)

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

#endif

// NOTE: these are used for surpressing the clang-tidy error "totto-use-fixed-width-types-var"
// in places, where we interface with libc, it is intentional, that this check doesn't resolves
// aliases, as we intend to use this alias, we can use non-fixed types
typedef char LibCChar;
typedef int LibCInt;
typedef long LibCLong;

#ifdef __cplusplus
}
#endif
