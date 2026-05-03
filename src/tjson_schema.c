#include "./tjson_schema.h"

#include <tmap.h>
#include <tvec.h>

#include "./_impl/simple_regex.h"
#include "./_impl/utils.h"

#include <trc.h>
#include <tstr_builder.h>

typedef struct {
	JsonSchema schema;
	bool required;
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
	bool allow_additional_properties;
};

/**
 * @enum MASK / FLAGS
 */
typedef enum C_23_NARROW_ENUM_TO(uint8_t) {
	JsonSchemaArrayPropertiesFlagsNone = 0,
	//
	JsonSchemaArrayPropertiesFlagsMin = 0x1,
	JsonSchemaArrayPropertiesFlagsMax = 0x2,
} JsonSchemaArrayPropertiesFlags;

typedef struct {
	JsonSchemaArrayPropertiesFlags flags;
	size_t min_items;
	size_t max_items;
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
	JsonSchema items;
	JsonSchemaArrayProperties props;
	bool require_unique_items;
};

/**
 * @enum MASK / FLAGS
 */
typedef enum C_23_NARROW_ENUM_TO(uint8_t) {
	JsonSchemaStringPropertiesFlagsNone = 0,
	//
	JsonSchemaStringPropertiesFlagsMin = 0x1,
	JsonSchemaStringPropertiesFlagsMax = 0x2,
	JsonSchemaStringPropertiesFlagsPattern = 0x4,
} JsonSchemaStringPropertiesFlags;

typedef struct {
	JsonSchemaStringPropertiesFlags flags;
	size_t min_length;
	size_t max_length;
	JsonSchemaRegex* pattern;
} JsonSchemaStringProperties;

struct JsonSchemaStringImpl {
	JsonSchemaStringProperties props;
};

