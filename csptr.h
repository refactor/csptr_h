//
// Created by wulei on 2022/2/25.
//

#ifndef MY_LIBCSPTR_H
#define MY_LIBCSPTR_H

#include <assert.h>
# include <string.h>
# include <stdlib.h>
#include <stdint.h>
//#include <stdarg.h>

# ifdef __GNUC__
#  define CSPTR_INLINE      __attribute__ ((always_inline)) inline
#  define CSPTR_MALLOC_API  __attribute__ ((malloc))
#  define CSPTR_PURE        __attribute__ ((pure))
# elif defined(_MSC_VER)
#  define CSPTR_INLINE      __forceinline
#  define CSPTR_MALLOC_API
#  define CSPTR_PURE
# else
#  define CSPTR_INLINE
#  define CSPTR_MALLOC_API
#  define CSPTR_PURE
# endif

# ifdef CSPTR_NO_SENTINEL
#  ifndef __GNUC__
#   error Variadic structure sentinels can only be disabled on a compiler supporting GNU extensions
#  endif
#  define CSPTR_SENTINEL
#  define CSPTR_SENTINEL_DEC
# else
#  define CSPTR_SENTINEL        .sentinel_ = 0,
#  define CSPTR_SENTINEL_DEC int sentinel_;
# endif

enum pointer_kind {
    UNIQUE,
    SHARED,

    ARRAY = 1 << 8
};

typedef void (*f_destructor)(void *, void *);

typedef struct {
    void *(*alloc)(size_t);
    void (*dealloc)(void *);
} s_allocator;

extern s_allocator smalloc_allocator;

typedef struct {
    CSPTR_SENTINEL_DEC
    size_t item_size;
    size_t item_num;
    enum pointer_kind kind;
    f_destructor dtor;
    struct {
        const void *data;
        size_t size;
    } userdata;
} s_smalloc_args;

CSPTR_PURE void *get_smart_ptr_userdata(const void * const ptr);
void *sref(void *ptr);
CSPTR_MALLOC_API void *smalloc_(s_smalloc_args *args);
void sfree(void *ptr);
void *smove_size(void *ptr, size_t size);

#  define smalloc(...) \
    smalloc_(&(s_smalloc_args) {CSPTR_SENTINEL __VA_ARGS__ })

#  define smove(Ptr) \
    smove_size((Ptr), sizeof (*(Ptr)))

CSPTR_INLINE void sfree_stack(void *ptr) {
    union {
        void **real_ptr;
        void *ptr;
    } conv;
    conv.ptr = ptr;
    sfree(*conv.real_ptr);
    *conv.real_ptr = NULL;
}

# define ARGS_ args.dtor, { args.userdata.ptr, args.userdata.size }

# define autoclean __attribute__ ((cleanup(sfree_stack)))
# define smart __attribute__ ((cleanup(sfree_stack)))
# define smart_ptr(Kind, Type, ...)                                         \
    ({                                                                      \
        struct s_tmp {                                                      \
            CSPTR_SENTINEL_DEC                                              \
            __typeof__(Type) value;                                         \
            f_destructor dtor;                                              \
            struct {                                                        \
                const void *ptr;                                            \
                size_t size;                                                \
            } userdata;                                                     \
        } args = {                                                          \
            CSPTR_SENTINEL                                                  \
            __VA_ARGS__                                                     \
        };                                                                  \
        const __typeof__(Type[1]) dummy;                                    \
        void *var = sizeof (dummy[0]) == sizeof (dummy)                     \
            ? smalloc(sizeof (Type), 1, Kind, ARGS_)                        \
            : smalloc(sizeof (dummy[0]),                                    \
                    sizeof (dummy) / sizeof (dummy[0]), Kind, ARGS_);       \
        if (var != NULL)                                                    \
            memcpy(var, &args.value, sizeof (Type));                        \
        (__typeof__(Type)*) var;                                            \
    })

# define smart_arr(Kind, Type, Length, ...)                                 \
    ({                                                                      \
        enum pointer_kind akind = Kind | ARRAY;                             \
        struct s_tmp {                                                      \
            CSPTR_SENTINEL_DEC                                              \
            __typeof__(Type) *value;                                        \
            f_destructor dtor;                                              \
            struct {                                                        \
                const void *ptr;                                            \
                size_t size;                                                \
            } userdata;                                                     \
        } args = {                                                          \
            CSPTR_SENTINEL                                                  \
            __VA_ARGS__                                                     \
        };                                                                  \
        void *var = smalloc(sizeof (Type), Length, akind, ARGS_);           \
        if (var != NULL) {                                                  \
            if (args.value != NULL) {                                       \
                memcpy(var, args.value, sizeof (Type) * Length);            \
            }                                                               \
            else {                                                          \
                memset(var, 0, sizeof(Type) * Length);                      \
            }                                                               \
        }                                                                   \
        var;                                                                \
    })

