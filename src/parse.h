#pragma once
#include "lex.h"

typedef struct Parser Parser;

struct Parser
{
    Lex l;
    Token tCurr;
    Token tNext;
    JSONNode* pHead;
};

void ParserLoadJson(Parser* self, char* path);
void ParserClean(Parser* self);
void ParserNext(Parser* self);
void ParserParse(Parser* self, JSONNode* pNode);
