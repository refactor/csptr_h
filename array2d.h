//
// Created by wulei on 23-3-10.
//

#ifndef CSPTR_H_ARRAY2D_H
#define CSPTR_H_ARRAY2D_H

#include "csptr.h"

#define smart_array2d(Kind, Type, C, R, ...) ({                             \
        struct  {                                                      \
            CSPTR_SENTINEL_DEC                                              \
            struct {                                                        \
                int32_t col;                                                \
                int32_t row;                                                \
            } tm;                                                           \
            const __typeof__(Type) *value;                                  \
        } args2d = {                                                        \
            CSPTR_SENTINEL                                                  \
            (C),                                                            \
            (R),                                                            \
            __VA_ARGS__                                                     \
        };                                                                  \
        smart_arr(Kind, Type,                                               \
                  args2d.tm.col * args2d.tm.row, .value=args2d.value,       \
                    .userdata={.ptr=&args2d.tm, .size=sizeof(args2d.tm)});  \
    })

#define array2d_get(ptr, c, r) ({                   \
    struct {                                        \
        int32_t col;                                \
        int32_t row;                                \
    }* m =  get_smart_ptr_userdata(ptr);            \
    assert(c >= 0 && c<m->col);                     \
    assert(r >= 0 && r<m->row);                     \
    ptr[r * m->col + c];                            \
})

#define array2d_set(ptr, c, r, v) ({                \
    struct {                                        \
        int32_t col;                                \
        int32_t row;                                \
    }* m =  get_smart_ptr_userdata(ptr);            \
    assert(c >= 0 && c<m->col);                     \
    assert(r >= 0 && r<m->row);                     \
    ptr[r * m->col + c] = v;                        \
})


#endif //CSPTR_H_ARRAY2D_H
