#include "greatest.h"

#define MY_LIBCSPTR_IMPLEMENTATION
#include "csptr.h"

#include "utils.h"

const struct meta m = {1, 2, 3};

SUITE_EXTERN(misc_suite);
SUITE_EXTERN(scalar_suite);
SUITE_EXTERN(shared_suite);
SUITE_EXTERN(array_suite);
SUITE_EXTERN(array2_suite);


GREATEST_MAIN_DEFS();

int main(int argc, char* argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(misc_suite);
    RUN_SUITE(scalar_suite);
    RUN_SUITE(shared_suite);
    RUN_SUITE(array_suite);
    RUN_SUITE(array2_suite);

    GREATEST_MAIN_END();
}

