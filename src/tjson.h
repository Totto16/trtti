#pragma once

#include <stdbool.h>
#include <tstr.h>

// public part of the utils

#if _TJSON_COMPILE_WITH_NARROWED_ENUMS
	#define TJSON_C_23_NARROW_ENUM_TO(x) : x
	#define TJSON_C_23_ENUM_TYPE(x) x

	#define TJSON_VARIANT_IMPL_COMPILED_WITH_NARROWED_ENUMS 1
#else
	#define TJSON_C_23_NARROW_ENUM_TO(x)
	#define TJSON_C_23_ENUM_TYPE(x) int

	#define TJSON_VARIANT_IMPL_COMPILED_WITH_NARROWED_ENUMS 0
#endif

#define VARIANT_IMPL_JSON_VARIANTS_COMPILED_WITH_NARROWED_ENUMS \
	TJSON_VARIANT_IMPL_COMPILED_WITH_NARROWED_ENUMS

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000) || __cplusplus
	#define TJSON_NODISCARD [[nodiscard]]
	#define TJSON_MAYBE_UNUSED [[maybe_unused]]
#else
    // see e.g. https://www.gnu.org/software/gnulib/manual/html_node/Attributes.html
	#define TJSON_NODISCARD __attribute__((__warn_unused_result__))
	#define TJSON_MAYBE_UNUSED __attribute__((__unused__))
#endif

#define TJSON_UNUSED(v) ((void)(v))

// cool trick from here:
// https://stackoverflow.com/questions/777261/avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
#ifdef NDEBUG
	#define TJSON_UNREACHABLE() \
		do { \
			fprintf(stderr, "[%s %s:%d]: UNREACHABLE\n", __func__, __FILE__, __LINE__); \
			exit(EXIT_FAILURE); \
		} while(false)

	#define TJSON_OOM_ASSERT(value, message) \
		do { \
			if(!(value)) { \
				fprintf(stderr, "[%s %s:%d]: OOM: %s\n", __func__, __FILE__, __LINE__, (message)); \
				exit(EXIT_FAILURE); \
			} \
		} while(false)

#else
	#include <assert.h>

	#define TJSON_UNREACHABLE() \
		do { \
			assert(false && "UNREACHABLE"); /* NOLINT(cert-dcl03-c,misc-static-assert) */ \
		} while(false)

	#define TJSON_OOM_ASSERT(value, message) \
		do { \
			assert((value) && (message)); /* NOLINT(cert-dcl03-c,misc-static-assert) */ \
		} while(false)

#endif

// end of utils

#include "./tjson/variants.h"

#ifdef __cplusplus
extern "C" {
#endif

// see: https://datatracker.ietf.org/doc/html/rfc8259

typedef struct JsonObjectImpl JsonObject;

typedef struct JsonArrayImpl JsonArray;

typedef struct JsonStringImpl JsonString;

typedef struct {
	double value;
} JsonNumber;

typedef struct {
	bool value;
} JsonBoolean;

typedef struct {
	size_t line;
	size_t col;
} JsonSourcePosition;

typedef struct {
	// NOTE: this "references" the original file path data, it can be freed, as this is passed by
	// the user, the user can only free the file tstr, that he passed in, if he doesn't use this
	// anymore
	const tstr* file_path;
} JsonFileSource;

typedef struct {
	// NOTE: this "references" the original data, it can be freed, as this is passed by the user,
	// the user can only free the underlying data of the tstr_view, that he passed in, if he doesn't
	// use this anymore
	tstr_view data;
} JsonStringSource;

GENERATE_VARIANT_ALL_JSON_SOURCE()

typedef struct {
	JsonSource source;
	JsonSourcePosition pos;
} JsonSourceLocation;

typedef struct {
	tstr_static message;
	// note: this has references to the original passed in data, either a tstr file_path or  the
	// tstr_view of the passed ins tring, so it is only valid, until the user freed that data
	JsonSourceLocation loc;
} JsonError;

// GCOVR_EXCL_START (external library)
GENERATE_VARIANT_ALL_JSON_VALUE()

GENERATE_VARIANT_ALL_JSON_PARSE_RESULT()
// GCOVR_EXCL_STOP

// parse json strings

TJSON_NODISCARD JsonParseResult json_value_parse_from_str(tstr_view data);

TJSON_NODISCARD JsonParseResult json_value_parse_from_file(const tstr* file_path);

void free_json_value(JsonValue* json_value);

// serialize json values

typedef struct {
	size_t indent_size;
} JsonSerializeOptions;

TJSON_NODISCARD tstr json_value_to_string(const JsonValue* json_value);

TJSON_NODISCARD tstr json_value_to_string_advanced(const JsonValue* json_value,
                                                   JsonSerializeOptions options);

// utility / get functions

TJSON_NODISCARD bool json_string_eq(const JsonString* str1, const JsonString* str2);

TJSON_NODISCARD bool json_string_starts_with(const JsonString* str, const JsonString* prefix);

TJSON_NODISCARD size_t json_array_size(const JsonArray* array);

TJSON_NODISCARD const JsonValue* json_array_at(const JsonArray* array, size_t index);

TJSON_NODISCARD size_t json_object_count(const JsonObject* object);

typedef struct JsonObjectEntryImpl JsonObjectEntry;

TJSON_NODISCARD const JsonObjectEntry* json_object_get_entry_by_key(const JsonObject* object,
                                                                    const JsonString* key);

typedef struct JsonObjectIterImpl JsonObjectIter;

TJSON_NODISCARD JsonObjectIter* json_object_get_iterator(const JsonObject* object);

TJSON_NODISCARD const JsonObjectEntry* json_object_iterator_next(JsonObjectIter* iter);

void json_object_free_iterator(JsonObjectIter* iter);

TJSON_NODISCARD const JsonString* json_object_entry_get_key(const JsonObjectEntry* object_entry);

TJSON_NODISCARD const JsonValue* json_object_entry_get_value(const JsonObjectEntry* object_entry);

// create functions

TJSON_NODISCARD JsonString* json_get_string_from_cstr(const char* cstr);

TJSON_NODISCARD JsonString* json_get_string_from_tstr(const tstr* str);

TJSON_NODISCARD JsonString* json_get_string_from_tstr_view(tstr_view str_view);

void free_json_string(JsonString* json_string);

TJSON_NODISCARD JsonArray* json_array_get_empty(void);

TJSON_NODISCARD tstr_static json_array_add_entry(JsonArray* json_array, JsonValue entry);

void free_json_array(JsonArray* json_arr);

TJSON_NODISCARD JsonObject* json_object_get_empty(void);

TJSON_NODISCARD tstr_static json_object_add_entry(JsonObject* json_object, JsonString** key_moved,
                                                  JsonValue value);

TJSON_NODISCARD tstr_static json_object_add_entry_dup(JsonObject* json_object,
                                                      const JsonString* key, JsonValue value);

TJSON_NODISCARD tstr_static json_object_add_entry_tstr(JsonObject* json_object, const tstr* key,
                                                       JsonValue value);

TJSON_NODISCARD tstr_static json_object_add_entry_cstr(JsonObject* json_object, const char* key,
                                                       JsonValue value);

void free_json_object(JsonObject* json_obj);

// utility functions

TJSON_NODISCARD JsonSourceLocation json_source_location_get_null(void);

TJSON_NODISCARD bool json_source_location_is_null(JsonSourceLocation location);

TJSON_NODISCARD tstr json_format_source_location(JsonSourceLocation location);

TJSON_NODISCARD tstr json_format_error(JsonError error);

#ifdef __cplusplus
}
#endif
