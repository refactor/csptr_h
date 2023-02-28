#pragma once

#define GREATEST_USE_LONGJMP 1
#include "greatest.h"
#include <inttypes.h>
#include <stdbool.h>

__attribute__((always_inline))
inline bool is_aligned(void *ptr) {
    uintptr_t off = (uintptr_t) ptr;
    return !(off % sizeof (void *));
}

static enum greatest_test_res
assert_valid_ptr(void* ptr) {
    ASSERT_NEQm("Expected unique_ptr to return a non-null pointer.", NULL, ptr);
    ASSERT_EQm("Expected unique_ptr to return an aligned pointer.", true, is_aligned(ptr));
    PASS();
}

//*

struct my_userdata {
    int i;
    long l;
    double d;
};

extern const struct my_userdata g_metadata;

static enum greatest_test_res
assert_valid_meta(const struct my_userdata *m1, const struct my_userdata *m2) {
    ASSERT_NEQm("Expected metadata to be present", NULL, m2);
    ASSERT_NEQm("Expected metadata to be copied", m1, m2);
    const int intact = m1->i == m2->i
        && m1->l == m2->l
        && m1->d == m2->d;
    ASSERT_EQm("Expected metadata to be intact.", true, intact);
    PASS();
}

static enum greatest_test_res
assert_valid_meta_with_ASSERT_OR_LONGJMP(const struct my_userdata *m1, const struct my_userdata *m2) {
    ASSERT_OR_LONGJMPm("Expected usermeta to be present", NULL != m2);
    ASSERT_OR_LONGJMPm("Expected usermeta to be copied", m1 != m2);
    const int intact = m1->i == m2->i
        && m1->l == m2->l
        && m1->d == m2->d;
    ASSERT_OR_LONGJMPm("Expected usermeta to be intact.", intact);
    PASS();
}

// */
#define lambda(RType, Body) ({ RType __fn__ Body; __fn__; })
#define UNUSED __attribute__ ((unused))


