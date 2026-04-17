#include "./tjson_schema.h"

#include <tmap.h>
#include <tvec.h>

#include "./_impl/utils.h"

#include <tstr_builder.h>

typedef struct {
	bool required;
} JsonSchemaOBjectEntryProperties;

typedef struct {
	JsonSchema schema;
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

/**
 * @enum MASK / FLAGS
 */
typedef enum ENUM_IS_MASK C_23_NARROW_ENUM_TO(uint16_t){
	ArrayPropertiesFlagsNone = 0,
	//
	ArrayPropertiesFlagsMin = 0x1,
	ArrayPropertiesFlagsMax = 0x2,
} ArrayPropertiesFlags;

typedef struct {
	ArrayPropertiesFlags flags;
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
	JsonSchema item;
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
	JsonObject* object;
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

typedef struct {
	size_t global_count;
	JsonDefEntryMap defs;
} JsonSchemaState;

// manual "variant", but only used internally, so it's fine
typedef struct {
	bool is_error;
	union {
		JsonDefId ok;
		tstr_static error;
	} data;
} JsonSchemaAddResult;

NODISCARD static inline JsonSchemaAddResult
new_json_schema_add_result_error(tstr_static const error) {
	return (JsonSchemaAddResult){ .is_error = true, .data = { .error = error } };
}

NODISCARD MAYBE_UNUSED static inline JsonSchemaAddResult
new_json_schema_add_result_ok(JsonDefId const ok) {
	return (JsonSchemaAddResult){ .is_error = false, .data = { .ok = ok } };
}

static JsonSchemaAddResult json_schema_to_string_make_def_impl(JsonObject* const object,
                                                               JsonSchemaState* const state) {

	JsonDefId id = { .value = state->global_count };

	state->global_count++;

	JsonDefEntry entry = { .object = object };

	TmapInsertResult result = TMAP_INSERT(JsonDefEntryMapImpl, &(state->defs), id, entry, false);

	OOM_ASSERT(result == TmapInsertResultOk, "insert failed");
	return new_json_schema_add_result_ok(id);
}

#define SCHEMA_NAME_TEMPLATE "__schema%zu"

TJSON_NODISCARD static tstr json_schema_impl_get_def_schema_name(const JsonDefId id) {

	StringBuilder* string_builder = string_builder_init();

	ASSERT(string_builder != NULL);

	STRING_BUILDER_APPENDF(string_builder, return tstr_null();
	                       , "#/$defs/" SCHEMA_NAME_TEMPLATE, id.value);

	return string_builder_release_into_tstr(&string_builder);
}

TJSON_NODISCARD static tstr json_schema_impl_get_schema_name(const JsonDefId id) {

	StringBuilder* string_builder = string_builder_init();

	ASSERT(string_builder != NULL);

	STRING_BUILDER_APPENDF(string_builder, return tstr_null();, SCHEMA_NAME_TEMPLATE, id.value);

	return string_builder_release_into_tstr(&string_builder);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_impl(const JsonSchema* const schema,

                           JsonSchemaState* const state);

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_object_impl(const JsonSchemaObject* const object,
                                  JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_array_impl(const JsonSchemaArray* const array, JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/array

	JsonObject* const root = get_empty_json_object();

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string(json_get_string_from_cstr("array")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	ASSERT(root != NULL);

	{ // add items field

		const JsonSchemaAddResult add_result = json_schema_to_string_impl(&(array->item), state);

		if(add_result.is_error) {
			return add_result;
		}

		const JsonDefId result_id = add_result.data.ok;

		JsonObject* const entry_obj = get_empty_json_object();

		{

			const tstr schema_def_name = json_schema_impl_get_def_schema_name(result_id);

			tstr_static insert_result = json_object_add_entry_cstr(
			    entry_obj, "$ref",
			    new_json_value_string(json_get_string_from_tstr(&schema_def_name)));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}

		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "items", new_json_value_object(entry_obj));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	{ // handle props

		// TODO:handle props
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "PROPS", new_json_value_string(json_get_string_from_cstr("TODO")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_number_impl(JsonSchemaState* const state) {

	// see: https://json-schema.org/understanding-json-schema/reference/numeric

	JsonObject* const root = get_empty_json_object();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string(json_get_string_from_cstr("number")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_string_impl(const JsonSchemaString* const string,
                                  JsonSchemaState* const state) {
	return NULL;
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_boolean_impl(JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/boolean

	JsonObject* const root = get_empty_json_object();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string(json_get_string_from_cstr("boolean")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_null_impl(JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/null

	JsonObject* const root = get_empty_json_object();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string(json_get_string_from_cstr("null")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_one_of_impl(const JsonSchemaOneOf* const one_of,
                                  JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/combining#oneOf

	JsonObject* const root = get_empty_json_object();

	ASSERT(root != NULL);

	JsonArray* one_of_arr = get_empty_json_array();

	ASSERT(one_of_arr != NULL);

	{

		for(size_t i = 0; i < TVEC_LENGTH(JsonSchema, one_of->values); ++i) {
			const JsonSchema value = TVEC_AT(JsonSchema, (one_of->values), i);

			const JsonSchemaAddResult add_result = json_schema_to_string_impl(&value, state);

			if(add_result.is_error) {
				return add_result;
			}

			const JsonDefId result_id = add_result.data.ok;

			JsonObject* const entry_obj = get_empty_json_object();

			{

				const tstr schema_def_name = json_schema_impl_get_def_schema_name(result_id);

				tstr_static insert_result = json_object_add_entry_cstr(
				    entry_obj, "$ref",
				    new_json_value_string(json_get_string_from_tstr(&schema_def_name)));

				if(!tstr_static_is_null(insert_result)) {
					return new_json_schema_add_result_error(insert_result);
				}
			}

			tstr_static insert_result =
			    json_array_add_entry(one_of_arr, new_json_value_object(entry_obj));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	{
		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "oneOf", new_json_value_array(one_of_arr));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_literal_impl(const JsonSchemaLiteral* const literal,
                                   JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/const
	// and: https://json-schema.org/understanding-json-schema/reference/string

	JsonObject* const root = get_empty_json_object();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string(json_get_string_from_cstr("string")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		insert_result = json_object_add_entry_cstr(
		    root, "const", new_json_value_string(json_get_string_from_tstr(&(literal->value))));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_impl(const JsonSchema* const schema,

                           JsonSchemaState* const state) {

	SWITCH_JSON_SCHEMA(*schema) {
		CASE_JSON_SCHEMA_IS_OBJECT_CONST(*schema) {
			return json_schema_to_string_object_impl(object.obj, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ARRAY_CONST(*schema) {
			return json_schema_to_string_array_impl(array.arr, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NUMBER() {
			return json_schema_to_string_number_impl(state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_STRING_CONST(*schema) {
			return json_schema_to_string_string_impl(string.str, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_BOOLEAN() {
			return json_schema_to_string_boolean_impl(state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NULL() {
			return json_schema_to_string_null_impl(state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ONE_OF_CONST(*schema) {
			return json_schema_to_string_one_of_impl(one_of.one_of, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_LITERAL_CONST(*schema) {
			return json_schema_to_string_literal_impl(literal.lit, state);
		}
		VARIANT_CASE_END();
		default: {
			return new_json_schema_add_result_error(TSTR_STATIC_LIT("unknown schema type"));
		}
	}
}

static void free_json_schema_state(JsonSchemaState* const state) {
	TMAP_TYPENAME_ITER(JsonDefEntryMapImpl)
	iter = TMAP_ITER_INIT(JsonDefEntryMapImpl, &(state->defs));

	TMAP_TYPENAME_ENTRY(JsonDefEntryMapImpl) value;

	while(TMAP_ITER_NEXT(JsonDefEntryMapImpl, &iter, &value)) {

		free_json_object(value.value.object);
	}
}

// TODO(Totto): use better return type (a variant xD) !
TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* const schema) {

	JsonSchemaState state = {
		.global_count = 0,
		.defs = TMAP_EMPTY(JsonDefEntryMapImpl),
	};

	const JsonSchemaAddResult root_res = json_schema_to_string_impl(schema, &state);

	if(root_res.is_error) {
		// TODO(Totto): maybe use the error?
		free_json_schema_state(&state);
		return tstr_null();
	}

	const JsonDefId root_id = root_res.data.ok;

	JsonObject* const root = get_empty_json_object();

	{
		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "$schema",
		                               new_json_value_string(json_get_string_from_cstr(
		                                   "https://json-schema.org/draft/2020-12/schema")));

		ASSERT(tstr_static_is_null(insert_result));

		const JsonDefEntry* const root_properties =
		    TMAP_GET(JsonDefEntryMapImpl, &(state.defs), root_id);

		ASSERT(root_properties != NULL);

		{ // add the root entry

			JsonObjectIter* iter = json_object_get_iterator(root_properties->object);

			JsonString* invalid_start_char = json_get_string_from_cstr("$");

			ASSERT(invalid_start_char != NULL);

			while(true) {

				const JsonObjectEntry* entry = json_object_iterator_next(iter);

				if(entry == NULL) {
					break;
				}

				const JsonString* const key = json_object_entry_get_key(entry);

				ASSERT(key != NULL);

				if(json_string_starts_with(key, invalid_start_char)) {
					// TODO(Totto): free everything and log the reason?
					return tstr_null();
				}

				const JsonValue* const value = json_object_entry_get_value(entry);

				ASSERT(value != NULL);

				insert_result = json_object_add_entry_dup(root, key, *value);

				ASSERT(tstr_static_is_null(insert_result));
			}

			json_object_free_iterator(iter);
			free_json_string(invalid_start_char);
		}

		// remove that entry

		// TODO(Totto): check if this was successfull, as this function doesn't return anything yet!
		TMAP_REM(JsonDefEntryMapImpl, &(state.defs), root_id);

		{ // add the defiunions

			JsonObject* const defs = get_empty_json_object();

			TMAP_TYPENAME_ITER(JsonDefEntryMapImpl)
			iter = TMAP_ITER_INIT(JsonDefEntryMapImpl, &(state.defs));

			TMAP_TYPENAME_ENTRY(JsonDefEntryMapImpl) value;

			while(TMAP_ITER_NEXT(JsonDefEntryMapImpl, &iter, &value)) {

				const JsonDefId def_id = value.key;

				const tstr schema_name = json_schema_impl_get_schema_name(def_id);

				insert_result = json_object_add_entry_tstr(
				    defs, &schema_name, new_json_value_object(value.value.object));
			}

			insert_result = json_object_add_entry_cstr(root, "defs", new_json_value_object(defs));

			ASSERT(tstr_static_is_null(insert_result));
		}
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
