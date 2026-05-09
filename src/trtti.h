
#pragma once

// TODO
#define RTTI_ANNOTATED_PTR_CAST(A, b) (A*)b

typedef void* RTTIAnnotatedPtr;

//

// int *foo [[gnu::btf_decl_tag ("__percpu")]];

// int * [[gnu::btf_type_tag ("__user")]] foo;

// __PRETTY_FUNCTION__

// __typeof__

// __builtin_types_compatible_p

// __attribute__((annotate("rtti")))

/* struct __attribute__((annotate("rtti"))) Foo {
    int x;
}; */

// static_assert(__builtin_has_attribute(struct Foo, annotate), "rtti");

// static_assert(__builtin_has_attribute(struct Foo, annotate("rtti")), "rtti");

/*
#define RTTI_TYPE(T) \
    typedef T T; \
    enum { T##_has_rtti = 1 }

Then:

RTTI_TYPE(struct Foo);

and check:

#ifdef Foo_has_rtti

or via generated metadata tables.

This tends to be:

more portable
easier to debug
friendlier to tooling
more reliable than compiler-attribute introspection
Another Strong Option: Embedded Metadata
typedef struct {
    const char* type_name;
} RTTIInfo;

#define RTTI_HEADER \
    RTTIInfo* rtti

Then all RTTI-enabled structs contain:

struct Foo {
    RTTI_HEADER;
};

This is how many engines do it.

Practical Recommendation

If you're GCC/Clang-only and want compile-time enforcement:

Cleanest solution:
#define RTTI __attribute__((annotate("rtti")))

plus:

__builtin_has_attribute(type, annotate) */

struct RttiT {
	int todo;
};

#define RTTI __attribute__((designated_init))

struct __attribute__((copy((struct RttiT*)0))) C {
	int z;
};

struct RTTI Foo2 {
	int x;
};

static_assert(__builtin_has_attribute(struct Foo2, designated_init), "rtti");

struct A1 {
	int o;
};

struct A2 {
	int o;
	int l;
};

static_assert(__builtin_classify_type(struct A1) == __builtin_classify_type(struct A2));
static_assert(__builtin_classify_type(struct A1) != __builtin_classify_type(int));

static_assert(__builtin_classify_type(struct Foo) != __builtin_classify_type(int[5]));

static_assert(__builtin_classify_type(struct Foo) != __builtin_classify_type(int[5]));
static_assert(__builtin_classify_type(void*) == __builtin_classify_type(void*));
