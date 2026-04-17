
#include "./tjson_schema.h"

#include <tmap.h>
#include <tvec.h>

#include "./_impl/utils.h"

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
	JsonSchema schema;
} JsonDefEntry;

typedef struct {
	size_t value;
} JsonDefId;

/* NOLINTBEGIN(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */
// GCOVR_EXCL_START (external library)
TMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(JsonDefId, JsonDefIdName, JsonDefEntry, JsonDefEntryMapImpl)
// GCOVR_EXCL_STOP
/* NOLINTEND(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */

TMAP_HASH_FUNC_SIG(JsonDefId, JsonDefIdName) {
	return TMAP_HASH_SCALAR(key.value);
}

TMAP_EQ_FUNC_SIG(JsonDefId, JsonDefIdName) {
	return key1.value == key2.value;
}

typedef TMAP_TYPENAME_MAP(JsonDefEntryMapImpl) JsonDefEntryMap;

/* NOLINTBEGIN(misc-use-internal-linkage,totto-function-passing-type,totto-use-fixed-width-types-var)
 */
// GCOVR_EXCL_START (external library)
TVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE_EXTENDED(JsonSchema*, JsonSchemaPtr)
// GCOVR_EXCL_STOP
/* NOLINTEND(misc-use-internal-linkage,totto-function-passing-type,totto-use-fixed-width-types-var)
 */

typedef TVEC_TYPENAME(JsonSchemaPtr) JsonSchemaPtrArray;

typedef struct {
	size_t global_count;
	JsonDefEntryMap defs;
	JsonSchemaPtrArray current_nested_values;
} JsonSchemaState;

TJSON_NODISCARD static JsonObject* json_schema_to_string_impl(const JsonSchema* const schema,
                                                              const JsonSchemaOptions options,
                                                              JsonSchemaState* const state);

TJSON_NODISCARD static JsonObject*
json_schema_to_string_object_impl(const JsonSchemaObject* const object,
                                  const JsonSchemaOptions options, JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonObject*
json_schema_to_string_array_impl(const JsonSchemaArray* const object,
                                 const JsonSchemaOptions options, JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonObject* json_schema_to_string_number_impl(void) {
	return NULL;
}

TJSON_NODISCARD static JsonObject*
json_schema_to_string_string_impl(const JsonSchemaString* const string,
                                  const JsonSchemaOptions options, JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonObject* json_schema_to_string_boolean_impl() {
	return NULL;
}

TJSON_NODISCARD static JsonObject* json_schema_to_string_null_impl() {
	return NULL;
}

TJSON_NODISCARD static JsonObject*
json_schema_to_string_one_of_impl(const JsonSchemaOneOf* const one_of,
                                  const JsonSchemaOptions options, JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonObject*
json_schema_to_string_literal_impl(const JsonSchemaLiteral* const literal,
                                   const JsonSchemaOptions options, JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static bool json_schema_to_string_impl_check_cycle(JsonSchemaState* const state,
                                                                   const JsonSchemaOptions options,
                                                                   const JsonSchema* const schema) {

	for(size_t i = 0; i < TVEC_LENGTH(JsonSchemaPtr, (state->current_nested_values)); ++i) {
		const JsonSchema* const schema_ptr =
		    TVEC_AT(JsonSchemaPtr, (state->current_nested_values), i);

		if(schema == schema_ptr) {
			if(options.cycles == JsonSchemaOptionTypeCycleThrow) {
				return false;
			} else if(options.cycles == JsonSchemaOptionTypeCycleRef) {
				return REF;
			} else {
				return NULL;
			}
		}
	}

	return true;
}

TJSON_NODISCARD static JsonObject* json_schema_to_string_impl(const JsonSchema* const schema,
                                                              const JsonSchemaOptions options,
                                                              JsonSchemaState* const state) {

	if(!json_schema_to_string_impl_check_cycle(state, options, schema)) {
		return NULL;
	}

	JsonObject* result = NULL;

	SWITCH_JSON_SCHEMA(*schema) {
		CASE_JSON_SCHEMA_IS_OBJECT_CONST(*schema) {
			result = json_schema_to_string_object_impl(object.obj, options, state);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ARRAY_CONST(*schema) {
			result = json_schema_to_string_array_impl(array.arr, options, state);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NUMBER() {
			result = json_schema_to_string_number_impl();
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_STRING_CONST(*schema) {
			result = json_schema_to_string_string_impl(string.str, options, state);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_BOOLEAN() {
			result = json_schema_to_string_boolean_impl();
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NULL() {
			result = json_schema_to_string_null_impl();
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ONE_OF_CONST(*schema) {
			result = json_schema_to_string_one_of_impl(one_of.one_of, options, state);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_LITERAL_CONST(*schema) {
			result = json_schema_to_string_literal_impl(literal.lit, options, state);
		}
		break;
		VARIANT_CASE_END();
		default: {
			result = NULL;
			break;
		}
	}

	if(result == NULL) {
		return NULL;
	}

	if(type == Ref) {
		add_def(result);
		return empty_objx()
	}
}

TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* const schema) {
	return json_schema_to_string_advanced(
	    schema, (JsonSchemaOptions){ .cycles = JsonSchemaOptionTypeCycleRef,
	                                 .reused = JsonSchemaOptionTypeReusedRef });
}

static void free_json_schema_state(JsonSchemaState* state) {
	// TODO
}

TJSON_NODISCARD tstr json_schema_to_string_advanced(const JsonSchema* const schema,
                                                    const JsonSchemaOptions options) {

	JsonSchemaState state = {
		.global_count = 0,
		.defs = TMAP_EMPTY(JsonDefEntryMapImpl),
		.current_nested_values = TVEC_EMPTY(JsonSchemaPtr),
	};

	const JsonObject* const json_items = json_schema_to_string_impl(schema, options, &state);

	if(json_items == NULL) {
		free_json_schema_state(&state);
		return tstr_null();
	}

	JsonObject* const root = get_empty_json_object();

	{
		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "$schema",
		                               new_json_value_string(json_get_string_from_cstr(
		                                   "https://json-schema.org/draft/2020-12/schema")));

		ASSERT(tstr_static_is_null(insert_result));

		/*                                                     {
		"$schema": "https://json-schema.org/draft/2020-12/schema",
		...json_items,
		"$defs": json_defs
	} */
	}

	JsonValue final_value = new_json_value_object(root);
	const tstr result = json_value_to_string(&final_value);

	if(tstr_is_null(&result)) {
		free_json_schema_state(&state);
		free_json_value(&final_value);
		return tstr_null();
	}

	free_json_schema_state(&state);
	free_json_value(&final_value);

	return result;
}
