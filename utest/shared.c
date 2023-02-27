#include "utils.h"

#include "../csptr.h"

typedef struct dog {
    const char* name;
    int age;
} dog;

TEST shared_uninit(void) {
    smart dog *ptr = shared_ptr(dog);
    CHECK_CALL(assert_valid_ptr(ptr));
    ASSERT_EQ(NULL, ptr->name);
    ASSERT_EQ(0, ptr->age);
    ASSERT_EQm("Expected pointer to have no usermeta", NULL, get_smart_ptr_usermeta(ptr));
    PASS();
}

static int dead_dog_count = 0;
void tombed_dog(void* ptr, UNUSED void* meta) {
    dog* d = (dog*)ptr;
    printf("RIP dog: name=%s, age=%d\n", d->name, d->age);
    ++dead_dog_count;
}

TEST shared_init_dog(void) {
    dead_dog_count = 0;
    dog *ptr;
    {
        const char *name = "Tom";
        smart dog *ptr1 = shared_ptr(dog, {.name=name}, tombed_dog);
        CHECK_CALL(assert_valid_ptr(ptr1));
        ASSERT_EQ(name, ptr1->name);
        ASSERT_EQ(0, ptr1->age);
        ptr = sref(ptr1);
    }
    ASSERT_EQm("expected dead dog is 0", 0, dead_dog_count);

    //*
    {
        const char *name = "tommason";
        smart dog *ptr2 = shared_ptr(dog, name,10, tombed_dog);
        CHECK_CALL(assert_valid_ptr(ptr2));
        ASSERT_EQ(name, ptr2->name);
        ASSERT_EQ_FMT(10, ptr2->age, "%d");

        ASSERT_EQm("expected dead dog is 0", 0, dead_dog_count);
        // sfree(ptr);
    }
    ASSERT_EQm("expected dead dog is 1", 1, dead_dog_count);

    sfree(ptr);
    ASSERT_EQm("expected dead dog is 2", 2, dead_dog_count);
   // */
    PASS();
}

TEST shared_sref(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });
    dog *ptr = shared_ptr(dog, { .name="Tom", .age=42 }, dtor);
    CHECK_CALL(assert_valid_ptr(ptr));

    {
        smart dog *ptr2 = sref(ptr);
        ASSERT_EQ("Tom", ptr2->name);
        ASSERT_EQ(42, ptr2->age);
        ASSERT_EQm("Expected reference to be the same pointer.", ptr, ptr2);
        ASSERT_EQm("Expected destructor NOT to have run.", 0, dtor_run);
    }
    ASSERT_EQm("Expected destructor NOT to have run.", 0, dtor_run);
    sfree(ptr);
    ASSERT_EQm("Expected destructor have run.", 1, dtor_run);

    PASS();
}

TEST unique_sref(void) {
    int dtor_run = 0;
    f_destructor dtor = lambda(void, (UNUSED void *ptr, UNUSED void *meta) { dtor_run = 1; });
    smart dog *ptr = unique_ptr(dog, { .name="Tom", .age=42 }, dtor);
    CHECK_CALL(assert_valid_ptr(ptr) );
    ASSERT_EQ("Tom", ptr->name);
    ASSERT_EQ(42, ptr->age);
    {
        dog *ptr2 = (ptr);
        ASSERT_EQ("Tom", ptr2->name);
        ASSERT_EQ(42, ptr2->age);
    }
//    sfree(ptr);
    ASSERT_EQ(0, dtor_run);
    PASS();
}
TEST shared_dog_with_usermeta(void) {
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
        f_destructor dtor = lambda(void, (void *ptr, void *meta) {
            dog* a = (dog*)ptr;
            struct owner* pm = (struct owner*)meta;
 //           struct meta* pm = (struct meta*)array_user_meta(meta);
            printf("\tdtor -> dog: name=%s, age=%d ; owner: %s, lost_dog_num=%d\n", a->name, a->age,  pm->master,pm->lost_dog_num);
            dtor_run++;
        });
        const char* name = "Tommason";
        smart dog *ptr1 = shared_ptr(dog, {.name=name,.age=11}, .dtor=dtor, .meta={&um, sizeof(um)});
        CHECK_CALL(assert_valid_ptr(ptr1));
        ASSERT_EQ(name, ptr1->name);
        ASSERT_EQ_FMT(11, ptr1->age, "%d");
        ptr = sref(ptr1);
        ASSERT_EQ(0, dtor_run);
    }
    ASSERT_EQ(0, dtor_run);
    sfree(ptr);
    ASSERT_EQ(1, dtor_run);
    PASS();
}

GREATEST_SUITE(shared_suite) {
    RUN_TEST(shared_uninit);
    RUN_TEST(shared_init_dog);
    RUN_TEST(shared_sref);
    RUN_TEST(shared_dog_with_usermeta);
    RUN_TEST(unique_sref);
}

