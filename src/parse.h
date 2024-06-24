#pragma once
#include "lex.h"

typedef struct Parser Parser;

struct Parser
{
    char* name;
    Lex l;
    Token tCurr;
    Token tNext;
    JSONNode* pHead;
};

void ParserLoadJson(Parser* self, char* path);
void ParserParse(Parser* self);
void ParserPrintJSON(Parser* self);
void ParserClean(Parser* self);
