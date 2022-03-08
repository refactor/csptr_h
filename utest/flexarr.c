#include "utils.h"
#include "csptr.h"
#include <stddef.h>

#define ARRAY_SIZE 25

static enum greatest_test_res
assert_valid_array(void *ptr, size_t len, size_t size) {
    ASSERT_EQm("Mismatching array lengths.", array_length(ptr), len);
    ASSERT_EQm("Mismatching compound type sizes.", array_type_size(ptr), size);
    PASS();
}

struct fa1 {
    char* name;
    int age;
    int len;
    float vf[];
};

//#define shared_fa1(...) smart_flex_arr(SHARED, struct fa1, vf, len, __VA_ARGS__)
#define shared_fa1(...) shared_flexarr(struct fa1, vf, len, __VA_ARGS__)

TEST flexarray(void) {
    float fa[] = {5.1f,4.2f,3.3f,2.4f,1.5f};
//    smart struct fa1 *farr = smart_flexible_arr(SHARED, struct fa1, vf, len, {.name="woo",.age=11,.len=5}, fa);
    smart struct fa1 *farr = shared_fa1({.name="woo", .age=42, .len=5}, fa);
    printf("farr -> %p, farr->vf=%p\n", farr,farr->vf);
    printf("farr->name: %s, %d\n", farr->name,farr->age);
    ASSERT_EQ_FMT(5, farr->len, "%u");
    for (size_t i=0; i<5; ++i)
        ASSERT_EQ_FMT(fa[i], farr->vf[i], "%f");
    //CHECK_CALL(assert_valid_array(farr->vf, 5, sizeof(float)));
    PASS();
}


TEST flexarray_dtor_run(void) {
    int dtor_run = 0;
    int the_len = -1;
    f_destructor dtor = lambda(void, (void *ptr, UNUSED void *meta) {
            struct fa1* ps = (struct fa1*)ptr;
            //ASSERT_EQ(arr + dtor_run, ptr);
            printf("ps->len: %d\n", ps->len);
            the_len = ps->len;
            dtor_run++;
        });
    struct fa1 *farr = shared_fa1({.name="woo", .age=42, .len=5}, (float[]){5.1f,4.2f,3.3f,2.4f,1.5f}, .dtor=dtor);
    //CHECK_CALL(assert_valid_array(farr, 5, sizeof(struct fa1)));
    ASSERT_EQ(5, farr->len);

    ASSERT_EQ_FMT(-1, the_len, "%d");
    sfree(farr);
    ASSERT_EQ_FMT(5, the_len, "%d");
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

TEST flexarray_meta(void) {
//    smart int *arr = unique_ptr(int[ARRAY_SIZE], {}, .meta = { &m, sizeof(m) });
    smart struct fa1 *farr = shared_fa1({.name="woo", .age=42, .len=5}, (float[]){5.1f,4.2f,3.3f,2.4f,1.5f}, .meta={&m,sizeof(m)});
    //CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    //CHECK_CALL(assert_valid_meta(&m, array_user_meta(farr)));
    void *user_meta = get_smart_ptr_meta(farr);
    CHECK_CALL(assert_valid_meta(&m, user_meta));
    PASS();
}

TEST flexarray_dtor_run_with_meta(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });

    struct fa1 *farr = shared_fa1({.name="woo", .age=42, .len=5}, (float[]){5.1f,4.2f,3.3f,2.4f,1.5f}, dtor,{&m,sizeof(m)});
    //CHECK_CALL(assert_valid_array(arr, ARRAY_SIZE, sizeof(int)));
    //CHECK_CALL(assert_valid_meta(&m, array_user_meta(arr)));
    ASSERT_EQ(5, farr->len);
    void *user_meta = get_smart_ptr_meta(farr);
    CHECK_CALL(assert_valid_meta(&m, user_meta));

    sfree(farr);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}


GREATEST_SUITE(flexarr_suite) {
    RUN_TEST(flexarray);
    RUN_TEST(flexarray_dtor_run);
    RUN_TEST(flexarray_meta);
    RUN_TEST(flexarray_dtor_run_with_meta);
}

