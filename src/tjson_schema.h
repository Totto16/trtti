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

/**
 * @enum value
 */
typedef enum TJSON_C_23_NARROW_ENUM_TO(bool) {
	JsonSchemaOptionTypeCycleRef,
	JsonSchemaOptionTypeCycleThrow,
} JsonSchemaOptionTypeCycle;

/**
 * @enum value
 */
typedef enum TJSON_C_23_NARROW_ENUM_TO(bool) {
	JsonSchemaOptionTypeReusedRef,
	JsonSchemaOptionTypeReusedInline,
} JsonSchemaOptionTypeReused;

typedef struct {
	JsonSchemaOptionTypeCycle cycles;
	JsonSchemaOptionTypeReused reused;
} JsonSchemaOptions;

TJSON_NODISCARD tstr json_schema_to_string(const JsonSchema* schema);

TJSON_NODISCARD tstr json_schema_to_string_advanced(const JsonSchema* schema,
                                                    JsonSchemaOptions options);
