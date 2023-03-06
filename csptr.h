/* csptr.h - v0.1.0 - public domain structures for autoclean/smart pointer -
    Created by wulei on 2022/2/25.
    Inspired by https://snai.pe/posts/c-smart-pointers
*/
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
    UNIQUE = 1,
    SHARED = 2,

    STATIC_ARRAY = 4,
    DYNAMIC_ARRAY = 8
};

typedef void (*f_destructor)(void *, void *);

typedef struct {
    void *(*alloc)(size_t);
    void (*dealloc)(void *);
    void *(*realloc)(void*, size_t);
} s_allocator;

extern s_allocator smalloc_allocator;

typedef struct {
    CSPTR_SENTINEL_DEC
    size_t item_size;
    size_t item_cap;
    size_t item_num;
    enum pointer_kind kind;
    f_destructor dtor;
    struct {
        const void *data;
        size_t size;
    } userdata;
} s_smalloc_args;

CSPTR_PURE void *get_smart_ptr_userdata(const void * const smart_ptr);
void *sref(void *ptr);
CSPTR_MALLOC_API void *smalloc_(s_smalloc_args *args);
void sfree(void *smart_ptr);
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
            ? smalloc(sizeof (Type), 1, 1, Kind, ARGS_)                        \
            : smalloc(sizeof (dummy[0]),                                    \
                    sizeof (dummy) / sizeof (dummy[0]), 1, Kind, ARGS_);       \
        if (var != NULL)                                                    \
            memcpy(var, &args.value, sizeof (Type));                        \
        (__typeof__(Type)*) var;                                            \
    })

# define smart_arr(Kind, Type, Length, ...)                                 \
    ({                                                                      \
        enum pointer_kind akind = Kind | STATIC_ARRAY;                      \
        struct s_tmp {                                                      \
            CSPTR_SENTINEL_DEC                                              \
            const __typeof__(Type) *value;                                        \
            f_destructor dtor;                                              \
            struct {                                                        \
                const void *ptr;                                            \
                size_t size;                                                \
            } userdata;                                                     \
        } args = {                                                          \
            CSPTR_SENTINEL                                                  \
            __VA_ARGS__                                                     \
        };                                                                  \
        size_t Cap = Length;                                                \
        size_t Len = (args.value == NULL) ? 0 : Length;                     \
        void *var = smalloc(sizeof (Type), Cap,Len, akind, ARGS_);          \
        if (var != NULL) {                                                  \
            if (args.value != NULL) {                                       \
                memcpy(var, args.value, sizeof (Type) * Length);            \
            }                                                               \
            else {                                                          \
                memset(var, 0, sizeof(Type) * Length);                      \
            }                                                               \
        }                                                                   \
        (__typeof__(Type)*) var;                                            \
    })

# define shared_ptr(Type, ...) smart_ptr(SHARED, Type, __VA_ARGS__)
# define unique_ptr(Type, ...) smart_ptr(UNIQUE, Type, __VA_ARGS__)

# define shared_arr(Type, Length, ...) smart_arr(SHARED, Type, Length, __VA_ARGS__)
# define unique_arr(Type, Length, ...) smart_arr(UNIQUE, Type, Length, __VA_ARGS__)

#define arrput smt__arrappend
#define arrlenu(a) (!(a) ? 0 : static_array.length(a))

#define arrappend smt__arrappend
#define arrpop smt__arrpop
#define arrdel smt__arrdel
#define arrdeln smt__arrdeln
#define arrins smt__arrins
#define arrlast     smt__arrlast

#define smt__arrappend(a,v) (smt__arrmaybegrow(a,1), (a)[get_smart_ptr_meta_array_(a)->item_num++] = (v))
#define smt__arrpop(a) ((a)[--get_smart_ptr_meta_array_(a)->item_num])
#define smt__arrdel(a,i) smt__arrdeln(a,i,1)
#define smt__arrdeln(a,i,n)   (memmove(&(a)[i], &(a)[(i)+(n)], get_smart_ptr_meta_array_(a)->item_size * (get_smart_ptr_meta_array_(a)->item_num-(n)-(i))), get_smart_ptr_meta_array_(a)->item_num -= (n))
#define smt__arrins(a,i,v)    (smt__arrinsn((a),(i),1), (a)[i]=(v))

