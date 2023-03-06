#include "utils.h"

#define MY_LIBCSPTR_IMPLEMENTATION
#include "../csptr.h"

const struct my_userdata g_metadata = {1, 2, 3};

SUITE_EXTERN(misc_suite);
SUITE_EXTERN(primitive_sptr);
SUITE_EXTERN(struct_sptr);
SUITE_EXTERN(primitive_static_array);
SUITE_EXTERN(struct_static_array);
SUITE_EXTERN(primitive_dynamic_array);

GREATEST_MAIN_DEFS();

int main(int argc, char* argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(misc_suite);
    RUN_SUITE(primitive_sptr);
    RUN_SUITE(struct_sptr);
    RUN_SUITE(primitive_static_array);
    RUN_SUITE(struct_static_array);
    RUN_SUITE(primitive_dynamic_array);

    GREATEST_MAIN_END();
}

