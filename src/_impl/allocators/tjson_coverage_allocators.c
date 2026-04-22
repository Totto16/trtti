#include "./tjson_coverage_allocators.h"

#include <stdlib.h>

#include "../utils.h"

void* tjson_coverage_allocator_malloc(size_t size) {
	// TODO: add inststrumented behaviour
	return malloc(size);
}

void* tjson_coverage_allocator_calloc(size_t nmemb, size_t size) {
	// TODO: add inststrumented behaviour
	return calloc(nmemb, size);
}

void* tjson_coverage_allocator_realloc(void* ptr, size_t size) {
	// TODO: add inststrumented behaviour
	return realloc(ptr, size);
}

void tjson_coverage_allocator_free(void* ptr) {
	// TODO: add inststrumented behaviour
	free(ptr);
}

struct AllocatorFunctionHandle {
	int todo;
};

AllocatorFunctionHandle* tjson_coverage_allocator_get_handle(AllocatorFunctionType type) {
	// TODO
	UNUSED(type);
	return NULL;
}

bool tjson_coverage_allocator_handle_fail_always(AllocatorFunctionHandle* handle) {
	// TODO
	UNUSED(handle);
	return false;
}

bool tjson_coverage_allocator_handle_fail_after(AllocatorFunctionHandle* handle, size_t count) {
	// TODO
	UNUSED(handle);
	UNUSED(count);
	return false;
}

bool tjson_coverage_allocator_handle_fail_never(AllocatorFunctionHandle* handle) {
	// TODO
	UNUSED(handle);
	return false;
}

void tjson_coverage_allocator_free_handle(AllocatorFunctionHandle* handle) {
	// TODO
	UNUSED(handle);
}
