#include "utils.h"

#include "csptr.h"

typedef struct dog {
    const char* name;
    int age;
} dog;

TEST shared_uninit(void) {
    smart dog *ptr = shared_ptr(dog);
    CHECK_CALL(assert_valid_ptr(ptr));
    ASSERT_EQ(NULL, ptr->name);
    ASSERT_EQ(0, ptr->age);
    PASS();
}

void tombed_dog(void* ptr, UNUSED void* meta) {
    dog* d = (dog*)ptr;
    printf("RIP dog: name=%s, age=%d\n", d->name, d->age);
}

TEST shared_init_dog(void) {
    smart dog *ptr;
    {
        smart dog *ptr1 = shared_ptr(dog, {.name="tom"}, tombed_dog);
        CHECK_CALL(assert_valid_ptr(ptr1));
        ASSERT_EQ("tom", ptr1->name);
        ASSERT_EQ(0, ptr1->age);
        ptr = sref(ptr1);
    }
    printf(" ----------------------- \n");

    //*
    {
        smart dog *ptr2 = shared_ptr(dog, "tommason",10, tombed_dog);
        CHECK_CALL(assert_valid_ptr(ptr2));
        ASSERT_EQ("tommason", ptr2->name);
        ASSERT_EQ_FMT(10, ptr2->age, "%d");

        //ptr = ptr2;
   // sfree(ptr);
    }
    printf(" ----------------------- \n");

    printf("DONE\n");
   // */
    PASS();
}

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
    RUN_TEST(shared_uninit);
    RUN_TEST(shared_init_dog);
    RUN_TEST(shared_init);
    RUN_TEST(shared_sref);
}

