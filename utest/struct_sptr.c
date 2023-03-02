#include "utils.h"

#include "../csptr.h"

typedef struct dog {
    const char* name;
    int age;
    int weight;
} dog;

TEST shared_uninit(void) {
    smart dog *ptr = shared_ptr(dog);
    CHECK_CALL(assert_valid_ptr(ptr));
    ASSERT_EQ(NULL, ptr->name);
    ASSERT_EQ(0, ptr->age);
    ASSERT_EQ(0, ptr->weight);
    ASSERT_EQm("Expected pointer to have no usermeta", NULL, get_smart_ptr_userdata(ptr));
    PASS();
}

TEST unique_uninited(void) {
    smart dog *ptr = unique_ptr(dog);
    CHECK_CALL(assert_valid_ptr(ptr));
    ASSERT_EQ(NULL, ptr->name);
    ASSERT_EQ(0, ptr->age);
    ASSERT_EQ(0, ptr->weight);
    ASSERT_EQm("Expected pointer to have no usermeta", NULL, get_smart_ptr_userdata(ptr));
    PASS();
}

static int dead_dog_count = 0;
void tombed_dog(void* ptr, UNUSED void* meta) {
    dog* d = (dog*)ptr;
    ASSERT_OR_LONGJMPm("Expected unset struct.member will be init to ZERO", 0 == d->weight);
    ++dead_dog_count;
}

TEST unique_uninited_with_dtor(void) {
    dead_dog_count = 0;
    {
        autoclean dog *ptr = unique_ptr(dog, .dtor=tombed_dog);
        CHECK_CALL(assert_valid_ptr(ptr));
        ASSERT_EQ(NULL, ptr->name);
        ASSERT_EQ(0, ptr->age);
        ASSERT_EQ(0, ptr->weight);
        ASSERT_EQm("Expected pointer to have no usermeta", NULL, get_smart_ptr_userdata(ptr));
        ASSERT_EQ(0, dead_dog_count);
    }
    ASSERT_EQ(1, dead_dog_count);
    PASS();
}

TEST unique_inited_with_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *userdata) { dtor_run = 1; });
    {
        autoclean dog *ptr = unique_ptr(dog, { .name="Tom", .age=42 }, dtor);
        CHECK_CALL(assert_valid_ptr(ptr) );
        ASSERT_EQ("Tom", ptr->name);
        ASSERT_EQ(42, ptr->age);
        ASSERT_EQ(0, ptr->weight);
        ASSERT_EQ(0, dtor_run);
    }
//    sfree(ptr);
    ASSERT_EQ(1, dtor_run);
    PASS();
}

TEST shared_inited_with_dtor(void) {
    dead_dog_count = 0;
    {
        smart dog *ptr;
        {
            const char *name = "Tom";
            smart dog *ptr1 = shared_ptr(dog, {.name=name, .age=7}, tombed_dog);
            CHECK_CALL(assert_valid_ptr(ptr1));
            ASSERT_EQ(name, ptr1->name);
            ASSERT_EQ(7, ptr1->age);
           ptr = sref(ptr1);
            ASSERT_EQm("Expected pointer to have no userdata", NULL, get_smart_ptr_userdata(ptr));
        }
        ASSERT_EQm("expected dead dog is 0", 0, dead_dog_count);

        {
            const char *name = "tommason";
            smart dog *ptr2 = shared_ptr(dog, {name,10}, tombed_dog);
            CHECK_CALL(assert_valid_ptr(ptr2));
            ASSERT_EQ(name, ptr2->name);
            ASSERT_EQ_FMT(10, ptr2->age, "%d");
            ASSERT_EQm("Expected pointer to have no userdata", NULL, get_smart_ptr_userdata(ptr));

            ASSERT_EQm("expected dead dog is 0", 0, dead_dog_count);
        }
        ASSERT_EQm("expected dead dog is 1", 1, dead_dog_count);
    }
    ASSERT_EQ_FMTm("expected dead dog is 2", 2, dead_dog_count, "%d");
//    sfree(ptr);
    PASS();
}

TEST unique_inited_with_userdata_and_dtor(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *userdata) {
        struct my_userdata* ud = userdata;
        ASSERT_OR_LONGJMP(g_metadata.d == ud->d);
        ASSERT_OR_LONGJMP(g_metadata.l == ud->l);
        ASSERT_OR_LONGJMP(g_metadata.i == ud->i);
        ++dtor_run;
    });
    {
        autoclean dog *ptr = NULL;
        {
            ptr = unique_ptr(dog, { .name="Tom", .age=42 }, dtor, .userdata={&g_metadata, sizeof(g_metadata)} );
            CHECK_CALL(assert_valid_ptr(ptr));

            ASSERT_EQm("Expected destructor NOT to have run.", 0, dtor_run);
        }
    }
//    sfree(ptr);
    ASSERT_EQm("Expected destructor have run.", 1, dtor_run);
    PASS();
}

TEST shared_inited_with_userdata_and_dtor(void) {
    struct owner {
        const char* master;
        int lost_dog_num;
    } um = {
        .master = "Bill Gates",
        .lost_dog_num = 42
    };
    int dtor_run = 0;
    dog *ptr = NULL;
    {
        f_destructor dtor = lambda(void, (void *ptr, void *userdata) {
            dog* a = (dog*)ptr;
            ASSERT_OR_LONGJMPm("Expected unset struct.member will be init to ZERO", 0 == a->weight);

                struct owner* pm = (struct owner*)userdata;
 //           struct meta* pm = (struct meta*)array_userdata(meta);
            if (dtor_run == 0) dtor_run = pm->lost_dog_num;
            dtor_run++;
        });
        const char* name = "Tommason";
        smart dog *ptr1 = shared_ptr(dog, {.name=name,.age=11}, .dtor=dtor, .userdata={&um, sizeof(um)});
        CHECK_CALL(assert_valid_ptr(ptr1));
        ASSERT_EQ(name, ptr1->name);
        ASSERT_EQ_FMT(11, ptr1->age, "%d");
        ptr = sref(ptr1);
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(ptr);
    ASSERT_EQ(um.lost_dog_num + 1, dtor_run);
    PASS();
}

GREATEST_SUITE(struct_sptr) {
    RUN_TEST(shared_uninit);
    RUN_TEST(unique_uninited);
    RUN_TEST(unique_uninited_with_dtor);
    RUN_TEST(shared_inited_with_dtor);
    RUN_TEST(unique_inited_with_userdata_and_dtor);
    RUN_TEST(shared_inited_with_userdata_and_dtor);
    RUN_TEST(unique_inited_with_dtor);
}