# define shared_ptr(Type, ...) smart_ptr(SHARED, Type, __VA_ARGS__)
# define unique_ptr(Type, ...) smart_ptr(UNIQUE, Type, __VA_ARGS__)

# define shared_arr(Type, Length, ...) smart_arr(SHARED, Type, Length, __VA_ARGS__)
# define unique_arr(Type, Length, ...) smart_arr(UNIQUE, Type, Length, __VA_ARGS__)



typedef struct {
    size_t itemnum;
    size_t itemsize;
} s_meta_array;

CSPTR_PURE size_t array_length(const void *ptr);

CSPTR_PURE size_t array_item_size(const void *ptr);
CSPTR_PURE size_t array_size(const void *ptr);

CSPTR_PURE void *array_userdata(void *ptr);


#endif //MY_LIBCSPTR_H

#ifdef MY_LIBCSPTR_IMPLEMENTATION


CSPTR_PURE
static s_meta_array *get_smart_ptr_meta_array_(const void * const ptr) {
    return get_smart_ptr_userdata(ptr); // TODO: s_meta_array should not in userdata area
}

CSPTR_PURE size_t array_length(const void *ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(ptr);
    return meta ? meta->itemnum : 1;  // TODO: instead of 1, it should be real array length
}

CSPTR_PURE size_t array_item_size(const void *ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(ptr);
    return meta ? meta->itemsize : 0;
}

CSPTR_PURE size_t array_size(const void *ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(ptr);
    return meta ? meta->itemsize * meta->itemnum : 0;
}

CSPTR_PURE CSPTR_INLINE void *array_userdata(void *ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(ptr);
    return meta ? meta + 1 : NULL;
}


typedef struct {
    enum pointer_kind kind;
    f_destructor dtor;
#ifndef NDEBUG
    void *ptr;
#endif /* !NDEBUG */
} s_meta;

typedef struct {
    enum pointer_kind kind;
    f_destructor dtor;
#ifndef NDEBUG
    void *ptr;
#endif /* !NDEBUG */
    volatile size_t ref_count;
} s_meta_shared;

static CSPTR_INLINE size_t align(size_t s) {
    return (s + (sizeof (char *) - 1)) & ~(sizeof (char *) - 1);
}

static CSPTR_PURE CSPTR_INLINE s_meta *get_meta_(const void * const ptr) {
    size_t *size = (size_t *) ptr - 1;
    return (s_meta *) ((char *) size - *size);
}

s_allocator smalloc_allocator = {malloc, free};

#ifdef _MSC_VER
# include <windows.h>
# include <malloc.h>
#endif

#ifndef _MSC_VER
static CSPTR_INLINE size_t atomic_add(volatile size_t *count, const size_t limit, const size_t val) {
    size_t old_count, new_count;
    do {
      old_count = *count;
      if (old_count == limit)
          abort();
      new_count = old_count + val;
    } while (!__sync_bool_compare_and_swap(count, old_count, new_count));
    return new_count;
}
#endif

static CSPTR_INLINE size_t atomic_increment(volatile size_t *count) {
#ifdef _MSC_VER
    return InterlockedIncrement64(count);
#else
    return atomic_add(count, SIZE_MAX, 1);
#endif
}

static CSPTR_INLINE size_t atomic_decrement(volatile size_t *count) {
#ifdef _MSC_VER
    return InterlockedDecrement64(count);
#else
    return atomic_add(count, 0, -1);
#endif
}

CSPTR_INLINE void *get_smart_ptr_userdata(const void * const ptr) {
    assert((size_t) ptr == align((size_t) ptr));

    s_meta * const meta = get_meta_(ptr);
    assert(meta->ptr == ptr);

    size_t head_size = meta->kind & SHARED ? sizeof (s_meta_shared) : sizeof (s_meta);
    size_t *metasize = (size_t *) ptr - 1;
    if (*metasize == head_size)
        return NULL;

    return (char *) meta + head_size;
}

void *sref(void *ptr) {
    s_meta *meta = get_meta_(ptr);
    assert(meta->ptr == ptr);
    assert(meta->kind & SHARED);
    atomic_increment(&((s_meta_shared *) meta)->ref_count);
    return ptr;
}

