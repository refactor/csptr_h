#include "utils.h"
#include "../csptr.h"

#define ARRAY_SIZE 25

typedef struct anim {
    const char* name;
    int age;
    double weight;
} anim;


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

TEST unique_uninit(void) {
    smart anim *arr = unique_arr(anim, LEN(as));
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(0, static_array.length(arr));
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    ASSERT_EQ(LEN(as), static_array.capacity(arr));

    anim emptys[] ={{},{}};
    CHECK_CALL(assert_eq_array(emptys, arr, LEN(emptys)));
    PASS();
}

TEST unique_inited(void) {
    smart anim *arr = unique_arr(anim, LEN(as), as);
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(LEN(as), static_array.length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));
    PASS();
}

TEST unique_inited_with_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr,  void *userdata) {
            ASSERT_OR_LONGJMPm("Expected usermeta to be copied", NULL == userdata);
            dtor_run++;
        });
    anim *arr = unique_arr(anim, LEN(as), as, dtor);
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(LEN(as), static_array.length(arr));
    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", LEN(as), dtor_run);
    PASS();
}

TEST unique_inited_with_userdata(void) {
    smart anim *arr = unique_arr(anim, LEN(as), as, .userdata= { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(LEN(as), static_array.length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));
    ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr));
    PASS();
}

TEST unique_inited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, void *userdata) {
            struct my_userdata* pm = (struct my_userdata*)userdata;
            assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, pm);
            dtor_run++;
        });
 
    anim *arr = unique_arr(anim, LEN(as), as, dtor, { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(LEN(as), static_array.length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));
    ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", LEN(as), dtor_run);
    PASS();
}

TEST shared_inited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, void *userdata) {
            struct my_userdata* pm = (struct my_userdata*)userdata;
            assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, pm);
            dtor_run++;
        });

    anim *arr = shared_arr(anim, LEN(as), as, dtor, { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(LEN(as), static_array.capacity(arr));
    ASSERT_EQ(LEN(as), static_array.length(arr));
    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, LEN(as), sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));
    ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr));
    {
        autoclean anim* arr2 = sref(arr);
        autoclean anim* arr3 = sref(arr);
        {
            autoclean anim* arr4 = sref(arr);
            {
                autoclean anim* arr5 = sref(arr4);

                ASSERT_EQm("Expected destructor to run", 0, dtor_run);
            }
            ASSERT_EQm("Expected destructor to run", 0, dtor_run);
        }
        ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    }
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", LEN(as), dtor_run);
    PASS();
}

TEST unique_uninited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, void *userdata) {
            struct my_userdata* pm = (struct my_userdata*)userdata;
            assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, pm);
            dtor_run++;
        });

    const size_t len = LEN(as);
    anim *arr = unique_arr(anim, len, .dtor=dtor, { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(len, static_array.capacity(arr));
    ASSERT_EQ(0, static_array.length(arr));
//    CHECK_CALL(assert_eq_array(as, arr, LEN(as)));

    CHECK_CALL(assert_valid_array(arr, len, sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));
    ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    PASS();
}

TEST shared_uninited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, void *userdata) {
            struct my_userdata* pm = (struct my_userdata*)userdata;
            assert_valid_meta_with_ASSERT_OR_LONGJMP(&g_metadata, pm);
            dtor_run++;
    });

    const size_t len = LEN(as);
    anim *arr = shared_arr(anim, len, .dtor=dtor, { &g_metadata, sizeof(g_metadata) });
    ASSERT_EQ(len, static_array.capacity(arr));
    ASSERT_EQ(0, static_array.length(arr));

    {
        autoclean anim* arr2 = sref(arr);
        {
            autoclean anim* arr3 = sref(arr);
            autoclean anim* arr4 = sref(arr);
            {
                autoclean anim* arr5 = sref(arr4);
                ASSERT_EQ(len, static_array.capacity(arr5));
                ASSERT_EQ(0, static_array.length(arr5));

                CHECK_CALL(assert_valid_array(arr5, len, sizeof(struct anim)));
                CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr5)));
                ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr5));
                ASSERT_EQm("Expected destructor to run", 0, dtor_run);
            }
            ASSERT_EQm("Expected destructor to run", 0, dtor_run);
        }
        ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    }
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    CHECK_CALL(assert_valid_array(arr, len, sizeof(struct anim)));
    CHECK_CALL(assert_valid_meta(&g_metadata, get_smart_ptr_userdata(arr)));
    ASSERT_NEQ(&g_metadata, get_smart_ptr_userdata(arr));

    sfree(arr);
    ASSERT_EQm("Expected destructor to run", 0, dtor_run);
    PASS();
}

GREATEST_SUITE(struct_static_array) {
    RUN_TEST(unique_uninit);
    RUN_TEST(unique_inited);
    RUN_TEST(unique_inited_with_dtor);
    RUN_TEST(unique_inited_with_userdata);
    RUN_TEST(unique_inited_with_userdata_and_dtor);
    RUN_TEST(shared_inited_with_userdata_and_dtor);
    RUN_TEST(unique_uninited_with_userdata_and_dtor);
    RUN_TEST(shared_uninited_with_userdata_and_dtor);
}

