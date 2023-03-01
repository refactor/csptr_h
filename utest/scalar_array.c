#include "utils.h"
#include "../csptr.h"

#define ARRAY_SIZE 25

static enum greatest_test_res
assert_valid_array(void *ptr, size_t expected_len, size_t element_size) {
    ASSERT_EQ_FMTm("Mismatching array lengths.", expected_len, array_length(ptr), "%zu");
    ASSERT_EQ_FMTm("Mismatching compound type sizes.", element_size, array_item_size(ptr), "%zu");
    ASSERT_EQ_FMTm("Mismatching array sizes.", element_size*expected_len, array_size(ptr), "%zu");
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
    typedef int(static_int)[25];
    smart static_int *arr = unique_ptr(static_int);
    ASSERT_EQ_FMTm("Mismatching array lengths.", (size_t)1, array_length(arr), "%zu");
//    CHECK_CALL(assert_valid_array(arr, 1, sizeof(int)));
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
    volatile int sum = 0;
    volatile size_t dtor_run = 0;
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
    const size_t arr_len = sizeof(va)/sizeof(va[0]);
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

TEST unique_array_uninit_dtor_run_with_meta(void) {
    int dtor_run = 0;
    size_t sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int *arr = unique_arr(int, ARRAY_SIZE, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));

    sfree(arr);
    ASSERT_EQm("Expected uninit-array set 0", 0, sum);
    ASSERT_EQm("Expected destructor to run", ARRAY_SIZE, dtor_run);
    PASS();
}

TEST unique_array_init_dtor_run_with_meta(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int ARR[] = {1, 3, 5, 7, 9, 11};
    const size_t LEN = sizeof(ARR)/sizeof(ARR[0]);
    int *arr = unique_arr(int, LEN, ARR, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, LEN, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));

    sfree(arr);
    int s = 0;
    for (size_t i = 0; i < LEN; ++i) s += ARR[i];
    ASSERT_EQm("Expected array sum", s, sum);
    ASSERT_EQm("Expected destructor to run", LEN, dtor_run);
    PASS();
}

TEST shared_array_uninit_dtor_run_with_meta(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int *arr = shared_arr(int, ARRAY_SIZE, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));

    {
        autoclean int* ptr = sref(arr);
        CHECK_CALL(assert_valid_array(ptr, ARRAY_SIZE, sizeof(int)));
        CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(ptr)));
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(arr);
    ASSERT_EQm("Expected array sum", 0, sum);
    ASSERT_EQm("Expected destructor to run", ARRAY_SIZE, dtor_run);
    PASS();
}
TEST shared_array_init_dtor_run_with_meta(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int ARR[] = {1, 3, 5, 7, 9, 11};
    const size_t LEN = sizeof(ARR)/sizeof(ARR[0]);
    int *arr = shared_arr(int, LEN, ARR, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(arr, LEN, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));

    {
        autoclean int* ptr = sref(arr);
        CHECK_CALL(assert_valid_array(ptr, LEN, sizeof(int)));
        CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(ptr)));
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(arr);
    int s = 0;
    for (size_t i = 0; i < LEN; ++i) s += ARR[i];
    ASSERT_EQm("Expected array sum", s, sum);
    ASSERT_EQm("Expected destructor to run", LEN, dtor_run);
    PASS();
}

TEST zero_array(void) {
    const size_t LEN = 0;
    smart int *arr = shared_arr(int, 0);
    ASSERT_EQm("Expected NULL for zero-array", NULL, arr);
//    printf("+++++++++= arr.len=%zu\n", array_length(arr));
//    printf("+++++++++= elem.sz=%zu\n", array_item_size(arr));
//    CHECK_CALL(assert_valid_array(arr, LEN, sizeof(int)));
    PASS();
}

GREATEST_SUITE(scalar_array_suite) {
    RUN_TEST(weird_array);
    RUN_TEST(array_uninit);
    RUN_TEST(array_cloned);
    RUN_TEST(array_with_dtor);
    RUN_TEST(zero_array);
    RUN_TEST(unique_array_uninit_dtor_run_with_meta);
    RUN_TEST(unique_array_init_dtor_run_with_meta);
    RUN_TEST(shared_array_uninit_dtor_run_with_meta);
    RUN_TEST(shared_array_init_dtor_run_with_meta);
}