#define smt__arrmaybegrow(a,n)  ((!(a) || get_smart_ptr_meta_array_(a)->item_num + (n) > get_smart_ptr_meta_array_(a)->item_capacity) \
                                  ? (smt__arrgrow(a,n,0),0) : 0)
#define smt__arrgrow(a,b,c)   ((a) = smt__arrgrowf_((a), (b), (c)))

#define smt__arrinsn(a,i,n)   (smt__arraddnindex((a),(n)), memmove(&(a)[(i)+(n)], &(a)[i], get_smart_ptr_meta_array_(a)->item_size * (get_smart_ptr_meta_array_(a)->item_num-(n)-(i))))
#define smt__arraddnindex(a,n) (smt__arrmaybegrow(a,n), (n) ? (get_smart_ptr_meta_array_(a)->item_num += (n), get_smart_ptr_meta_array_(a)->item_num-(n)) : get_smart_ptr_meta_array_(a)->item_num)
#define smt__arrlast(a)       ((a)[get_smart_ptr_meta_array_(a)->item_num-1])

struct smt_static_array_ns {
    size_t (*capacity)(const void* smart_arr);
    size_t (*length)(const void* smart_arr);
    size_t (*item_size)(const void* smart_arr);
    void* (*userdata)(const void* const smart_arr);
};

extern const struct smt_static_array_ns static_array;
extern const struct smt_dynamic_array_ns dynamic_array;

typedef struct {
    size_t item_num;
    size_t item_size;
    size_t item_capacity;
} s_meta_array;

extern s_meta_array *get_smart_ptr_meta_array_(const void * const smart_ptr);
extern void * smt__arrgrowf_(void *a, size_t addlen, size_t min_cap);
#endif //MY_LIBCSPTR_H

#ifdef MY_LIBCSPTR_IMPLEMENTATION
#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#endif

typedef struct {
    struct s_meta_header_s {
        enum pointer_kind kind;
        f_destructor dtor;
#ifndef NDEBUG
        void *ptr;
#endif /* !NDEBUG */
    } header;
#ifndef __STDC_NO_ATOMICS__
    volatile atomic_int ref_count;
#else
    volatile int32_t ref_count;
#endif
} s_meta_shared;

typedef struct s_meta_header_s s_meta_header;

static CSPTR_PURE CSPTR_INLINE s_meta_header *get_smart_ptr_meta_(const void * const smart_ptr) {
    size_t *sz_ptr = (size_t *) smart_ptr - 1;
    return (s_meta_header *) ((char *) sz_ptr - *sz_ptr);
}

CSPTR_PURE
s_meta_array *get_smart_ptr_meta_array_(const void * const smart_ptr);

CSPTR_PURE
static size_t array_length_(const void *smart_ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(smart_ptr);
    return meta ? meta->item_num : 1;  // TODO: instead of 1, it should be real array length
}

CSPTR_PURE
static size_t array_item_size_(const void *smart_ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(smart_ptr);
    return meta ? meta->item_size : 0;
}

CSPTR_PURE
static size_t array_capacity_(const void *smart_ptr) {
    s_meta_array *meta = get_smart_ptr_meta_array_(smart_ptr);
    return meta ? meta->item_capacity : 0;
}

const struct smt_static_array_ns static_array = {
        .length = array_length_,
        .item_size = array_item_size_,
        .capacity = array_capacity_,
        .userdata = get_smart_ptr_userdata
};

static size_t get_smart_ptr_total_meta_sz_(const void* smart_ptr) {
    return (char*)smart_ptr - ((char*)get_smart_ptr_meta_(smart_ptr));
}