void *smove_size(void *ptr, size_t size) {
    s_meta *meta = get_meta_(ptr);
    assert(meta->kind & UNIQUE);

    s_smalloc_args args;

    size_t *metasize = (size_t *) ptr - 1;
    if (meta->kind & ARRAY) {
        s_meta_array *arr_meta = get_smart_ptr_meta_array_(ptr);
        args = (s_smalloc_args) {
            .item_size = arr_meta->itemsize,
            .item_num = arr_meta->itemnum,
            .kind = (enum pointer_kind) (SHARED | ARRAY),
            .dtor = meta->dtor,
            .userdata = { arr_meta, *metasize },
        };
    } else {
        void *userdata = get_smart_ptr_userdata(ptr);
        args = (s_smalloc_args) {
            .item_size = size,
            .kind = SHARED,
            .dtor = meta->dtor,
            .userdata = {userdata, *metasize },
        };
    }

    void *newptr = smalloc_(&args);
    memcpy(newptr, ptr, size);
    return newptr;
}

CSPTR_MALLOC_API
CSPTR_INLINE static void *alloc_entry(size_t head, size_t size, size_t metasize) {
    const size_t totalsize = head + size + metasize + sizeof (size_t);
#ifdef SMALLOC_FIXED_ALLOCATOR
    return malloc(totalsize);
#else /* !SMALLOC_FIXED_ALLOCATOR */
    return smalloc_allocator.alloc(totalsize);
#endif /* !SMALLOC_FIXED_ALLOCATOR */
}

CSPTR_INLINE static void dealloc_entry(s_meta *meta, void *ptr) {
    if (meta->dtor) {
        void *userdata = get_smart_ptr_userdata(ptr);
        if (meta->kind & ARRAY) {
            s_meta_array *arr_meta = userdata;//(void *) (meta + 1);
            for (size_t i = 0; i < arr_meta->itemnum; ++i)
                meta->dtor((char *) ptr + arr_meta->itemsize * i, arr_meta + 1);
        }
        else
            meta->dtor(ptr, userdata);
    }

#ifdef SMALLOC_FIXED_ALLOCATOR
    free(meta);
#else /* !SMALLOC_FIXED_ALLOCATOR */
    smalloc_allocator.dealloc(meta);
#endif /* !SMALLOC_FIXED_ALLOCATOR */
}

CSPTR_MALLOC_API
static void *smalloc_impl_(const s_smalloc_args *args) {
    if (!(args->item_size && args->item_num))
        return NULL;

    // align the sizes to the item_size of a word
    size_t aligned_metasize = align(args->userdata.size);
    size_t rowdata_size = align(args->item_size * args->item_num);

    size_t head_size = args->kind & SHARED ? sizeof (s_meta_shared) : sizeof (s_meta);
    void *ptr = alloc_entry(head_size, rowdata_size, aligned_metasize);
    if (ptr == NULL)
        return NULL;

    char *shifted = (char *) ptr + head_size;
    if (args->userdata.size && args->userdata.data)
        memcpy(shifted, args->userdata.data, args->userdata.size);

    size_t *sz = (size_t *) (shifted + aligned_metasize);
    *sz = head_size + aligned_metasize;

    *(s_meta*) ptr = (s_meta) {
        .kind = args->kind,
        .dtor = args->dtor,
#ifndef NDEBUG
        .ptr = sz + 1
#endif
    };

    if (args->kind & SHARED)
        ((s_meta_shared*) ptr)->ref_count = 1;

    return sz + 1;
}

CSPTR_MALLOC_API
static void *smalloc_array_(const s_smalloc_args *args) {
    const size_t aligned_metasize = align(args->userdata.size + sizeof(s_meta_array));
#ifdef _MSC_VER
    char *new_meta = _alloca(aligned_metasize);
#else
    char new_meta[aligned_metasize];
#endif
    s_meta_array *arr_meta = (void *) new_meta;
    *arr_meta = (s_meta_array) {
        .itemsize = args->item_size,
        .itemnum = args->item_num,
    };
    if (args->userdata.size && args->userdata.data)
        memcpy(arr_meta + 1, args->userdata.data, args->userdata.size);

    return smalloc_impl_(&(s_smalloc_args) {
            .item_size = args->item_size,
            .item_num = args->item_num,
            .kind = (enum pointer_kind) (args->kind | ARRAY),
            .dtor = args->dtor,
            .userdata = {&new_meta, aligned_metasize},
    });
}

CSPTR_MALLOC_API
void *smalloc_(s_smalloc_args *args) {
//    return (args->item_num == 0 ? smalloc_impl_ : smalloc_array_)(args);
    return (args->kind & ARRAY ? smalloc_array_ : smalloc_impl_)(args);
}

void sfree(void *ptr) {
    if (!ptr) return;

    assert((size_t) ptr == align((size_t) ptr));
    s_meta *meta = get_meta_(ptr);
    assert(meta->ptr == ptr);

    if (meta->kind & SHARED && atomic_decrement(&((s_meta_shared *) meta)->ref_count))
        return;

    dealloc_entry(meta, ptr);
}

#endif
