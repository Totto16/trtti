
#include "./tjson_schema.h"

#include <tmap.h>
#include <tvec.h>

typedef struct {
	bool required;
} JsonSchemaOBjectEntryProperties;

typedef struct {
	JsonSchema schmea;
	JsonSchemaOBjectEntryProperties props;
} JsonObjectEntrySchema;

/* NOLINTBEGIN(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */
// GCOVR_EXCL_START (external library)
TMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(tstr, Tstr, JsonObjectEntrySchema, JsonObjectEntryMapImpl)
// GCOVR_EXCL_STOP
/* NOLINTEND(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */

TMAP_HASH_FUNC_SIG(tstr, Tstr) {
	return TMAP_HASH_STR(tstr_cstr(&key));
}

TMAP_EQ_FUNC_SIG(tstr, Tstr) {
	return tstr_eq(&key1, &key2);
}

typedef TMAP_TYPENAME_MAP(JsonObjectEntryMapImpl) JsonObjectEntryMap;

struct JsonSchemaObjectImpl {
	JsonObjectEntryMap map;
};

typedef struct {
	size_t min_length;
	size_t max_length;
} JsonSchemaArrayProperties;

/* NOLINTBEGIN(misc-use-internal-linkage,totto-function-passing-type,totto-use-fixed-width-types-var)
 */
// GCOVR_EXCL_START (external library)
TVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(JsonSchema)
// GCOVR_EXCL_STOP
/* NOLINTEND(misc-use-internal-linkage,totto-function-passing-type,totto-use-fixed-width-types-var)
 */

typedef TVEC_TYPENAME(JsonSchema) JsonSchemaArrOfValuesImpl;

struct JsonSchemaArrayImpl {
	JsonSchemaArrOfValuesImpl values;
	JsonSchemaArrayProperties props;
};

typedef struct {
	JsonSchemaArrayProperties parent;
} JsonSchemaStringProperties;

struct JsonSchemaStringImpl {
	JsonSchemaStringProperties props;
};

struct JsonSchemaOneOfImpl {
	JsonSchemaArrOfValuesImpl values;
};

typedef struct {
	size_t count;
} JsonSchemaState;

TJSON_NODISCARD static JsonObject* json_schema_to_string_impl(const JsonSchema* const schema,
                                                              const JsonSchemaOptions options,
                                                              JsonSchemaState* state) {
	return tstr_null();
}

TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* const schema) {
	return json_schema_to_string_advanced(
	    schema, (JsonSchemaOptions){ .cycles = JsonSchemaOptionTypeCycleRef,
	                                 .reused = JsonSchemaOptionTypeReusedRef });
}

TJSON_NODISCARD tstr json_schema_to_string_advanced(const JsonSchema* const schema,
                                                    const JsonSchemaOptions options) {

	const JsonObject* const json_items = json_schema_to_string_impl();

	/*                                                     {
	    "$schema": "https://json-schema.org/draft/2020-12/schema",
	    ...json_items,
	    "$defs": json_defs
	} */

	return tstr_null();
}
