#pragma once
#include "common.h"

#define PAIR_GEN_CODE(NAME, T0, T1)                                                                                    \
    typedef struct NAME                                                                                                \
    {                                                                                                                  \
        union {                                                                                                        \
            T0 first;                                                                                                  \
            T0 f;                                                                                                      \
            T0 x;                                                                                                      \
            T0 a;                                                                                                      \
            T0 data;                                                                                                   \
        };                                                                                                             \
        union {                                                                                                        \
            T1 second;                                                                                                 \
            T1 s;                                                                                                      \
            T1 y;                                                                                                      \
            T1 b;                                                                                                      \
            T1 size;                                                                                                   \
        };                                                                                                             \
    } NAME;
