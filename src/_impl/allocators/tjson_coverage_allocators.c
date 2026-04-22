#include "./tjson_coverage_allocators.h"

#include <stdio.h>
#include <stdlib.h>

#include "../utils.h"

typedef enum {
	AllocatorFunctionHandleTypeFailAlways,
	AllocatorFunctionHandleTypeFailAfter,
	AllocatorFunctionHandleTypeFailNever,
} AllocatorFunctionHandleType;

// manual "variant", but only used internally, so it's fine
typedef struct {
	AllocatorFunctionHandleType type;
	union {
		struct {
			size_t count;
		} after;
	} data;
} AllocatorFunctionHandleContent;

NODISCARD static inline AllocatorFunctionHandleContent
new_allocator_function_handle_content_fail_always(void) {
	return (AllocatorFunctionHandleContent){ .type = AllocatorFunctionHandleTypeFailAlways,
		                                     .data = {} };
}

NODISCARD static inline AllocatorFunctionHandleContent
new_allocator_function_handle_content_fail_after(size_t count) {
	return (AllocatorFunctionHandleContent){ .type = AllocatorFunctionHandleTypeFailAfter,
		                                     .data = { .after = { .count = count } } };
}

NODISCARD static inline AllocatorFunctionHandleContent
new_allocator_function_handle_content_fail_never(void) {
	return (AllocatorFunctionHandleContent){ .type = AllocatorFunctionHandleTypeFailNever,
		                                     .data = {} };
}

// manual "variant", but only used internally, so it's fine
struct AllocatorFunctionHandleImpl {
	AllocatorFunctionHandleContent content;
	AllocatorFunctionType type;
};

static bool handle_should_fail_impl(AllocatorFunctionHandle* const handle) {
	switch(handle->content.type) {
		case AllocatorFunctionHandleTypeFailAlways: {
			return true;
		}
		case AllocatorFunctionHandleTypeFailAfter: {
			size_t* const count = &(handle->content.data.after.count);

			bool result = false;

			if(*count == 0) {
				result = true;
				goto change_to_never_fail;
			} else {
				result = false;
			}

			(*count)--;

			return result;
		change_to_never_fail:
			handle->content = new_allocator_function_handle_content_fail_never();
			return result;
		}
		case AllocatorFunctionHandleTypeFailNever: {
			return false;
		}
		default: {
			return true;
		}
	}
}

AllocatorFunctionHandle* malloc_handle = NULL;

void* tjson_coverage_allocator_malloc(size_t size) {
	if(malloc_handle != NULL) {
		if(handle_should_fail_impl(malloc_handle)) {
			return NULL;
		}
	}

	return malloc(size);
}

AllocatorFunctionHandle* calloc_handle = NULL;

void* tjson_coverage_allocator_calloc(size_t nmemb, size_t size) {
	if(calloc_handle != NULL) {
		if(handle_should_fail_impl(calloc_handle)) {
			return NULL;
		}
	}

	return calloc(nmemb, size);
}

AllocatorFunctionHandle* realloc_handle = NULL;

void* tjson_coverage_allocator_realloc(void* ptr, size_t size) {
	if(realloc_handle != NULL) {
		if(handle_should_fail_impl(realloc_handle)) {
			return NULL;
		}
	}

	return realloc(ptr, size);
}

void tjson_coverage_allocator_free(void* ptr) {
	// NOTE: free can't ever fail
	free(ptr);
}

AllocatorFunctionHandle* tjson_coverage_allocator_get_handle(const AllocatorFunctionType type) {

	AllocatorFunctionHandle** to_use = NULL;

	switch(type) {
		case AllocatorFunctionTypeMalloc: {
			to_use = &malloc_handle;
			break;
		}
		case AllocatorFunctionTypeCalloc: {
			to_use = &calloc_handle;
			break;
		}
		case AllocatorFunctionTypeRealloc: {
			to_use = &realloc_handle;
			break;
		}
		default: {
			return NULL;
		}
	}

	if(to_use == NULL) {
		return NULL;
	}

	if(*to_use != NULL) {
		fprintf(stderr, "Handle already retrieved, this handle can only be used once!\n");
		return NULL;
	}

	AllocatorFunctionHandle* allocated = malloc(sizeof(AllocatorFunctionHandle));

	if(allocated == NULL) {
		return NULL;
	}

	*allocated =
	    (AllocatorFunctionHandle){ .content = new_allocator_function_handle_content_fail_never(),
		                           .type = type };

	*to_use = allocated;

	return *to_use;
}

bool tjson_coverage_allocator_handle_fail_always(AllocatorFunctionHandle* const handle) {
	handle->content = new_allocator_function_handle_content_fail_always();
	return true;
}

bool tjson_coverage_allocator_handle_fail_after(AllocatorFunctionHandle* const handle,
                                                size_t count) {
	handle->content = new_allocator_function_handle_content_fail_after(count);
	return true;
}

bool tjson_coverage_allocator_handle_fail_never(AllocatorFunctionHandle* const handle) {
	handle->content = new_allocator_function_handle_content_fail_never();
	return true;
}

void tjson_coverage_allocator_free_handle(AllocatorFunctionHandle* const handle) {

	AllocatorFunctionHandle** used = NULL;

	switch(handle->type) {
		case AllocatorFunctionTypeMalloc: {
			used = &malloc_handle;
			break;
		}
		case AllocatorFunctionTypeCalloc: {
			used = &calloc_handle;
			break;
		}
		case AllocatorFunctionTypeRealloc: {
			used = &realloc_handle;
			break;
		}
		default: {
			return;
		}
	}

	free(handle);
	*used = NULL;
}
