

#include "./utf8.h"

#include <utf8proc.h>

NODISCARD static inline Utf8DataResult new_utf8_data_result_error(tstr_static const error) {
	return (Utf8DataResult){ .is_error = true, .data = { .error = error } };
}

NODISCARD static inline Utf8DataResult new_utf8_data_result_ok(Utf8Data const result) {
	return (Utf8DataResult){ .is_error = false, .data = { .result = result } };
}

NODISCARD Utf8DataResult get_utf8_string(const tstr_view str_view) {

	utf8proc_int32_t* buffer = malloc(sizeof(utf8proc_int32_t) * str_view.len);

	if(!buffer) {
		return new_utf8_data_result_error(TSTR_STATIC_LIT("failed malloc"));
	}

	utf8proc_ssize_t result = utf8proc_decompose(
	    (const uint8_t*)str_view.data, (long)str_view.len, buffer, (long)str_view.len,
	    (utf8proc_option_t)0); // NOLINT(cppcoreguidelines-narrowing-conversions,clang-analyzer-optin.core.EnumCastOutOfRange)

	if(result < 0) {
		free(buffer);
		return new_utf8_data_result_error(tstr_static_from_static_cstr(utf8proc_errmsg(result)));
	}

	if(result != (utf8proc_ssize_t)str_view.len) {
		// truncate the buffer
		void* new_buffer = realloc(buffer, sizeof(utf8proc_int32_t) * result);

		if(!new_buffer) {
			free(buffer);
			return new_utf8_data_result_error(TSTR_STATIC_LIT("failed realloc"));
		}
		buffer = new_buffer;
	}

	const Utf8Data utf8_data = {
		.data = buffer,
		.size = result,
	};

	return new_utf8_data_result_ok(utf8_data);
}

void free_utf8_data(Utf8Data data) {
	free(data.data);
}
