#pragma once

#include "./tjson.h"

#define VARIANT_IMPL_JSON_SCHEMA_VARIANTS_COMPILED_WITH_NARROWED_ENUMS \
	TJSON_VARIANT_IMPL_COMPILED_WITH_NARROWED_ENUMS

#include "./tjson/variants.h"

typedef struct JsonSchemaObjectImpl JsonSchemaObject;

typedef struct JsonSchemaArrayImpl JsonSchemaArray;

typedef struct JsonSchemaStringImpl JsonSchemaString;

typedef struct {
	tstr value;
} JsonSchemaLiteral;

#define JSON_SCHEMA_LIT(val) ((JsonSchemaLiteral){ .value = TSTR_LIT(val) })

void free_json_schema_literal(JsonSchemaLiteral* json_schema_lit);

typedef struct JsonSchemaOneOfImpl JsonSchemaOneOf;

GENERATE_VARIANT_ALL_JSON_SCHEMA()

TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* schema);

TJSON_NODISCARD JsonSchemaObject* json_schema_object_get(bool allow_additional_properties);

TJSON_NODISCARD tstr_static json_schema_object_add_entry(JsonSchemaObject* json_schema_object,
                                                         tstr* moved_key, JsonSchema value,
                                                         bool required);

TJSON_NODISCARD tstr_static json_schema_object_add_entry_dup(JsonSchemaObject* json_schema_object,
                                                             const tstr* key, JsonSchema value,
                                                             bool required);

void free_json_schema_object(JsonSchemaObject* json_schema_object);

TJSON_NODISCARD JsonSchemaArray* json_schema_array_get(JsonSchema items, bool require_unique_items);

TJSON_NODISCARD tstr_static json_schema_array_set_min(JsonSchemaArray* json_schema_arr, size_t min);

TJSON_NODISCARD tstr_static json_schema_array_set_max(JsonSchemaArray* json_schema_arr, size_t max);

void free_json_schema_array(JsonSchemaArray* json_schema_arr);

TJSON_NODISCARD JsonSchemaString* json_schema_string_get(void);

TJSON_NODISCARD tstr_static json_schema_string_set_nonempty(JsonSchemaString* json_schema_str);

TJSON_NODISCARD tstr_static json_schema_string_set_min(JsonSchemaString* json_schema_str,
                                                       size_t min);

TJSON_NODISCARD tstr_static json_schema_string_set_max(JsonSchemaString* json_schema_str,
                                                       size_t max);

typedef struct JsonSchemaRegexImpl JsonSchemaRegex;

TJSON_NODISCARD JsonSchemaRegex* json_schema_regex_get(const char* str);

TJSON_NODISCARD JsonSchemaRegex* json_schema_regex_get_tstr(const tstr* str);

void free_json_schema_regex(JsonSchemaRegex* json_schema_regex);

TJSON_NODISCARD tstr_static json_schema_string_set_regex(JsonSchemaString* json_schema_str,
                                                         JsonSchemaRegex* regex);

void free_json_schema_string(JsonSchemaString* json_schema_string);

TJSON_NODISCARD JsonSchemaOneOf* json_schema_one_of_get_empty(void);

TJSON_NODISCARD tstr_static json_schema_one_of_add_entry(JsonSchemaOneOf* json_schema_one_of,
                                                         JsonSchema schema);

void free_json_schema_one_of(JsonSchemaOneOf* json_schema_one_of);

void free_json_schema(JsonSchema* json_schema);
