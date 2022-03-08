//#include "greatest.h"
#include "utils.h"

//#define SMALLOC_FIXED_ALLOCATOR
//#define MY_LIBCSPTR_IMPLEMENTATION
#include "csptr.h"


TEST zero_size(void) {
    void *ptr = smalloc(0, 0, UNIQUE);
    ASSERT_EQ(NULL, ptr);
    PASS();
}

#ifndef SMALLOC_FIXED_ALLOCATOR
TEST alloc_failure(void) {
    smalloc_allocator = (s_allocator) {
        lambda(void*, (UNUSED size_t s) { return NULL; }),
        lambda(void, (UNUSED void *ptr) {})
    };
    smart void *ptr = unique_ptr(int, 42);
    ASSERT_EQm("Expected NULL pointer to be returned.", NULL, ptr);
    smalloc_allocator = (s_allocator){malloc, free};
    PASS();
}
#endif

GREATEST_SUITE(misc_suite) {
    RUN_TEST(zero_size);
#ifndef SMALLOC_FIXED_ALLOCATOR
    RUN_TEST(alloc_failure);
#endif
}

/*
GREATEST_MAIN_DEFS();

int main(int argc, char* argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(misc_suite);


    GREATEST_MAIN_END();
}
// */