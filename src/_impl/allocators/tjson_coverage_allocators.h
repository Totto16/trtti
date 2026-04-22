#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* tjson_coverage_allocator_malloc(size_t size);

void* tjson_coverage_allocator_calloc(size_t nmemb, size_t size);

void* tjson_coverage_allocator_realloc(void* ptr, size_t size);

void tjson_coverage_allocator_free(void* ptr);

typedef enum {
	AllocatorFunctionTypeMalloc,
	AllocatorFunctionTypeCalloc,
	AllocatorFunctionTypeRealloc,
} AllocatorFunctionType;

typedef struct AllocatorFunctionHandle AllocatorFunctionHandle;

AllocatorFunctionHandle* tjson_coverage_allocator_get_handle(AllocatorFunctionType type);

bool tjson_coverage_allocator_handle_fail_always(AllocatorFunctionHandle* handle);

bool tjson_coverage_allocator_handle_fail_after(AllocatorFunctionHandle* handle, size_t count);

bool tjson_coverage_allocator_handle_fail_never(AllocatorFunctionHandle* handle);

void tjson_coverage_allocator_free_handle(AllocatorFunctionHandle* handle);

#ifdef __cplusplus
}
#endif
