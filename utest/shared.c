#include "utils.h"

#include "csptr.h"

TEST shared_init(void) {
    smart void *ptr = shared_ptr(int, 42);
    CHECK_CALL(assert_valid_ptr(ptr));
    PASS();
}

TEST shared_sref(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });
    smart void *ptr = shared_ptr(int, 42, dtor);
    CHECK_CALL(assert_valid_ptr(ptr));

    {
        smart void *ptr2 = sref(ptr);
        ASSERT_EQm("Expected reference to be the same pointer.", ptr, ptr2);
    }
    ASSERT_EQm("Expected destructor NOT to have run.", 0, dtor_run);
    PASS();
}

GREATEST_SUITE(shared_suite) {
    RUN_TEST(shared_init);
    RUN_TEST(shared_sref);
}

