#pragma once
#include "common.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define ARENA_1K (1024UL)
#define ARENA_4K (ARENA_1K * 4UL)
#define ARENA_1M (ARENA_1K * 1024UL)
#define ARENA_05M (ARENA_1M / 2UL)
#define ARENA_1G (ARENA_1M * 1024UL)
#define ARENA_05G (ARENA_1G / 2UL)

#define ARENA_FIRST(A) ((A)->pBlocks)
#define ARENA_NEXT(AB) ((AB)->pNext)
#define ARENA_FOREACH(A, IT) for (typeof(ARENA_FIRST(A)) (IT) = ARENA_FIRST(A); (IT); (IT) = ARENA_NEXT(IT))
#define ARENA_FOREACH_SAFE(A, IT, TMP) for (typeof(ARENA_FIRST(A)) (IT) = ARENA_FIRST(A), (TMP) = nullptr; (IT) && ((TMP) = ARENA_NEXT(IT), true); (IT) = (TMP))

typedef struct ArenaBlock
{
    struct ArenaBlock* pNext;
    size_t size;
    void* pLastAllocation;
    size_t lastAllocationSize;
    uint8_t pData[]; /* flexible array member */
} ArenaBlock;

typedef struct Arena
{
    ArenaBlock* pBlocks;
    const size_t cap;
} Arena;

static inline size_t
ArenaAlignedSize(size_t bytes)
{
    double mulOf = (double)bytes / (double)sizeof(long);
    size_t mulAligned = ceil(mulOf);
    bytes = sizeof(size_t) * mulAligned;

    return bytes;
}

static inline ArenaBlock*
ArenaBlockNew(size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);
    ArenaBlock* pNew = malloc(sizeof(ArenaBlock) + alignedSize);
    pNew->size = 0;
    pNew->pNext = nullptr;
    pNew->pLastAllocation = nullptr;
    pNew->lastAllocationSize = 0;

    return pNew;
}

static inline Arena
ArenaCreate(size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);
    Arena a = {.cap = alignedSize};
    ArenaBlock* pNew = ArenaBlockNew(alignedSize);
    a.pBlocks = pNew;

    return a;
}

static inline void
ArenaFree(Arena* a)
{
    ARENA_FOREACH_SAFE(a, pBlock, tmp)
        free(pBlock);
}

static inline void*
ArenaAlloc(Arena* a, size_t bytes)
{
    size_t alignedSize = ArenaAlignedSize(bytes);

    assert(alignedSize < a->cap && "trying to allocate more than 1 arena block");

    void* pRetBlock = nullptr;
    ArenaBlock* pLast = nullptr;

    ArenaBlock* pBlock = a->pBlocks;
    ArenaBlock* pPrevBlock = pBlock;

    while (pBlock)
    {
        if (pBlock->size + alignedSize < a->cap)
        {
            pLast = pBlock;
            break;
        }

        pPrevBlock = pBlock;
        pBlock = pBlock->pNext;
    }

    if (!pBlock)
    {
        pPrevBlock->pNext = ArenaBlockNew(a->cap);
        pLast = pPrevBlock->pNext;
    }

    pRetBlock = &pLast->pData[pLast->size];
    pLast->pLastAllocation = pRetBlock;
    pLast->lastAllocationSize = pLast->size;
    pLast->size += alignedSize;

    return pRetBlock;
}

static inline void*
ArenaRealloc(Arena* a, void* pSrc, size_t nbytes, size_t newBytes)
{
    void* pRetBlock = nullptr;
    size_t alignedSize = ArenaAlignedSize(newBytes);

    ArenaBlock* pBlock = a->pBlocks;

    while (pBlock)
    {
        /* just bump if there is nothing in front */
        if (pBlock->pLastAllocation == pSrc && (pBlock->lastAllocationSize + alignedSize) < a->cap)
        {
            pBlock->size = pBlock->lastAllocationSize + alignedSize;
            pRetBlock = pSrc;
            break;
        }

        pBlock = pBlock->pNext;
    }

    if (!pBlock)
    {
        pRetBlock = ArenaAlloc(a, newBytes);
        memcpy(pRetBlock, pSrc, nbytes);
    }

    return pRetBlock;
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
    ARENA_FOREACH(a, pBlock)
        pBlock->size = 0;
}
