#include "utils.h"
#include "csptr.h"

#define ARRAY_SIZE 25

static enum greatest_test_res
assert_valid_array(void *ptr, size_t len, size_t size) {
    ASSERT_EQm("Mismatching array lengths.", array_length(ptr), len);
    ASSERT_EQm("Mismatching compound type sizes.", array_type_size(ptr), size);
    ASSERT_EQm("Mismatching array sizes.", array_size(ptr), size*len);
    PASS();
}

static enum greatest_test_res
assert_eq_array(int *arr1, int *arr2, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ(arr1[i], arr2[i]);
    }
    PASS();
}

TEST array(void) {
    smart int *arr = unique_ptr(int[ARRAY_SIZE], {});
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    PASS();
}
TEST array0(void) {
    smart int *arr = unique_arr(int, ARRAY_SIZE);
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    int ea[ARRAY_SIZE] = {};
    CHECK_CALL(assert_eq_array(ea, arr, ARRAY_SIZE));
    PASS();
}
TEST array1(void) {
    int va[] = {1,2,3,4,5,6,7};
    smart int *arr = shared_arr(int, 7, va);//(int[7]){1,2,3,4,5,6,7});
    CHECK_CALL(assert_valid_array(arr, 7, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, arr, 7));
    PASS();
}

TEST array2(void) {
    int sum = 0;
    f_destructor print_int = lambda(void, (void *ptr, void *meta) {
        (void) meta;
        int* pi = (int*)ptr;
        // ptr points to the current element
        // meta points to the array metadata (global to the array), if any.
        printf("dtor --> %d\n", *(int*) ptr);
        sum += *pi;
    });
    int va[] = {5, 4, 3, 2, 1};
    size_t len = sizeof(va)/sizeof(va[0]);
    int *ints = unique_arr(int, len, va, print_int);
    CHECK_CALL(assert_valid_array(ints, len, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, ints, len));

    sfree(ints);

    int expected = 0;
    for (size_t i=0; i<len; ++i) expected += va[i];
    ASSERT_EQ(expected, sum);
    PASS();
}

TEST array_dtor_run(void) {
    int dtor_run = 0;
    int *arr;
    f_destructor dtor = lambda(void, (void *ptr, UNUSED void *meta) {
            (void)ptr;
            //ASSERT_EQ(arr + dtor_run, ptr);
            dtor_run++;
        });
    arr = unique_ptr(int[ARRAY_SIZE], {}, dtor);
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", ARRAY_SIZE, dtor_run);
    PASS();
}

TEST array_meta(void) {
    smart int *arr = unique_ptr(int[ARRAY_SIZE], {}, .meta = { &m, sizeof(m) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&m, array_user_meta(arr)));
    PASS();
}

TEST array_dtor_run_with_meta(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });

    int *arr = unique_ptr(int[ARRAY_SIZE], {}, dtor, { &m, sizeof(m) });
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&m, array_user_meta(arr)));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}


GREATEST_SUITE(array_suite) {
    RUN_TEST(array);
    RUN_TEST(array0);
    RUN_TEST(array1);
    RUN_TEST(array2);
    RUN_TEST(array_dtor_run);
    RUN_TEST(array_meta);
    RUN_TEST(array_dtor_run_with_meta);
}

