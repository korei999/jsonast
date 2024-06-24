#pragma once
#include "lex.h"

#define PARSER_ERR(CODE, ...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        CERR("parser error: ");                                                                                        \
        CERR(__VA_ARGS__);                                                                                             \
    } while (0)

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
void ParserClean(Parser* self);
Token ParserNext(Parser* self);
void ParserParseNode(Parser* self, JSONNode* pNode);
void ParserPrintNode(Parser* self, JSONNode* pNode, SliceStr slEnding);
void ParserPrintJSON(Parser* self);
