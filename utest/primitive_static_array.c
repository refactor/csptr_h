#include "utils.h"

#define ARRAY_SIZE 25


static enum greatest_test_res
assert_eq_array(int *arr1, int *arr2, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ_FMT(arr1[i], arr2[i], "%d");
    }
    PASS();
}

TEST weird_point(void) {
    typedef int(static_int)[25];
    smart static_int *arr = unique_ptr(static_int);
    ASSERT_EQ_FMTm("Mismatching array lengths.", (size_t)1, static_array.length(arr), "%zu");
//    CHECK_CALL(assert_valid_array(arr, 1, sizeof(int)));
    PASS();
}

TEST unique_uninited(void) {
    smart int *arr = unique_arr(int, ARRAY_SIZE);
    ASSERT_EQ(0, static_array.length(arr));
    ASSERT_EQ(ARRAY_SIZE, static_array.capacity(arr));
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    int ea[ARRAY_SIZE] = { 0 };
    CHECK_CALL(assert_eq_array(ea, arr, ARRAY_SIZE));
    PASS();
}

TEST shared_inited(void) {
    int va[] = {1,2,3,4,5,6,7};
    const size_t len = LEN(va);
    smart int *arr = shared_arr(int, len, va);
    ASSERT_EQ(len, static_array.length(arr));
    ASSERT_EQ(len, static_array.capacity(arr));
    CHECK_CALL(assert_valid_array(arr, len, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, arr, len));
    ASSERT_EQ(NULL, get_smart_ptr_userdata(arr));
    PASS();
}

TEST shared_inited_with_dtor(void) {
    volatile int sum = 0;
    volatile size_t dtor_run = 0;
    f_destructor arrary_element_dtor = lambda(void, (void *ptr, void *userdata) {
        // userdata points to the array userdata (global to the array), if any.
        ASSERT_OR_LONGJMPm("Expected usermeta to be copied", NULL == userdata);
        int* elem = (int*)ptr;
        // ptr points to the current element
        sum += *elem;
        ++dtor_run;
    });
    int va[] = {5, 4, 3, 2, 1};
    const size_t len = LEN(va);
    int *ints = shared_arr(int, len, va, arrary_element_dtor);
    ASSERT_EQ(len, static_array.length(ints));
    ASSERT_EQ(len, static_array.capacity(ints));
    CHECK_CALL(assert_valid_array(ints, len, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, ints, len));

    sfree(ints);
    ASSERT_EQ_FMTm("Expected every array-element be destroyed", len, dtor_run, "%zu");

    int expected = 0;
    for (size_t i=0; i < len; ++i) expected += va[i];
    ASSERT_EQ(expected, sum);
    PASS();
}

TEST unique_uninited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    size_t sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int *ints = unique_arr(int, ARRAY_SIZE, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_array(ints, ARRAY_SIZE, sizeof(int)));
    ASSERT_EQ(0, static_array.length(ints));
    ASSERT_EQ(ARRAY_SIZE, static_array.capacity(ints));
    int va[ARRAY_SIZE]  = { 0 };
    CHECK_CALL(assert_eq_array(va, ints, ARRAY_SIZE));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(ints)));

    sfree(ints);
    ASSERT_EQm("Expected uninit-array set 0", 0, sum);
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    PASS();
}

TEST unique_inited_with_userdata_and_dtor(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int va[] = {1, 3, 5, 7, 9, 11};
    const size_t len = LEN(va);
    int *ints = unique_arr(int, len, va, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ_FMT(len, static_array.length(ints), "%zu");
    ASSERT_EQ(len, static_array.capacity(ints));
    CHECK_CALL(assert_valid_array(ints, len, sizeof(int)));
    CHECK_CALL(assert_eq_array(va, ints, len));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(ints)));

    sfree(ints);
    int s = 0;
    for (size_t i = 0; i < len; ++i) s += va[i];
    ASSERT_EQm("Expected array sum", s, sum);
    ASSERT_EQm("Expected destructor to run", len, dtor_run);
    PASS();
}

TEST shared_uninited_with_userdata_and_dtor(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int *arr = shared_arr(int, ARRAY_SIZE, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(0, static_array.length(arr));
    ASSERT_EQ(ARRAY_SIZE, static_array.capacity(arr));
    CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));

    {
        autoclean int* ptr = sref(arr);
        CHECK_CALL(assert_valid_array(ptr, ARRAY_SIZE, sizeof(int)));
        CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(ptr)));
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(arr);
    ASSERT_EQm("Expected array sum", 0, sum);
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    PASS();
}

TEST shared_inited_with_userdata_and_dtor(void) {
    size_t dtor_run = 0;
    int sum = 0;
    f_destructor dtor = lambda(void, (void *ptr, void *meta) {
        struct my_userdata* m = meta;
        assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, m);
        sum += *(int*)ptr;
        dtor_run++;
    });

    int ARR[] = {1, 3, 5, 7, 9, 11};
    const size_t len = LEN(ARR);
    int *arr = shared_arr(int, len, ARR, .dtor=dtor, .userdata={ &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(len, static_array.length(arr));
    ASSERT_EQ(len, static_array.capacity(arr));
    CHECK_CALL(assert_valid_array(arr, len, sizeof(int)));
    CHECK_CALL(assert_eq_array(ARR, arr, len));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));

    {
        autoclean int* ptr = sref(arr);
        CHECK_CALL(assert_valid_array(ptr, len, sizeof(int)));
        CHECK_CALL(assert_eq_array(ARR, ptr, len));
        CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(ptr)));
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(arr);
    int s = 0;
    for (size_t i = 0; i < len; ++i) s += ARR[i];
    ASSERT_EQm("Expected array-item-destructor to run, by sum array", s, sum);
    ASSERT_EQm("Expected array-item-destructor to run", len, dtor_run);
    PASS();
}

TEST zero_array(void) {
    smart int *arr = shared_arr(int, 0);
    ASSERT_EQm("Expected NULL for zero-array", NULL, arr);
    PASS();
}

TEST append(void) {
    {
        smart int *a = shared_arr(int, 5);
        ASSERT_EQ(5, static_array.capacity(a));
        ASSERT_EQ(0, static_array.length(a));

        int A[] = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10, 11};
        const size_t len = LEN(A);
        for (uint32_t i = 0; i < len; ++i) {
            arrappend(a, A[i]);
            printf("arr: capacity=%zu, length=%zu, A[%u/%zu]=%d, a[%u]=%d\n", static_array.capacity(a), static_array.length(a), i, len, A[i], i, a[i]);
        }
    }
    PASS();
}

GREATEST_SUITE(primitive_static_array) {
    RUN_TEST(weird_point);
    RUN_TEST(unique_uninited);
    RUN_TEST(shared_inited);
    RUN_TEST(shared_inited_with_dtor);
    RUN_TEST(zero_array);
    RUN_TEST(unique_uninited_with_userdata_and_dtor);
    RUN_TEST(unique_inited_with_userdata_and_dtor);
    RUN_TEST(shared_uninited_with_userdata_and_dtor);
    RUN_TEST(shared_inited_with_userdata_and_dtor);
    RUN_TEST(append);
}

