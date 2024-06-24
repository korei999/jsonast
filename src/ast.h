#pragma once
#include "logs.h"

#define JSON_NEW_TAGVAL(TAG, VAL) ((JSONTagVal){.tag = TAG, .val.TAG = {VAL}})

typedef struct JSONTagVal JSONTagVal;
typedef struct JSONNode JSONNode;

enum JSONTag
{
    JSON_NULL,
    JSON_STRING,
    JSON_INT,
    JSON_FLOAT,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_BOOL
};

[[maybe_unused]] static const char* tagStrings[] = {
    [JSON_NULL]   = "JSON_NULL",
    [JSON_STRING] = "JSON_STRING",
    [JSON_INT]    = "JSON_INT",
    [JSON_FLOAT]  = "JSON_FLOAT",
    [JSON_ARRAY]  = "JSON_ARRAY",
    [JSON_OBJECT] = "JSON_OBJECT",
    [JSON_BOOL]   = "JSON_BOOL"
};

union JSONVal
{
    struct JSON_STRING { SliceStr sl;                                  } JSON_STRING;
    struct JSON_INT    { long i;                                       } JSON_INT;
    struct JSON_FLOAT  { double i;                                     } JSON_FLOAT;
    struct JSON_ARRAY  { JSONTagVal* aTagValues; size_t tagValueCount; } JSON_ARRAY;
    struct JSON_OBJECT { JSONNode* aNodes;       size_t nodeCount;     } JSON_OBJECT;
    struct JSON_BOOL   { bool b;                                       } JSON_BOOL;
    struct JSON_NULL   { void* empty;                                  } JSON_NULL;
};

struct JSONTagVal
{
    enum JSONTag tag;
    union JSONVal val;
};

struct JSONNode
{
    char* strKey;
    JSONTagVal tagVal;
};
