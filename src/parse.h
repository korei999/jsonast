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

void JSONParserLoadJSON(JSONParser* self, Arena* a, char* path);
void JSONParserParse(JSONParser* self);
void JSONParserPrintJSON(JSONParser* self);
void JSONParserClean(JSONParser* self);
