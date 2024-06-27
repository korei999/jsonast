#pragma once
#include "lex.h"
#include "adt/arena.h"

typedef struct JSONParser JSONParser;

struct JSONParser
{
    char* name;
    Lex l;
    Token tCurr;
    Token tNext;
    JSONNode* pHead;
    Arena* pArena;
};

JSONParser JSONParserCreate(Arena* a);
void JSONParserLoadJSON(JSONParser* self, char* path);
void JSONParserParse(JSONParser* self);
void JSONParserPrintJSON(JSONParser* self);
