//#include "greatest.h"
#include "utils.h"

//#define MY_LIBCSPTR_IMPLEMENTATION
#include "csptr.h"

TEST pointer_valid(void) {
    smart int *a = unique_ptr(int, 42);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_meta(a));
    PASS();
}

TEST dtor_run(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });
    int *a = unique_ptr(int, 42, dtor);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_meta(a));
    sfree(a);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

TEST meta_test(void) {
    smart int *a = unique_ptr(int, 42, .meta = { &m, sizeof(m) });
    CHECK_CALL(assert_valid_ptr(a));
    CHECK_CALL(assert_valid_meta(&m, get_smart_ptr_meta(a)));
    PASS();
}

TEST dtor_run_with_meta(void) {
    int dtor_run = 0;

    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });
    int *a = unique_ptr(int, 42, dtor, { &m, sizeof(m) });
    CHECK_CALL(assert_valid_ptr(a));
    CHECK_CALL(assert_valid_meta(&m, get_smart_ptr_meta(a)));

    sfree(a);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

GREATEST_SUITE(scalar_suite) {
    RUN_TEST(pointer_valid);
    RUN_TEST(meta_test);
    RUN_TEST(dtor_run);
    RUN_TEST(dtor_run_with_meta);
}

/*
GREATEST_MAIN_DEFS();

int main(int argc, char* argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(scalar_suite);


    GREATEST_MAIN_END();
}
// */
