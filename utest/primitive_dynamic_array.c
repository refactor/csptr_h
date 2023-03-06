#include "utils.h"

static const int A[] = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10, 11};

TEST array_append(void) {
    {
        smart int *a = shared_arr(int, 5);
        ASSERT_EQ(5, static_array.capacity(a));
        ASSERT_EQ(0, static_array.length(a));

        const size_t len = LEN(A);
        for (uint32_t i = 0; i < len; ++i) {
            arrappend(a, A[i]);
        }
        ASSERT_EQ(len, static_array.length(a));
        ASSERT_LTE(len, static_array.capacity(a));
        assert_eq_arrays(A, a);
        for (uint32_t i = 0; i < len; ++i) {
            int v = arrpop(a);
            ASSERT_EQ(A[len - (i+1)], v);
            ASSERT_EQ(len - (i+1), static_array.length(a));
        }
        ASSERT_EQ(0, static_array.length(a));
    }
    PASS();
}

TEST array_delete1(void) {
    const size_t len = LEN(A);
    smart int *a = shared_arr(int, len, A);
    ASSERT_EQ(len, static_array.length(a));
    assert_eq_arrays(A, a);

    for (uint32_t i = 0; i < len; ++i) {
        ASSERT_EQ_FMT(A[i], a[0], "%d");
        arrdel(a, 0);
        ASSERT_EQ(len - (i+1), static_array.length(a));
    }
    ASSERT_EQ(0, static_array.length(a));

    PASS();
}

TEST array_deleten(void) {
    const size_t len = LEN(A);
    smart int *a = shared_arr(int, len, A);
    ASSERT_EQ(len, static_array.length(a));
    assert_eq_arrays(A, a);

    arrdeln(a, 0, len);
    ASSERT_EQ(0, static_array.length(a));

    PASS();
}

TEST array_insert(void) {
    smart uint32_t *a = shared_arr(uint32_t, 3);
    ASSERT_EQ(0, static_array.length(a));
    const uint32_t MAGIC_NUM = 9999;
    const size_t ARR_LENGTH = 100;
    arrins(a, 0, MAGIC_NUM);
    ASSERT_EQ(1, static_array.length(a));
    ASSERT_EQ(MAGIC_NUM, a[0]);

    for (uint32_t i=1; i<ARR_LENGTH; ++i) {
        arrins(a, 0, i);
        ASSERT_EQ(i, a[0]);
    }
    ASSERT_EQ(ARR_LENGTH, static_array.length(a));
    ASSERT_EQ(MAGIC_NUM, arrlast(a));
    PASS();
}


GREATEST_SUITE(primitive_dynamic_array) {
        RUN_TEST(array_append);
        RUN_TEST(array_delete1);
        RUN_TEST(array_deleten);
        RUN_TEST(array_insert);
}