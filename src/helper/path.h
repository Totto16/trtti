

#pragma once

#include "../utils.h"

#include <tstr.h>

// manual "variant", but only used internally, so it's fine
typedef struct {
	bool is_error;
	union {
		tstr_static error;
		tstr file;
	} data;
} ReadFileResult;

NODISCARD ReadFileResult read_entire_file(const tstr* file_path);
