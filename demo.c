#include <stdio.h>

#define MY_LIBCSPTR_IMPLEMENTATION
#include "csptr.h"

typedef struct cat {
    const char* name;
    int age;
} cat;

typedef struct foo {
    char* name;
    char c;
    int i;
    cat* t;
} foo;

void foo_dtor(void* p, __attribute__((unused)) void* meta) {
    foo* f = (foo*)p;
    printf("foo_dtor -> %p\n", f);
}

typedef struct bar {
    float f;
    foo* ptr;
} bar;

void bar_dtor(void* p, __attribute__((unused)) void* meta) {
    bar* b = (bar*)p;
    printf("bar_dtor -> %p\n", b);
    if (b->ptr != NULL) {
        smart foo* f = b->ptr;
//        sfree(b->ptr);
        b->ptr = NULL;
    }
}

bar* fn1() {
    foo* f = shared_ptr(foo, {.name="foo",.c='A', .i=42}, foo_dtor);
    bar* b = shared_ptr( .dtor=bar_dtor, bar, {.f=3.14, .ptr=f});
    return b;
}

int* fn2(int len) {
    int *arr = smalloc(sizeof(int), len, SHARED);
    for (int i=0; i<len; ++i) arr[i] = i;
    return arr;
}

void fn( bar* b) {
    printf("b -> %p\n", b);
    printf("b->f: %f\n", b->f);
    printf("b->ptr->name: %s\n", b->ptr->name);
    printf("b->ptr->c: %c\n", b->ptr->c);
    printf("b->ptr->i: %d\n", b->ptr->i);
}

int main(void) {
    smart bar* b = fn1();
    smart int* ia = fn2(3);
    printf("ia: %d\n", ia[0]);
    fn(b);

    return 0;
}
