#include "utils.h"
#include "../csptr.h"

#define ARRAY_SIZE 25

static enum greatest_test_res
assert_valid_array(void *ptr, size_t len, size_t element_size) {
    ASSERT_EQm("Mismatching array lengths.", array_length(ptr), len);
    ASSERT_EQm("Mismatching compound type sizes.", array_type_size(ptr), element_size);
    ASSERT_EQm("Mismatching array sizes.", array_size(ptr), element_size*len);
    PASS();
}

static enum greatest_test_res
assert_eq_array(int *arr1, int *arr2, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ(arr1[i], arr2[i]);
    }
    PASS();
}

TEST weird_array(void) {
    smart int *arr = unique_ptr(int[ARRAY_SIZE], {});
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    PASS();
}
TEST array_uninit(void) {
    smart int *arr = unique_arr(int, ARRAY_SIZE);
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    int ea[ARRAY_SIZE] = { 0 };
    CHECK_CALL(assert_eq_array(ea, arr, ARRAY_SIZE));
    PASS();
}
TEST array_cloned(void) {
    int va[] = {1,2,3,4,5,6,7};
    smart int *arr = shared_arr(int, 7, va);//(int[7]){1,2,3,4,5,6,7});
    CHECK_CALL(assert_valid_array(arr, 7, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, arr, 7));
    PASS();
}

TEST array_with_dtor(void) {
    int sum = 0;
    size_t dtor_run = 0;
    f_destructor arrary_element_dtor = lambda(void, (void *ptr, void *meta) {
        (void) meta;
        int* elem = (int*)ptr;
        // ptr points to the current element
        // meta points to the array metadata (global to the array), if any.
        printf("array-element-dtor(%p) = %d\n", ptr, *elem);
        sum += *elem;
        ++dtor_run;
    });
    int va[] = {5, 4, 3, 2, 1};
    size_t arr_len = sizeof(va)/sizeof(va[0]);
    int *ints = shared_arr(int, arr_len, va, arrary_element_dtor);
    CHECK_CALL(assert_valid_array(ints, arr_len, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, ints, arr_len));

    sfree(ints);
    ASSERT_EQ_FMTm("Expected every array-element be destroyed", arr_len, dtor_run, "%zu");

    int expected = 0;
    for (size_t i=0; i<arr_len; ++i) expected += va[i];
    ASSERT_EQ(expected, sum);
    PASS();
}

TEST array_meta(void) {
    smart int *arr = unique_arr(int, ARRAY_SIZE, .meta = { &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_user_meta(arr)));
    PASS();
}

TEST array_dtor_run_with_meta(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });

    int *arr = unique_arr(int, ARRAY_SIZE, .dtor=dtor, .meta={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_user_meta(arr)));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

TEST shared_array_dtor_run_with_meta(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });

    int *arr = shared_arr(int, ARRAY_SIZE, .dtor=dtor, .meta={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_user_meta(arr)));

    {
        autoclean int* ptr = sref(arr);
        CHECK_CALL(assert_valid_array(ptr, ARRAY_SIZE, sizeof(int)));
        CHECK_CALL(assert_valid_meta(&g_metadata, array_user_meta(ptr)));
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(arr);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}


GREATEST_SUITE(scalar_array_suite) {
    RUN_TEST(weird_array);
    RUN_TEST(array_uninit);
    RUN_TEST(array_cloned);
    RUN_TEST(array_with_dtor);
    RUN_TEST(array_meta);
    RUN_TEST(array_dtor_run_with_meta);
    RUN_TEST(shared_array_dtor_run_with_meta);
}

