#include "./path.h"

#include <sys/stat.h>
#include <unistd.h>

NODISCARD static inline ReadFileResult new_read_file_result_error(tstr_static const error) {
	return (ReadFileResult){ .is_error = true, .data = { .error = error } };
}

NODISCARD static inline ReadFileResult new_read_file_result_ok(tstr const file) {
	return (ReadFileResult){ .is_error = false, .data = { .file = file } };
}

NODISCARD ReadFileResult read_entire_file(const tstr* const file_path) {

	FILE* file = fopen(tstr_cstr(file_path), "rb");

	if(file == NULL) {
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't open file for reading"));
	}

	const LibCInt fseek_res = fseek(file, 0, SEEK_END);

	if(fseek_res != 0) {
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't seek to end of file"));
	}

	const LibCLong file_size = ftell(file);

	const LibCInt fseek_res2 = fseek(file, 0, SEEK_SET);

	if(fseek_res2 != 0) {
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't seek to start of file"));
	}

	LibCChar* file_data = (LibCChar*)malloc(file_size * sizeof(LibCChar));

	if(!file_data) {
		fclose(file);
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't allocate file data"));
	}

	const size_t fread_result = fread(file_data, 1, file_size, file);

	if(fread_result != (size_t)file_size) {
		fclose(file);
		free(file_data);

		return new_read_file_result_error(
		    TSTR_STATIC_LIT("Couldn't read the correct amount of bytes from file"));
	}

	const LibCInt fclose_result = fclose(file);

	if(fclose_result != 0) {
		free(file_data);
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't close file"));
	}

	tstr result = tstr_own(file_data, file_size, file_size);

	return new_read_file_result_ok(result);
}
