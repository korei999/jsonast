#pragma once
#include "common.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define ARENA_1K (1024UL)
#define ARENA_4K (4UL * (size_t)ARENA_1K)
#define ARENA_1M ((size_t)ARENA_1K * 1024UL)
#define ARENA_1G ((size_t)ARENA_1M * 1024UL)

#define ARENA_FIRST(A) ((A)->pFirst)
#define ARENA_NEXT(AB) ((AB)->pNext)
#define ARENA_FOREACH(A, IT) for (typeof(ARENA_FIRST(A)) (IT) = ARENA_FIRST(A); (IT); (IT) = ARENA_NEXT(IT))
#define ARENA_FOREACH_SAFE(A, IT, TMP) for (typeof(ARENA_FIRST(A)) (IT) = ARENA_FIRST(A), (TMP) = nullptr; (IT) && ((TMP) = ARENA_NEXT(IT), true); (IT) = (TMP))

typedef struct ArenaBlock
{
    struct ArenaBlock* pNext;
    size_t size;
    uint8_t pData[]; /* flexible array member */
} ArenaBlock;

typedef struct Arena
{
    ArenaBlock* pFirst;
    ArenaBlock* pLast;
    void* pLastAllocatedBlock;
    size_t lastAllocationSize;
    const size_t cap;
} Arena;

static inline size_t
ArenaAlignedSize(size_t bytes)
{
    size_t newSize = bytes;
    double mulOf = (double)newSize / (double)sizeof(long);
    size_t cMulOfl = ceil(mulOf);
    newSize = sizeof(long) * cMulOfl;

    return newSize;
}

static inline ArenaBlock*
ArenaBlockNew(size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);
    ArenaBlock* pNew = malloc(sizeof(ArenaBlock) + alignedSize);
    pNew->size = 0;
    pNew->pNext = nullptr;

    return pNew;
}

static inline Arena
ArenaCreate(size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);
    Arena a = {.cap = alignedSize};
    ArenaBlock* pNew = ArenaBlockNew(alignedSize);
    a.pLast = a.pFirst = pNew;
    a.pLastAllocatedBlock = nullptr;

    return a;
}

static inline void
ArenaFree(Arena* a)
{
    ARENA_FOREACH_SAFE(a, it, tmp)
        free(it);
}

static inline void*
ArenaAlloc(Arena* a, size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);

    assert(alignedSize <= a->cap && "trying to allocate more than 1 arena block");

    void* pRBlock = nullptr;
    ArenaBlock* pLast = a->pLast;

    if (pLast->size + alignedSize > a->cap)
    {
        /* won't be null after reset */
        if (!pLast->pNext)
            pLast->pNext = ArenaBlockNew(a->cap);

        pLast = a->pLast = pLast->pNext;
    }

    pRBlock = &pLast->pData[pLast->size];
    a->pLastAllocatedBlock = pRBlock;
    a->lastAllocationSize = pLast->size;

    pLast->size += alignedSize;

    return pRBlock;
}

static inline void*
ArenaRealloc(Arena* a, void* pSrc, size_t nbytes, size_t newBytes)
{
    void* pDest = nullptr;
    size_t alignedSize = ArenaAlignedSize(newBytes);

    if (a->pLastAllocatedBlock == pSrc && (a->lastAllocationSize + alignedSize) < a->cap)
    {
        a->pLast->size = a->lastAllocationSize + alignedSize;
        pDest = pSrc;
    }
    else
    {
        pDest = ArenaAlloc(a, newBytes);
        memcpy(pDest, pSrc, nbytes);
    }

    return pDest;
}

static inline void*
ArenaCalloc(Arena* a, size_t nmemb, size_t size)
{
    void* pDest = ArenaAlloc(a, nmemb * size);
    size_t alignedSize = ArenaAlignedSize(nmemb * size);
    memset(pDest, 0, alignedSize);

    return pDest;
}

static inline void
ArenaReset(Arena* a)
{
    ARENA_FOREACH(a, it)
        it->size = 0;

    a->pLast = a->pFirst;
}