void *smt__arrgrowf_(void *a, size_t addlen, size_t min_cap) {
    (void )min_cap;
    size_t elemsize = array_item_size_(a);
    size_t min_len = array_length_(a) + addlen;
    // compute the minimum capacity needed
    if (min_len > min_cap)
        min_cap = min_len;
    if (min_cap <= array_capacity_(a))
        return a;

    // increase needed capacity to guarantee O(1) amortized
    if (min_cap < 2 * array_capacity_(a))
        min_cap = 2 * array_capacity_(a);
    else if (min_cap < 4)
        min_cap = 4;

    s_meta_header* raw_a = get_smart_ptr_meta_(a);
    size_t total_head_meta_userdata_sz = get_smart_ptr_total_meta_sz_(a);
    // TODO: align memory check
    void* raw_b = smalloc_allocator.realloc(raw_a, elemsize * min_cap + total_head_meta_userdata_sz);
    void* b = (char*)raw_b + total_head_meta_userdata_sz;
#ifndef NDEBUG
    get_smart_ptr_meta_(b)->ptr = b;
#endif
    get_smart_ptr_meta_array_(b)->item_capacity = min_cap;
    return b;
}


static CSPTR_INLINE size_t align(size_t s) {
    return (s + (sizeof (char *) - 1)) & ~(sizeof (char *) - 1);
}

s_allocator smalloc_allocator = {malloc, free, realloc};

#ifdef __STDC_NO_ATOMICS__
#ifdef _MSC_VER
# include <windows.h>
# include <malloc.h>
#endif