struct JsonSchemaLiteralImpl {
	tstr value;
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

struct JsonSchemaRegexImpl {
	SimpleRegex regex;
	tstr original;
};

RC_DEFINE_TYPE(JsonSchemaRegex)
RC_DEFINE_TYPE(JsonSchemaObject)
RC_DEFINE_TYPE(JsonSchemaArray)
RC_DEFINE_TYPE(JsonSchemaString)
RC_DEFINE_TYPE(JsonSchemaLiteral)
RC_DEFINE_TYPE(JsonSchemaOneOf)

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
	// see: https://json-schema.org/understanding-json-schema/reference/object

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{ // basic properties
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("object")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		if(!(object->allow_additional_properties)) {

			insert_result =
			    json_object_add_entry_cstr(root, "additionalProperties",
			                               new_json_value_boolean((JsonBoolean){ .value = false }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	// complicated properties

	JsonObject* const properties_obj = json_object_get_empty();
	ASSERT(properties_obj != NULL);

	JsonArray* const required_arr = json_array_get_empty();
	ASSERT(required_arr != NULL);

	{

		TMAP_TYPENAME_ITER(JsonObjectEntryMapImpl)
		iter = TMAP_ITER_INIT(JsonObjectEntryMapImpl, &(object->map));

		TMAP_TYPENAME_ENTRY(JsonObjectEntryMapImpl) value;

		while(TMAP_ITER_NEXT(JsonObjectEntryMapImpl, &iter, &value)) {

			const tstr key = value.key;

			const JsonObjectEntrySchema obj_value = value.value;

			const JsonSchemaAddResult add_result =
			    json_schema_to_string_impl(&obj_value.schema, state);

			if(add_result.is_error) {
				return add_result;
			}

			const JsonDefId result_id = add_result.data.ok;

			JsonObject* const entry_obj = json_object_get_empty();

			{

				const tstr schema_def_name = json_schema_impl_get_def_schema_name(result_id);

				tstr_static insert_result = json_object_add_entry_cstr(
				    entry_obj, "$ref",
				    new_json_value_string_rc(json_get_string_from_tstr(&schema_def_name)));

				if(!tstr_static_is_null(insert_result)) {
					return new_json_schema_add_result_error(insert_result);
				}
			}

			tstr_static insert_result = json_object_add_entry_tstr(
			    properties_obj, &key, new_json_value_object_rc(entry_obj));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}

			if(obj_value.required) {
				insert_result = json_array_add_entry(
				    required_arr, new_json_value_string_rc(json_get_string_from_tstr(&key)));

				if(!tstr_static_is_null(insert_result)) {
					return new_json_schema_add_result_error(insert_result);
				}
			}
		}
	}

	{ // complicated properties

		if(json_array_get_size(required_arr) > 0) {

			tstr_static insert_result =
			    json_object_add_entry_cstr(root, "required", new_json_value_array_rc(required_arr));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		} else {
			free_json_array(required_arr);
		}

		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "properties", new_json_value_object_rc(properties_obj));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

#define HAS_FLAG(value, flag) (((value) & (flag)) == (flag))

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_array_impl(const JsonSchemaArray* const array, JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/array

	JsonObject* const root = json_object_get_empty();

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("array")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		if((array->require_unique_items)) {

			insert_result = json_object_add_entry_cstr(
			    root, "uniqueItems", new_json_value_boolean((JsonBoolean){ .value = true }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	ASSERT(root != NULL);

	{ // add items field

		const JsonSchemaAddResult add_result = json_schema_to_string_impl(&(array->items), state);

		if(add_result.is_error) {
			return add_result;
		}

		const JsonDefId result_id = add_result.data.ok;

		JsonObject* const entry_obj = json_object_get_empty();

		{

			const tstr schema_def_name = json_schema_impl_get_def_schema_name(result_id);

			tstr_static insert_result = json_object_add_entry_cstr(
			    entry_obj, "$ref",
			    new_json_value_string_rc(json_get_string_from_tstr(&schema_def_name)));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}

		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "items", new_json_value_object_rc(entry_obj));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	{ // handle props

		const JsonSchemaArrayProperties array_props = array->props;

		if(HAS_FLAG(array_props.flags, JsonSchemaArrayPropertiesFlagsMin)) {

			tstr_static insert_result = json_object_add_entry_cstr(
			    root, "minItems",
			    new_json_value_number((JsonNumber){ .value = (double)array_props.min_items }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}

		if(HAS_FLAG(array_props.flags, JsonSchemaArrayPropertiesFlagsMax)) {

			tstr_static insert_result = json_object_add_entry_cstr(
			    root, "maxItems",
			    new_json_value_number((JsonNumber){ .value = (double)array_props.max_items }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_number_impl(JsonSchemaState* const state) {

	// see: https://json-schema.org/understanding-json-schema/reference/numeric

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("number")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_string_impl(const JsonSchemaString* const string,
                                  JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/const
	// and: https://json-schema.org/understanding-json-schema/reference/string

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("string")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	{ // handle props

		const JsonSchemaStringProperties string_props = string->props;

		if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsMin)) {

			tstr_static insert_result = json_object_add_entry_cstr(
			    root, "minLength",
			    new_json_value_number((JsonNumber){ .value = (double)string_props.min_length }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}

		if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsMax)) {

			tstr_static insert_result = json_object_add_entry_cstr(
			    root, "maxLength",
			    new_json_value_number((JsonNumber){ .value = (double)string_props.max_length }));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}

		if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsPattern)) {

			assert(string_props.pattern != NULL);

			tstr_static insert_result =
			    json_object_add_entry_cstr(root, "pattern",
			                               new_json_value_string_rc(json_get_string_from_tstr(
			                                   &string_props.pattern->original)));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_boolean_impl(JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/boolean

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("boolean")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_null_impl(JsonSchemaState* const state) {
	// see: https://json-schema.org/understanding-json-schema/reference/null

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("null")));

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

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	JsonArray* one_of_arr = json_array_get_empty();

	ASSERT(one_of_arr != NULL);

	{

		for(size_t i = 0; i < TVEC_LENGTH(JsonSchema, one_of->values); ++i) {
			const JsonSchema value = TVEC_AT(JsonSchema, (one_of->values), i);

			const JsonSchemaAddResult add_result = json_schema_to_string_impl(&value, state);

			if(add_result.is_error) {
				return add_result;
			}

			const JsonDefId result_id = add_result.data.ok;

			JsonObject* const entry_obj = json_object_get_empty();

			{

				const tstr schema_def_name = json_schema_impl_get_def_schema_name(result_id);

				tstr_static insert_result = json_object_add_entry_cstr(
				    entry_obj, "$ref",
				    new_json_value_string_rc(json_get_string_from_tstr(&schema_def_name)));

				if(!tstr_static_is_null(insert_result)) {
					return new_json_schema_add_result_error(insert_result);
				}
			}

			tstr_static insert_result =
			    json_array_add_entry(one_of_arr, new_json_value_object_rc(entry_obj));

			if(!tstr_static_is_null(insert_result)) {
				return new_json_schema_add_result_error(insert_result);
			}
		}
	}

	{
		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "oneOf", new_json_value_array_rc(one_of_arr));

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

	JsonObject* const root = json_object_get_empty();

	ASSERT(root != NULL);

	{
		tstr_static insert_result = json_object_add_entry_cstr(
		    root, "type", new_json_value_string_rc(json_get_string_from_cstr("string")));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}

		insert_result = json_object_add_entry_cstr(
		    root, "const", new_json_value_string_rc(json_get_string_from_tstr(&(literal->value))));

		if(!tstr_static_is_null(insert_result)) {
			return new_json_schema_add_result_error(insert_result);
		}
	}

	return json_schema_to_string_make_def_impl(root, state);
}

TJSON_NODISCARD static JsonSchemaAddResult
json_schema_to_string_impl(const JsonSchema* const json_schema,

                           JsonSchemaState* const state) {

	SWITCH_JSON_SCHEMA(*json_schema) {
		CASE_JSON_SCHEMA_IS_OBJECT_CONST(*json_schema) {
			return json_schema_to_string_object_impl(object.obj, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ARRAY_CONST(*json_schema) {
			return json_schema_to_string_array_impl(array.arr, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NUMBER() {
			return json_schema_to_string_number_impl(state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_STRING_CONST(*json_schema) {
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
		CASE_JSON_SCHEMA_IS_ONE_OF_CONST(*json_schema) {
			return json_schema_to_string_one_of_impl(one_of.one_of, state);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_LITERAL_CONST(*json_schema) {
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

	TMAP_FREE(JsonDefEntryMapImpl, &(state->defs));
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

	JsonObject* const root = json_object_get_empty();

	{
		tstr_static insert_result =
		    json_object_add_entry_cstr(root, "$schema",
		                               new_json_value_string_rc(json_get_string_from_cstr(
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

				insert_result = json_object_add_entry_by_other_entry(root, entry);

				ASSERT(tstr_static_is_null(insert_result));
			}

			json_object_free_iterator(iter);
			free_json_string(invalid_start_char);
		}

		// TODO: check if this works as expected
		free_json_object(root_properties->object);

		// remove that entry

		// TODO(Totto): check if this was successfull, as this function doesn't return anything yet!
		TMAP_REM(JsonDefEntryMapImpl, &(state.defs), root_id);

		{ // add the defiunions

			JsonObject* const defs = json_object_get_empty();

			TMAP_TYPENAME_ITER(JsonDefEntryMapImpl)
			iter = TMAP_ITER_INIT(JsonDefEntryMapImpl, &(state.defs));

			TMAP_TYPENAME_ENTRY(JsonDefEntryMapImpl) value;

			while(TMAP_ITER_NEXT(JsonDefEntryMapImpl, &iter, &value)) {

				const JsonDefId def_id = value.key;

				const tstr schema_name = json_schema_impl_get_schema_name(def_id);

				insert_result = json_object_add_entry_tstr(
				    defs, &schema_name, new_json_value_object_rc(value.value.object));
			}

			insert_result =
			    json_object_add_entry_cstr(root, "$defs", new_json_value_object_rc(defs));

			ASSERT(tstr_static_is_null(insert_result));

			// as we "moved" all state->defs json_values into the new final value, we can clear it,
			// so that it doesn't make problems
			TMAP_CLEAR(JsonDefEntryMapImpl, &(state.defs));
		}
	}

	JsonValue final_value = new_json_value_object_rc(root);
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

static void free_json_object_entry_schema(JsonObjectEntrySchema* const entry) {
	free_json_schema(&(entry->schema));
}

static void json_schema_object_destroy_impl(JsonSchemaObject* const json_schema_object) {
	TMAP_TYPENAME_ITER(JsonObjectEntryMapImpl)
	iter = TMAP_ITER_INIT(JsonObjectEntryMapImpl, &(json_schema_object->map));

	TMAP_TYPENAME_ENTRY(JsonObjectEntryMapImpl) value;

	while(TMAP_ITER_NEXT(JsonObjectEntryMapImpl, &iter, &value)) {

		free_json_object_entry_schema(&value.value);
		tstr_free(&value.key);
	}

	TMAP_FREE(JsonObjectEntryMapImpl, &(json_schema_object->map));
}

TJSON_NODISCARD JsonSchemaObject* json_schema_object_get(const bool allow_additional_properties) {
	JsonSchemaObject* const object = RC_ALLOC(JsonSchemaObject, json_schema_object_destroy_impl);

	if(object == NULL) {
		return NULL;
	}

	object->map = TMAP_EMPTY(JsonObjectEntryMapImpl);
	object->allow_additional_properties = allow_additional_properties;

	return object;
}

NODISCARD static tstr_static
json_schema_object_add_entry_impl(JsonSchemaObject* const json_schema_object, tstr key,
                                  const JsonObjectEntrySchema entry) {

	const TmapInsertResult result =
	    TMAP_INSERT(JsonObjectEntryMapImpl, &(json_schema_object->map), key, entry, false);

	switch(result) {
		case TmapInsertResultOk: {
			return tstr_static_null();
		}
		case TmapInsertResultErr: {
			return TSTR_STATIC_LIT("json schema object add error");
		}
		case TmapInsertResultWouldOverwrite: {
			return TSTR_STATIC_LIT("json schema object has duplicate key");
		}
		default: {
			return TSTR_STATIC_LIT("json schema object add unknown error");
		}
	}
}

TJSON_NODISCARD tstr_static json_schema_object_add_entry(JsonSchemaObject* const json_schema_object,
                                                         tstr* const moved_key,
                                                         const JsonSchema value,
                                                         const bool required) {
	const tstr key = *moved_key;

	const JsonObjectEntrySchema entry = { .schema = value, .required = required };

	return json_schema_object_add_entry_impl(json_schema_object, key, entry);
}

TJSON_NODISCARD tstr_static
json_schema_object_add_entry_dup(JsonSchemaObject* const json_schema_object, const tstr* const key,
                                 const JsonSchema value, const bool required) {
	const tstr key_dup = tstr_dup(key);

	const JsonObjectEntrySchema entry = { .schema = value, .required = required };

	return json_schema_object_add_entry_impl(json_schema_object, key_dup, entry);
}

void free_json_schema_object(JsonSchemaObject* const json_schema_object) {
	RC_RELEASE(JsonSchemaObject, json_schema_object);
}

static JsonSchemaArrayProperties empty_array_props_impl(void) {

	const JsonSchemaArrayProperties props = {
		.flags = JsonSchemaArrayPropertiesFlagsNone,
		.max_items = 0,
		.min_items = 0,
	};

	return props;
}

static void json_schema_array_destroy_impl(JsonSchemaArray* const json_schema_arr) {
	free_json_schema(&(json_schema_arr->items));
}

TJSON_NODISCARD JsonSchemaArray* json_schema_array_get(const JsonSchema items,
                                                       const bool require_unique_items) {
	JsonSchemaArray* const array = RC_ALLOC(JsonSchemaArray, json_schema_array_destroy_impl);

	if(array == NULL) {
		return NULL;
	}

	array->items = items;
	array->require_unique_items = require_unique_items;
	array->props = empty_array_props_impl();

	return array;
}

TJSON_NODISCARD tstr_static json_schema_array_set_min(JsonSchemaArray* const json_schema_arr,
                                                      const size_t min) {

	if(HAS_FLAG(json_schema_arr->props.flags, JsonSchemaArrayPropertiesFlagsMin)) {
		return TSTR_STATIC_LIT("array prop error: min already set!");
	}

	json_schema_arr->props.min_items = min;
	json_schema_arr->props.flags |= JsonSchemaArrayPropertiesFlagsMin;
	return tstr_static_null();
}

TJSON_NODISCARD tstr_static json_schema_array_set_max(JsonSchemaArray* const json_schema_arr,
                                                      const size_t max) {
	if(HAS_FLAG(json_schema_arr->props.flags, JsonSchemaArrayPropertiesFlagsMax)) {
		return TSTR_STATIC_LIT("array prop error: max already set!");
	}

	json_schema_arr->props.max_items = max;
	json_schema_arr->props.flags |= JsonSchemaArrayPropertiesFlagsMax;
	return tstr_static_null();
}

void free_json_schema_array(JsonSchemaArray* const json_schema_arr) {
	RC_RELEASE(JsonSchemaArray, json_schema_arr);
}

static JsonSchemaStringProperties empty_string_props_impl(void) {

	const JsonSchemaStringProperties props = {
		.flags = JsonSchemaStringPropertiesFlagsNone,
		.min_length = 0,
		.max_length = 0,
		.pattern = NULL,
	};

	return props;
}

static void json_schema_string_destroy_impl(JsonSchemaString* const json_schema_string) {

	if(HAS_FLAG(json_schema_string->props.flags, JsonSchemaStringPropertiesFlagsPattern)) {

		assert(json_schema_string->props.pattern != NULL);

		free_json_schema_regex(json_schema_string->props.pattern);
	}
}

TJSON_NODISCARD JsonSchemaString* json_schema_string_get(void) {
	JsonSchemaString* const string = RC_ALLOC(JsonSchemaString, json_schema_string_destroy_impl);

	if(string == NULL) {
		return NULL;
	}

	string->props = empty_string_props_impl();

	return string;
}

TJSON_NODISCARD tstr_static
json_schema_string_set_nonempty(JsonSchemaString* const json_schema_str) {
	return json_schema_string_set_min(json_schema_str, 1);
}

TJSON_NODISCARD tstr_static json_schema_string_set_min(JsonSchemaString* const json_schema_str,
                                                       const size_t min) {
	if(HAS_FLAG(json_schema_str->props.flags, JsonSchemaStringPropertiesFlagsMin)) {
		return TSTR_STATIC_LIT("string prop error: min already set!");
	}

	json_schema_str->props.min_length = min;
	json_schema_str->props.flags |= JsonSchemaStringPropertiesFlagsMin;
	return tstr_static_null();
}

TJSON_NODISCARD tstr_static json_schema_string_set_max(JsonSchemaString* const json_schema_str,
                                                       const size_t max) {
	if(HAS_FLAG(json_schema_str->props.flags, JsonSchemaStringPropertiesFlagsMax)) {
		return TSTR_STATIC_LIT("string prop error: max already set!");
	}

	json_schema_str->props.max_length = max;
	json_schema_str->props.flags |= JsonSchemaStringPropertiesFlagsMax;
	return tstr_static_null();
}

TJSON_NODISCARD JsonSchemaRegex* json_schema_regex_get(const char* const str) {
	tstr temp = tstr_from(str);
	JsonSchemaRegex* const result = json_schema_regex_get_tstr(&temp);
	tstr_free(&temp);
	return result;
}

// NOTE: JsonSchemaRegex is also RCd

static void json_schema_regex_destroy_impl(JsonSchemaRegex* const json_schema_regex) {
	free_simple_regex(&(json_schema_regex->regex));
	tstr_free(&(json_schema_regex->original));
}

TJSON_NODISCARD JsonSchemaRegex* json_schema_regex_get_tstr(const tstr* const str) {

	SimpleRegexResult result = simple_regex_compile(str);

	if(result.is_error) {
		tstr_free(&result.data.error);
		return NULL;
	}

	const SimpleRegex regex = result.data.ok;

	JsonSchemaRegex* const json_schema_regex =
	    RC_ALLOC(JsonSchemaRegex, json_schema_regex_destroy_impl);

	if(json_schema_regex == NULL) {
		return NULL;
	}

	json_schema_regex->regex = regex;
	json_schema_regex->original = tstr_dup(str);

	return json_schema_regex;
}

void free_json_schema_regex(JsonSchemaRegex* const json_schema_regex) {
	RC_RELEASE(JsonSchemaRegex, json_schema_regex);
}

TJSON_NODISCARD static JsonSchemaRegex* rc_json_schema_regex(JsonSchemaRegex* regex) {
	return RC_ACQUIRE(JsonSchemaRegex, regex);
}

TJSON_NODISCARD tstr_static json_schema_string_set_regex(JsonSchemaString* const json_schema_str,
                                                         JsonSchemaRegex* const regex) {

	if(regex == NULL) {
		return TSTR_STATIC_LIT("string prop error: empty regex passed");
	}

	if(HAS_FLAG(json_schema_str->props.flags, JsonSchemaStringPropertiesFlagsPattern)) {
		return TSTR_STATIC_LIT("string prop error: pattern already set!");
	}

	json_schema_str->props.pattern = rc_json_schema_regex(regex);
	json_schema_str->props.flags |= JsonSchemaStringPropertiesFlagsPattern;
	return tstr_static_null();
}

void free_json_schema_string(JsonSchemaString* const json_schema_string) {
	RC_RELEASE(JsonSchemaString, json_schema_string);
}

static void json_schema_one_of_destroy_impl(JsonSchemaOneOf* const json_schema_one_of) {
	for(size_t i = 0; i < TVEC_LENGTH(JsonSchema, json_schema_one_of->values); ++i) {
		JsonSchema* const value = TVEC_GET_AT_MUT(JsonSchema, &(json_schema_one_of->values), i);
		free_json_schema(value);
	}
	TVEC_FREE(JsonSchema, &(json_schema_one_of->values));
}

TJSON_NODISCARD JsonSchemaOneOf* json_schema_one_of_get_empty(void) {
	JsonSchemaOneOf* const one_of = RC_ALLOC(JsonSchemaOneOf, json_schema_one_of_destroy_impl);

	if(one_of == NULL) {
		return NULL;
	}

	one_of->values = TVEC_EMPTY(JsonSchema);

	return one_of;
}

TJSON_NODISCARD tstr_static json_schema_one_of_add_entry(JsonSchemaOneOf* const json_schema_one_of,
                                                         const JsonSchema schema) {

	const TvecResult result = TVEC_PUSH(JsonSchema, &(json_schema_one_of->values), schema);

	if(result != TvecResultOk) {
		return TSTR_STATIC_LIT("json schema one of add error");
	}

	return tstr_static_null();
}

void free_json_schema_one_of(JsonSchemaOneOf* const json_schema_one_of) {
	RC_RELEASE(JsonSchemaOneOf, json_schema_one_of);
}

static void json_schema_literal_destroy_impl(JsonSchemaLiteral* const json_schema_lit) {
	tstr_free(&(json_schema_lit->value));
}

TJSON_NODISCARD static JsonSchemaLiteral* json_schema_literal_get_impl(tstr value) {
	JsonSchemaLiteral* const literal =
	    RC_ALLOC(JsonSchemaLiteral, json_schema_literal_destroy_impl);

	if(literal == NULL) {
		return NULL;
	}

	literal->value = value;

	return literal;
}

TJSON_NODISCARD JsonSchemaLiteral* json_schema_literal_get(tstr* value_moved) {
	const tstr value = *value_moved;

	return json_schema_literal_get_impl(value);
}

TJSON_NODISCARD JsonSchemaLiteral* json_schema_literal_get_dup(const tstr* value) {
	const tstr value_dup = tstr_dup(value);

	return json_schema_literal_get_impl(value_dup);
}

TJSON_NODISCARD JsonSchemaLiteral* json_schema_literal_get_cstr(const char* const value) {
	tstr value_dup = tstr_from(value);

	return json_schema_literal_get_impl(value_dup);
}

void free_json_schema_literal(JsonSchemaLiteral* const json_schema_lit) {
	RC_RELEASE(JsonSchemaLiteral, json_schema_lit);
}

void free_json_schema(JsonSchema* const json_schema) {
	SWITCH_JSON_SCHEMA(*json_schema) {
		CASE_JSON_SCHEMA_IS_OBJECT_CONST(*json_schema) {
			free_json_schema_object(object.obj);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ARRAY_CONST(*json_schema) {
			free_json_schema_array(array.arr);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NUMBER() {}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_STRING_CONST(*json_schema) {
			free_json_schema_string(string.str);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_BOOLEAN() {}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NULL() {}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ONE_OF_CONST(*json_schema) {
			free_json_schema_one_of(one_of.one_of);
		}
		break;
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_LITERAL_MUT(*json_schema) {
			free_json_schema_literal(literal.lit);
		}
		break;
		VARIANT_CASE_END();
		default: {
			break;
		}
	}

	*json_schema = new_json_schema_null();
}

#define FORMAT_TSTR(tstr_res, statement, format, ...) \
	do { \
		char* buf = NULL; \
		FORMAT_STRING(&buf, statement, format, __VA_ARGS__); \
		tstr_res = tstr_own_cstr(buf); \
	} while(false)

typedef struct {
	const JsonObjectEntrySchema* schema_ptr;
} JsonObjectSchemaCheckRequiredId;

/* NOLINTBEGIN(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */
// GCOVR_EXCL_START (external library)
TMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(JsonObjectSchemaCheckRequiredId,
                                   JsonObjectSchemaCheckRequiredIdName, bool,
                                   JsonObjectSchemaCheckRequiredMapImpl)
// GCOVR_EXCL_STOP
/* NOLINTEND(misc-use-internal-linkage,totto-function-passing-type,totto-const-correctness-c) */

TMAP_HASH_FUNC_SIG(JsonObjectSchemaCheckRequiredId, JsonObjectSchemaCheckRequiredIdName) {
	// hash ptrs
	const intptr_t ptr = (intptr_t)key.schema_ptr;

	return TMAP_HASH_SCALAR(ptr);
}

TMAP_EQ_FUNC_SIG(JsonObjectSchemaCheckRequiredId, JsonObjectSchemaCheckRequiredIdName) {
	// compare ptres
	return key1.schema_ptr == key2.schema_ptr;
}

NODISCARD static tstr
json_schema_validate_object_schema_data_impl(const JsonSchemaObject* json_schema_object,
                                             const JsonObject* const value) {

	TMAP_TYPENAME_MAP(JsonObjectSchemaCheckRequiredMapImpl)
	required_map = TMAP_EMPTY(JsonObjectSchemaCheckRequiredMapImpl);

#define FREE_AT_END() \
	do { \
		TMAP_FREE(JsonObjectSchemaCheckRequiredMapImpl, &(required_map)); \
	} while(false)

	{ // check all keys if they are allowed and check teh subschema

		JsonObjectIter* iter = json_object_get_iterator(value);

		while(true) {
			const JsonObjectEntry* entry = json_object_iterator_next(iter);

			if(entry == NULL) {
				break;
			}

			const JsonString* const key = json_object_entry_get_key(entry);

			if(key == NULL) {
				FREE_AT_END();
				return TSTR_LIT(
				    "ERROR: JsonObject implementation error: get at known good entry failed");
			}

			tstr key_str = json_string_get_as_str(key);

			if(tstr_is_null(&key_str)) {
				FREE_AT_END();
				return TSTR_LIT("ERROR: JsonString serialization error");
			}

			const JsonObjectEntrySchema* const schema_entry =
			    TMAP_GET(JsonObjectEntryMapImpl, &(json_schema_object->map), key_str);

			if(schema_entry == NULL) {

				if(json_schema_object->allow_additional_properties) {
					tstr_free(&key_str);
					// this is allowed here, but we allow any value here
					continue;
				}

				json_object_free_iterator(iter);

				tstr error;
				FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
				            , "object can't have additional properties: but got key '" TSTR_FMT "'",
				            TSTR_FMT_ARGS(key_str));

				tstr_free(&key_str);

				FREE_AT_END();
				return error;
			}

			const JsonValue sub_value = json_object_entry_get_value(entry);

			tstr subschema_result = json_schema_validate_data(&(schema_entry->schema), &sub_value);

			if(!tstr_is_null(&subschema_result)) {
				FREE_AT_END();

				// TODO: here we should add more details, that it was on an object with the key name
				return subschema_result;
			}

			if(schema_entry->required) {
				// add this to found required entries

				const JsonObjectSchemaCheckRequiredId required_key = { .schema_ptr = schema_entry };

				TmapInsertResult insert_result = TMAP_INSERT(
				    JsonObjectSchemaCheckRequiredMapImpl, &required_map, required_key, true, false);

				if(insert_result != TmapInsertResultOk) {
					FREE_AT_END();
					return TSTR_LIT("ERROR: Required tracking map impl error");
				}
			}
		}

		json_object_free_iterator(iter);
	}

	{ // check that all required values where found!

		TMAP_TYPENAME_ITER(JsonObjectEntryMapImpl)
		iter = TMAP_ITER_INIT(JsonObjectEntryMapImpl, &(json_schema_object->map));

		TMAP_TYPENAME_ENTRY(JsonObjectEntryMapImpl) entry_value;

		while(TMAP_ITER_NEXT(JsonObjectEntryMapImpl, &iter, &entry_value)) {

			const tstr obj_key = entry_value.key;
			const JsonObjectEntrySchema obj_value = entry_value.value;

			if(obj_value.required) {

				// NOTE: this is not optimal, as we query the map twice for the value, but we need
				// the "stable" ptr to that entry, for comparison, as it is much faster than other
				// means of comparing two entries by other means, as str comparison requires
				// normalization of the JsonString value first
				const JsonObjectEntrySchema* const schema_entry =
				    TMAP_GET(JsonObjectEntryMapImpl, &(json_schema_object->map), obj_key);

				if(schema_entry == NULL) {
					FREE_AT_END();
					return TSTR_LIT(
					    "ERROR: JsonObject implementation error: get at known good entry failed");
				}

				const JsonObjectSchemaCheckRequiredId required_key = { .schema_ptr = schema_entry };

				// check if we have encountered it
				const bool* const required_entry =
				    TMAP_GET(JsonObjectSchemaCheckRequiredMapImpl, &required_map, required_key);

				if(required_entry == NULL) {
					FREE_AT_END();

					tstr error;
					FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
					            , "object is missing required key '" TSTR_FMT "'",
					            TSTR_FMT_ARGS(obj_key));

					FREE_AT_END();

					return error;
				}
			}
		}
	}

	FREE_AT_END();

	return tstr_null();
}

#undef FREE_AT_END

NODISCARD static tstr
json_schema_validate_object_schema_raw_impl(const JsonSchemaObject* json_schema_object,
                                            const JsonValue* const value) {
	IF_JSON_VALUE_IS_OBJECT_CONST(*value) {
		return json_schema_validate_object_schema_data_impl(json_schema_object, object.obj);
	}

	return TSTR_LIT("JsonValue is not an object");
}

NODISCARD static tstr
json_schema_validate_array_schema_data_impl(const JsonSchemaArray* json_schema_array,
                                            const JsonArray* const value) {

	const JsonSchemaArrayProperties array_props = json_schema_array->props;

	if(HAS_FLAG(array_props.flags, JsonSchemaArrayPropertiesFlagsMin)) {

		const size_t min_items = array_props.min_items;

		const size_t size = json_array_get_size(value);

		if(min_items > size) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "array length (%zu) is smaller than the min length (%zu)", size,
			            min_items);
			return error;
		}

		// fall through to the next checks
	}

	if(HAS_FLAG(array_props.flags, JsonSchemaArrayPropertiesFlagsMax)) {

		const size_t max_items = array_props.max_items;

		const size_t size = json_array_get_size(value);

		if(max_items < size) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "array length (%zu) is larger than the max length (%zu)", size,
			            max_items);
			return error;
		}

		// fall through to the next checks
	}

	// TODO: require_unique_items is not checked yet!

	const size_t array_size = json_array_get_size(value);

	for(size_t i = 0; i < array_size; ++i) {

		const JsonValue* const sub_value = json_array_get_at(value, i);

		if(sub_value == NULL) {
			return TSTR_LIT(
			    "ERROR: JsonArray implementation error: get at known good index failed");
		}

		tstr subschema_result = json_schema_validate_data(&(json_schema_array->items), sub_value);

		if(!tstr_is_null(&subschema_result)) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "Item at index %zu in array is incorrect: " TSTR_FMT, i,
			            TSTR_FMT_ARGS(subschema_result));
			return error;
		}
	}

	return tstr_null();
}

NODISCARD static tstr
json_schema_validate_array_schema_raw_impl(const JsonSchemaArray* json_schema_array,
                                           const JsonValue* const value) {
	IF_JSON_VALUE_IS_ARRAY_CONST(*value) {
		return json_schema_validate_array_schema_data_impl(json_schema_array, array.arr);
	}

	return TSTR_LIT("JsonValue is not an array");
}

NODISCARD static tstr json_schema_validate_number_schema_raw_impl(const JsonValue* const value) {
	IF_JSON_VALUE_IS_NUMBER_IGN(*value) {
		return tstr_null();
	}

	return TSTR_LIT("JsonValue is not a number");
}

NODISCARD static tstr
json_schema_validate_string_schema_data_impl(const JsonSchemaString* json_schema_string,
                                             const JsonString* const value) {

	const JsonSchemaStringProperties string_props = json_schema_string->props;

	if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsMin)) {

		const size_t min_length = string_props.min_length;

		const size_t size = json_string_get_size(value);

		if(min_length > size) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "string size (%zu) is smaller than the min size (%zu)", size, min_length);
			return error;
		}

		// fall through to the next checks
	}

	if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsMax)) {

		const size_t max_length = string_props.max_length;

		const size_t size = json_string_get_size(value);

		if(max_length < size) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "string size (%zu) is larger than the max size (%zu)", size, max_length);
			return error;
		}

		// fall through to the next checks
	}

	if(HAS_FLAG(string_props.flags, JsonSchemaStringPropertiesFlagsPattern)) {

		const JsonSchemaRegex* pattern = string_props.pattern;
		assert(pattern != NULL);

		tstr value_str = json_string_get_as_str(value);

		if(tstr_is_null(&value_str)) {
			return TSTR_LIT("ERROR: JsonString serialization error");
		}

		const bool matches = simple_regex_match(&(pattern->regex), &value_str);

		if(!matches) {
			tstr error;
			FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
			            , "string '" TSTR_FMT "' doesn't match regex '" TSTR_FMT "'",
			            TSTR_FMT_ARGS(value_str), TSTR_FMT_ARGS((pattern->original)));

			tstr_free(&value_str);

			return error;
		}

		tstr_free(&value_str);

		// fall through to the next checks
	}

	return tstr_null();
}

NODISCARD static tstr
json_schema_validate_string_schema_raw_impl(const JsonSchemaString* json_schema_string,
                                            const JsonValue* const value) {
	IF_JSON_VALUE_IS_STRING_CONST(*value) {
		return json_schema_validate_string_schema_data_impl(json_schema_string, string);
	}

	return TSTR_LIT("JsonValue is not a string");
}

NODISCARD static tstr json_schema_validate_boolean_schema_raw_impl(const JsonValue* const value) {
	IF_JSON_VALUE_IS_BOOLEAN_IGN(*value) {
		return tstr_null();
	}

	return TSTR_LIT("JsonValue is not a boolean");
}

NODISCARD static tstr json_schema_validate_null_schema_raw_impl(const JsonValue* const value) {
	IF_JSON_VALUE_IS_NULL(*value) {
		return tstr_null();
	}

	return TSTR_LIT("JsonValue is not null");
}

// TODO(Totto): don't return a tstr, return a variant with a better error, with a JsonPath? or a
// similar hierarchy of where the error occured, with  a ref(RC REF!!) of the jsonvalue and schema
// which caused the error!

NODISCARD static tstr
json_schema_validate_one_of_schema_raw_impl(const JsonSchemaOneOf* json_schema_one_of,
                                            const JsonValue* const value) {

	// search for the first match
	for(size_t i = 0; i < TVEC_LENGTH(JsonSchema, json_schema_one_of->values); ++i) {
		const JsonSchema one_of_value = TVEC_AT(JsonSchema, (json_schema_one_of->values), i);

		tstr subschema_result = json_schema_validate_data(&one_of_value, value);

		if(tstr_is_null(&subschema_result)) {
			return tstr_null();
		}

		tstr_free(&subschema_result);
	}

	// TODO(Totto): better error reporting
	const size_t schema_amount = TVEC_LENGTH(JsonSchema, json_schema_one_of->values);

	tstr error;
	FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
	            , "JsonValue doesn't match one of the %zu subschemas", schema_amount);

	return error;
}

NODISCARD static tstr
json_schema_validate_literal_schema_data_impl(const JsonSchemaLiteral* json_schema_literal,
                                              const JsonString* const value) {

	tstr value_str = json_string_get_as_str(value);

	if(tstr_is_null(&value_str)) {
		return TSTR_LIT("ERROR: JsonString serialization error");
	}

	const bool matches = tstr_eq(&value_str, &(json_schema_literal->value));

	if(!matches) {
		tstr error;
		FORMAT_TSTR(error, OOM_ASSERT(false, "error in formatting error string");
		            , "string '" TSTR_FMT "' doesn't match literal '" TSTR_FMT "'",
		            TSTR_FMT_ARGS(value_str), TSTR_FMT_ARGS((json_schema_literal->value)));

		tstr_free(&value_str);

		return error;
	}

	tstr_free(&value_str);

	return tstr_null();
}

NODISCARD static tstr
json_schema_validate_literal_schema_raw_impl(const JsonSchemaLiteral* json_schema_literal,
                                             const JsonValue* const value) {
	IF_JSON_VALUE_IS_STRING_CONST(*value) {
		return json_schema_validate_literal_schema_data_impl(json_schema_literal, string);
	}

	return TSTR_LIT("JsonValue is not a string (literal)");
}

TJSON_NODISCARD tstr json_schema_validate_data(const JsonSchema* const schema,
                                               const JsonValue* const value) {
	SWITCH_JSON_SCHEMA(*schema) {
		CASE_JSON_SCHEMA_IS_OBJECT_CONST(*schema) {
			return json_schema_validate_object_schema_raw_impl(object.obj, value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ARRAY_CONST(*schema) {
			return json_schema_validate_array_schema_raw_impl(array.arr, value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NUMBER() {
			return json_schema_validate_number_schema_raw_impl(value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_STRING_CONST(*schema) {
			return json_schema_validate_string_schema_raw_impl(string.str, value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_BOOLEAN() {
			return json_schema_validate_boolean_schema_raw_impl(value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_NULL() {
			return json_schema_validate_null_schema_raw_impl(value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_ONE_OF_CONST(*schema) {
			return json_schema_validate_one_of_schema_raw_impl(one_of.one_of, value);
		}
		VARIANT_CASE_END();
		CASE_JSON_SCHEMA_IS_LITERAL_MUT(*schema) {
			return json_schema_validate_literal_schema_raw_impl(literal.lit, value);
		}
		VARIANT_CASE_END();
		default: {
			return TSTR_LIT("Unknown schema passed");
		}
	}
}

TJSON_NODISCARD JsonSchemaObject*
rc_json_schema_object(JsonSchemaObject* const json_schema_object) {
	return RC_ACQUIRE(JsonSchemaObject, json_schema_object);
}

TJSON_NODISCARD JsonSchemaArray* rc_json_schema_array(JsonSchemaArray* const json_schema_array) {
	return RC_ACQUIRE(JsonSchemaArray, json_schema_array);
}

TJSON_NODISCARD JsonSchemaString*
rc_json_schema_string(JsonSchemaString* const json_schema_string) {
	return RC_ACQUIRE(JsonSchemaString, json_schema_string);
}

TJSON_NODISCARD JsonSchemaLiteral*
rc_json_schema_literal(JsonSchemaLiteral* const json_schema_literal) {
	return RC_ACQUIRE(JsonSchemaLiteral, json_schema_literal);
}

TJSON_NODISCARD JsonSchemaOneOf* rc_json_schema_one_of(JsonSchemaOneOf* const json_schema_one_of) {
	return RC_ACQUIRE(JsonSchemaOneOf, json_schema_one_of);
}
