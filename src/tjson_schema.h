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

typedef struct JsonSchemaOneOfImpl JsonSchemaOneOf;

GENERATE_VARIANT_ALL_JSON_SCHEMA()

TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* schema);
