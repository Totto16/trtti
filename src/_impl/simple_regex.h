#pragma once

#include "../allocator.h"

#include <regex.h>
#include <tstr.h>

#include "./utils.h"

typedef struct {
	regex_t regex;
} SimpleRegex;

// manual "variant", but only used internally, so it's fine
typedef struct {
	bool is_error;
	union {
		SimpleRegex ok;
		tstr error;
	} data;
} SimpleRegexResult;

NODISCARD SimpleRegexResult simple_regex_compile(const tstr* str);

NODISCARD bool simple_regex_match(const SimpleRegex* regex, const tstr* str);

void free_simple_regex(SimpleRegex* regex);
