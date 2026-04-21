#include "./path.h"

#include <sys/stat.h>
#include <unistd.h>

NODISCARD static inline ReadFileResult new_read_file_result_error(tstr_static const error) {
	return (ReadFileResult){ .is_error = true, .data = { .error = error } };
}

NODISCARD static inline ReadFileResult
new_read_file_result_ok(tstr const file) { // NOLINT(totto-function-passing-type)
	return (ReadFileResult){ .is_error = false, .data = { .file = file } };
}

NODISCARD ReadFileResult read_entire_file(const tstr* const file_path) {

	FILE* file = fopen(tstr_cstr(file_path), "rb");

	if(file == NULL) {
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't open file for reading"));
	}

#define FREE_AT_END() \
	do { \
		fclose(file); \
	} while(false)

	const LibCInt fseek_res = fseek(file, (LibCLong)(0), SEEK_END);

	if(fseek_res != 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't seek to end of file"));
	}

	const LibCLong file_size = ftell(file);

	if(file_size < 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't get the file size"));
	}

	const LibCInt fseek_res2 = fseek(file, (LibCLong)(0), SEEK_SET);

	if(fseek_res2 != 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't seek to start of file"));
	}

	LibCChar* file_data = (LibCChar*)malloc((size_t)file_size * sizeof(LibCChar));

	if(!file_data) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't allocate file data"));
	}

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		fclose(file); \
		free(file_data); \
	} while(false)

	const size_t fread_result = fread(file_data, 1, (size_t)file_size, file);

	if(fread_result != (size_t)file_size) {
		FREE_AT_END();
		return new_read_file_result_error(
		    TSTR_STATIC_LIT("Couldn't read the correct amount of bytes from file"));
	}

	const LibCInt fclose_result = fclose(file);

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		free(file_data); \
	} while(false)

	if(fclose_result != 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't close file"));
	}

	const tstr result = tstr_own(file_data, (size_t)file_size, (size_t)file_size);

	return new_read_file_result_ok(result);
}

#undef FREE_AT_END
