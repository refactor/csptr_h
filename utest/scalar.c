//#include "greatest.h"
#include "utils.h"

//#define MY_LIBCSPTR_IMPLEMENTATION
#include "../csptr.h"

TEST unique_inited(void) {
    smart int *a = unique_ptr(int, 42);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_userdata(a));
    PASS();
}

TEST shared_inited(void) {
    smart int *a = shared_ptr(int, 42);
    CHECK_CALL(assert_valid_ptr(a));
    {
        autoclean int* b = sref(a);
        ASSERT_EQ(*a, *b);
        ASSERT_EQ(a, b);
    }
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_userdata(a));
    PASS();
}

TEST unique_inited_with_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { ++dtor_run; });
    int *a = unique_ptr(int, 42, dtor);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_userdata(a));
    ASSERT_EQm("Expected destructor NOT run", 0, dtor_run);
    sfree(a);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

TEST shared_inited_with_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { ++dtor_run; });
    int *a = shared_ptr(int, 42, dtor);
    ASSERT_EQ(42, *a);
    {
        autoclean int* b = sref(a);
        ASSERT_EQ(42, *a);
        {
            autoclean int* c = sref(b);
            ASSERT_EQ(42, *c);
            {
                autoclean int* d = sref(c);
                ASSERT_EQ(42, *d);
            }
            ASSERT_EQ(0, dtor_run);
        }
        ASSERT_EQ(*a, *b);
        ASSERT_EQ(0, dtor_run);
   }
    ASSERT_EQ(0, dtor_run);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQm("Expected pointer to have no metadata", NULL, get_smart_ptr_userdata(a));
    ASSERT_EQm("Expected destructor NOT run", 0, dtor_run);
    sfree(a);
    ASSERT_EQ(1, dtor_run);
    ASSERT_EQm("Expected destructor to run", 1, dtor_run);
    PASS();
}

TEST unique_inited_with_userdata(void) {
    smart int *a = unique_ptr(int, 42, .userdata = { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(42, *a);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(a));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(a)));
    PASS();
}

TEST shared_inited_with_userdata(void) {
    smart int *a = shared_ptr(int, 42, .userdata = { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(42, *a);
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(a));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(a)));
    {
        autoclean int* b = sref(a);
        ASSERT_EQ(42, *b);
        autoclean int* c = sref(b);
        ASSERT_EQ(42, *c);
        autoclean int* d = sref(c);
        ASSERT_EQ(42, *d);
        CHECK_CALL(assert_valid_ptr(d));
        ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(d));
        CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(d)));
    }
    PASS();
}

TEST unique_inited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *userdata) { ++dtor_run; });
    int *a = unique_ptr(int, 42, dtor, { &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(a));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(a)));

    ASSERT_EQm("Expected destructor NOT run", 0, dtor_run);
    sfree(a);
    ASSERT_EQm("Expected destructor to run after free memeory", 1, dtor_run);
    PASS();
}

TEST shared_inited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *userdata) { ++dtor_run; });
    int *a = shared_ptr(int, 42, dtor, { &g_metadata, sizeof(g_metadata) });
    CHECK_CALL(assert_valid_ptr(a));
    ASSERT_EQ(42, *a);
    ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(a));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(a)));

    {
        autoclean int* b = sref(a);
        CHECK_CALL(assert_valid_ptr(b));
        ASSERT_EQ(42, *b);
        ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(b));
        CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(b)));
        {
            autoclean int* c = sref(b);
            {
                autoclean int* d = sref(c);
                CHECK_CALL(assert_valid_ptr(d));
                ASSERT_EQ(42, *d);
                ASSERT_NEQm("Expected different points", &g_metadata, get_smart_ptr_userdata(d));
                CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(d)));
            }
        }
    }

    ASSERT_EQm("Expected destructor NOT run", 0, dtor_run);
    sfree(a);
    ASSERT_EQm("Expected destructor to run after free memeory", 1, dtor_run);
    PASS();
}

GREATEST_SUITE(scalar_suite) {
    RUN_TEST(unique_inited);
    RUN_TEST(shared_inited);
    RUN_TEST(unique_inited_with_userdata);
    RUN_TEST(shared_inited_with_userdata);
    RUN_TEST(unique_inited_with_dtor);
    RUN_TEST(shared_inited_with_dtor);
    RUN_TEST(unique_inited_with_userdata_and_dtor);
    RUN_TEST(shared_inited_with_userdata_and_dtor);
}
