#pragma once
#include "../csptr.h"

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

static enum greatest_test_res
assert_valid_array(void *ptr, size_t expected_cap, size_t element_size) {
    ASSERT_EQ_FMTm("Mismatching array lengths.", expected_cap, static_array.capacity(ptr), "%zu");
    ASSERT_EQ_FMTm("Mismatching compound type sizes.", element_size, static_array.item_size(ptr), "%zu");
    size_t total_size = static_array.item_size(ptr) * static_array.capacity(ptr);
    ASSERT_EQ_FMTm("Mismatching array sizes.", element_size * expected_cap, total_size, "%zu");
    PASS();
}

//*

struct my_userdata {
    int i;
    long l;
    double d;
};

extern const struct my_userdata g_metadata;


#define assert_eq_arrays(a1, a2) do{ \
    size_t len = static_array.length(a2); \
    for (size_t i = 0; i < len; ++i) {      \
        ASSERT_EQ(a1[i], a2[i]);                            \
    }                                    \
}while(0)

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

#define LEN(A) (sizeof(A)/sizeof(A[0]))