#ifndef _MSC_VER
static CSPTR_INLINE int32_t atomic_add(volatile int32_t *count, const int32_t limit, const int32_t val) {
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

static CSPTR_INLINE int32_t atomic_increment(volatile int32_t *count) {
#ifdef _MSC_VER
    return InterlockedIncrement64(count);
#else
    return atomic_add(count, INT32_MAX, 1);
#endif
}

static CSPTR_INLINE int32_t atomic_decrement(volatile int32_t *count) {
#ifdef _MSC_VER
    return InterlockedDecrement64(count);
#else
    return atomic_add(count, 0, -1);
#endif
}

#else

static CSPTR_INLINE int32_t atomic_increment(volatile atomic_int *count) {
    return atomic_fetch_add(count, 1) + 1;
}

static CSPTR_INLINE int32_t atomic_decrement(volatile atomic_int *count) {
    return atomic_fetch_sub(count, 1) - 1;
}

#endif

void *sref(void *ptr) {
    s_meta_header *meta = get_smart_ptr_meta_(ptr);
    assert(meta->ptr == ptr);
    assert(meta->kind & SHARED);
    atomic_increment(&((s_meta_shared *) meta)->ref_count);
    return ptr;
}

void *smove_size(void *ptr, size_t size) {
    s_meta_header *meta = get_smart_ptr_meta_(ptr);
    assert(meta->kind & UNIQUE);

    s_smalloc_args args;

    size_t *metasize = (size_t *) ptr - 1;
    if (meta->kind & STATIC_ARRAY) {
        s_meta_array *arr_meta = get_smart_ptr_meta_array_(ptr);
        args = (s_smalloc_args) {
            .item_size = arr_meta->item_size,
            .item_cap = arr_meta->item_num,
            .kind = (enum pointer_kind) (SHARED | STATIC_ARRAY),
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

CSPTR_INLINE static void dealloc_entry(s_meta_header *meta, void *ptr) {
    if (meta->dtor) {
        void * const userdata = get_smart_ptr_userdata(ptr);
        if (meta->kind & STATIC_ARRAY) {
            s_meta_array *arr_meta = get_smart_ptr_meta_array_(ptr);//(void *) (meta + 1);
            for (size_t i = 0; i < arr_meta->item_num; ++i)
                meta->dtor((char *) ptr + arr_meta->item_size * i, userdata);
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

static inline size_t get_meta_header_size_(enum pointer_kind kind) {
    return kind & SHARED ? sizeof (s_meta_shared) : sizeof (s_meta_header);
}
static inline size_t get_meta_array_size_(enum pointer_kind kind) {
//    return kind & STATIC_ARRAY ? sizeof(s_meta_array) : 0;
    switch (kind & (STATIC_ARRAY | DYNAMIC_ARRAY)) {
        case STATIC_ARRAY:
            return sizeof(s_meta_array);
        case DYNAMIC_ARRAY:
//            return sizeof(s_meta_dynamic_array);
        default:
            return 0;
    }
}
static inline size_t get_meta_size_(enum pointer_kind kind) {
    return get_meta_header_size_(kind) + get_meta_array_size_(kind);
}
static inline s_meta_array* get_ptr_meta_array_(const void* raw_ptr, enum pointer_kind kind) {
    return (s_meta_array*) ((char*)raw_ptr + get_meta_header_size_(kind));
}
static inline char* get_ptr_userdata_(const void* raw_ptr, enum pointer_kind kind) {
    return (char *) raw_ptr + get_meta_size_(kind);
}
CSPTR_MALLOC_API
static void *smalloc_impl_(const s_smalloc_args *args) {
    if (!(args->item_size && args->item_cap))
        return NULL;

    // align the sizes to the item_size of a word
    size_t aligned_userdata_size = align(args->userdata.size);
    size_t rowdata_size = align(args->item_size * args->item_cap);

    const size_t total_meta_size = get_meta_size_(args->kind);

    void *raw_ptr = alloc_entry(total_meta_size, rowdata_size, aligned_userdata_size);
    if (raw_ptr == NULL)
        return NULL;

    char * const userdata_ptr = get_ptr_userdata_(raw_ptr, args->kind);
    if (args->userdata.size && args->userdata.data)
        memcpy(userdata_ptr, args->userdata.data, args->userdata.size);

    if (args->kind & STATIC_ARRAY) {
        s_meta_array *meta_array = get_ptr_meta_array_(raw_ptr, args->kind);
        *meta_array = (s_meta_array) {
                .item_capacity = args->item_cap,
                .item_num = args->item_num,
                .item_size = args->item_size
        };
    }

    size_t * const sz_ptr = (size_t *) (userdata_ptr + aligned_userdata_size);
    *sz_ptr = total_meta_size + aligned_userdata_size;

    *(s_meta_header*) raw_ptr = (s_meta_header) {
        .kind = args->kind,
        .dtor = args->dtor,
#ifndef NDEBUG
        .ptr = sz_ptr + 1
#endif
    };

    if (args->kind & SHARED) {
#ifndef __STDC_NO_ATOMICS__
        atomic_init(&((s_meta_shared *) raw_ptr)->ref_count, 1);
#else
        ((s_meta_shared*) raw_ptr)->ref_count = 1;
#endif
    }
    return sz_ptr + 1;
}

s_meta_array *get_smart_ptr_meta_array_(const void * const smart_ptr) {
    assert((size_t) smart_ptr == align((size_t) smart_ptr));

    s_meta_header * const raw_ptr = get_smart_ptr_meta_(smart_ptr);
    assert(raw_ptr->ptr == smart_ptr);
    if (!(raw_ptr->kind & STATIC_ARRAY) ){
        return NULL;
    }
    size_t header_size = get_meta_header_size_(raw_ptr->kind);
    s_meta_array* ma = (s_meta_array *)(((char*)raw_ptr) + header_size);
    return ma;
}

CSPTR_PURE
void *get_smart_ptr_userdata(const void * const smart_ptr) {
    assert((size_t) smart_ptr == align((size_t) smart_ptr));

    s_meta_header * const raw_ptr = get_smart_ptr_meta_(smart_ptr);
    assert(raw_ptr->ptr == smart_ptr);

    size_t header_size = get_meta_size_(raw_ptr->kind);
    size_t *total_meta_size = (size_t *) smart_ptr - 1;
    if (*total_meta_size == header_size)
        return NULL;

    return (char *) raw_ptr + header_size;
}

CSPTR_MALLOC_API
void *smalloc_(s_smalloc_args *args) {
    return smalloc_impl_(args);
}

void sfree(void *smart_ptr) {
    if (!smart_ptr) return;

    assert((size_t) smart_ptr == align((size_t) smart_ptr));
    s_meta_header *meta = get_smart_ptr_meta_(smart_ptr);
    assert(meta->ptr == smart_ptr);

    if (meta->kind & SHARED && atomic_decrement(&((s_meta_shared *) meta)->ref_count))
        return;

    dealloc_entry(meta, smart_ptr);
}

#endif
