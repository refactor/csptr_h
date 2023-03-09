//
// Created by wulei on 23-3-10.
//
#include "utils.h"
#include "../array2d.h"

float A[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
const int32_t Col = 3, Row = 2;

TEST array2d_init(void) {
    autoclean float* arr2d = smart_array2d(UNIQUE, float, Col, Row, A);
    ASSERT_EQ(Col * Row, (int32_t)static_array.length(arr2d));
    assert_eq_arrays(A, arr2d);

    ASSERT_EQ(A[0], array2d_get(arr2d, 0, 0));
    ASSERT_EQ(A[1], array2d_get(arr2d, 1, 0));
    ASSERT_EQ(A[2], array2d_get(arr2d, 2, 0));
    ASSERT_EQ(A[3], array2d_get(arr2d, 0, 1));
    ASSERT_EQ(A[4], array2d_get(arr2d, 1, 1));
    ASSERT_EQ(A[5], array2d_get(arr2d, 2, 1));
    PASS();
}

TEST array2d_update(void) {
   autoclean float* arr2d = smart_array2d(UNIQUE, float, Col, Row);
   for (int32_t r = 0; r < Row; ++r)
       for (int32_t c = 0; c < Col; ++c) {
           ASSERT_EQ(0, array2d_get(arr2d, c, r));
           array2d_set(arr2d, c, r, A[r * Col + c]);
       }

    ASSERT_EQ(A[0], array2d_get(arr2d, 0, 0));
    ASSERT_EQ(A[1], array2d_get(arr2d, 1, 0));
    ASSERT_EQ(A[2], array2d_get(arr2d, 2, 0));
    ASSERT_EQ(A[3], array2d_get(arr2d, 0, 1));
    ASSERT_EQ(A[4], array2d_get(arr2d, 1, 1));
    ASSERT_EQ(A[5], array2d_get(arr2d, 2, 1));
   PASS();
}

GREATEST_SUITE(primitive_array2d) {
    RUN_TEST(array2d_init);
    RUN_TEST(array2d_update);
}