#include "./path.h"

#include <errno.h>
#include <fcntl.h>
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

	int fd = open(tstr_cstr(file_path), O_RDONLY);

	if(fd < 0) {
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't open file for reading"));
	}

#define FREE_AT_END() \
	do { \
		close(fd); \
	} while(false)

	struct stat statbuf;

	int result = fstat(fd, &statbuf);

	if(result != 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't get stats of the file"));
	}

	if(statbuf.st_size < 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't get the file size"));
	}

	const size_t file_size = (size_t)statbuf.st_size;

	LibCChar* const file_data = (LibCChar*)TJSON_MALLOC(file_size * sizeof(LibCChar));

	if(!file_data) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't allocate file data"));
	}

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		close(fd); \
		TJSON_FREE(file_data); \
	} while(false)

	{
		LibCChar* buf = file_data;
		size_t remaining_size = file_size;

		while(true) {

			const ssize_t read_result = read(fd, buf, remaining_size);

			if(read_result < 0) {
				if(errno == EINTR) {
					// try again
					continue;
				}

				FREE_AT_END();
				return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't read from the file"));
			}

			if(read_result == 0) {
				break;
			}

			size_t read_amount = (size_t)read_result;

			if(read_amount == remaining_size) {
				break;
			}

			if(read_amount > remaining_size) {
				// logic or libc error
				UNREACHABLE();
			}

			remaining_size -= read_amount;
			buf = buf + read_amount;
		}
	}

	const LibCInt close_result = close(fd);

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		TJSON_FREE(file_data); \
	} while(false)

	if(close_result != 0) {
		FREE_AT_END();
		return new_read_file_result_error(TSTR_STATIC_LIT("Couldn't close file"));
	}

	const tstr result_str = tstr_own(file_data, file_size, file_size);

	return new_read_file_result_ok(result_str);
}

#undef FREE_AT_END
