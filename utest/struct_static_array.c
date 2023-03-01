#include "utils.h"
#include "../csptr.h"

#define ARRAY_SIZE 25

static enum greatest_test_res
assert_valid_array(void *ptr, size_t len, size_t size) {
    ASSERT_EQm("Mismatching array lengths.", array_length(ptr), len);
    ASSERT_EQm("Mismatching compound type sizes.", array_item_size(ptr), size);
    ASSERT_EQm("Mismatching array sizes.", array_size(ptr), len*size);
    PASS();
}

typedef struct anim {
    const char* name;
    int age;
    double weight;
} anim;

#define LEN(A) (sizeof(A)/sizeof(A[0]))

anim as[] = {{
    .name = "dog",
    .age = 2,
    .weight = 3.14
}, {
    .name = "cat",
    .age = 1,
    .weight = 2.72
}};

static enum greatest_test_res
assert_eq_array(anim *arr1, anim *arr2, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (arr1[i].name == NULL)
            ASSERT(arr2[i].name == NULL);
        else
            ASSERT_STR_EQ(arr1[i].name, arr2[i].name);
        ASSERT_EQ(arr1[i].age, arr2[i].age);
        ASSERT_EQ(arr1[i].weight, arr2[i].weight);
    }
    PASS();
}

TEST uninit_array(void) {
    smart anim *arr = unique_arr(anim, LEN(as));
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    ASSERT_EQ(LEN(as), array_length(arr));

    anim emptys[] ={{},{}};
    CHECK_CALL(assert_eq_array(emptys, arr, LEN(emptys)));
    PASS();
}

TEST array(void) {
    smart anim *arr = unique_arr(anim, LEN(as), as);
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    ASSERT_EQ(LEN(as), array_length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));
    PASS();
}

TEST array_dtor_run(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr,  void *userdata) {
            ASSERT_OR_LONGJMPm("Expected usermeta to be copied", NULL == userdata);
            dtor_run++;
        });
    anim *arr = unique_arr(anim, LEN(as), as, dtor);
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    ASSERT_EQ(LEN(as), array_length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", LEN(as), dtor_run);
    PASS();
}

TEST array_meta(void) {
    smart anim *arr = unique_arr(anim, LEN(as), as, .userdata= { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(LEN(as), array_length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));
    ASSERT_NEQ(&g_metadata, array_userdata(arr));
    PASS();
}

TEST array_dtor_run_with_meta(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, void *userdata) {
            struct my_userdata* pm = (struct my_userdata*)userdata;
            assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, pm);
            dtor_run++;
        });
 
    anim *arr = unique_arr(anim, LEN(as), as, dtor, { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(LEN(as), array_length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, array_userdata(arr)));
    ASSERT_NEQ(&g_metadata, array_userdata(arr));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", LEN(as), dtor_run);
    PASS();
}


GREATEST_SUITE(struct_static_array) {
    RUN_TEST(uninit_array);
    RUN_TEST(array);
    RUN_TEST(array_dtor_run);
    RUN_TEST(array_meta);
    RUN_TEST(array_dtor_run_with_meta);
}
