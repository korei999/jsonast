#include "parse.h"
#include "adt/array.h"

void reallocNode(JSONNode* pNode, size_t size);

static void
ParserExpect(Parser* self, enum JSONToken t)
{
    if (self->tCurr.type != t)
        PARSER_ERR(2, "next expected: '{}', got '{}' instead\n", tokenStrings[t], tokenStrings[self->tCurr.type]);
}

static void
ParserExpectNext(Parser* self, enum JSONToken t)
{
    if (self->tNext.type != t)
        PARSER_ERR(2, "curr expected: '{}', got '{}' instead\n", tokenStrings[t], tokenStrings[self->tNext.type]);
}

void
ParserLoadJson(Parser* self, char* path)
{
    LexLoadFile(&self->l, path);

    if (self->l.slData.size <= 1)
    {
        CERR("{}: is empty\n", path);
        exit(1);
    }

    self->pHead = (JSONNode*)calloc(1, sizeof(JSONNode));
    self->name = path;

    self->tCurr = LexNext(&self->l);
    self->tNext = LexNext(&self->l);

    if (self->tCurr.type == TOK_LBRACE)
    {
        //
    }
    else if (self->tCurr.type == TOK_LBRACKET)
    {
        //
    }
    else
    {
        CERR("error: first token should be '{' or '['\n");
        exit(1);
    }
}

static void
ParserCleanNode(Parser* self, JSONNode* pNode)
{
    switch (pNode->tagVal.tag)
    {
        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                for (size_t i = 0; i < obj.nodeCount; i++)
                    ParserCleanNode(self, &obj.aNodes[i]);
                free(obj.aNodes);
            }
            break;
    }
}

void
ParserClean(Parser* self)
{
    ParserCleanNode(self, self->pHead);
    free(self->pHead);
    LexClean(&self->l);

}

Token
ParserNext(Parser* self)
{
    self->tCurr = self->tNext;
    self->tNext = LexNext(&self->l);

    /*COUT("tCurr: ({}) | '{}'\n", tokenStrings[self->tCurr.type], self->tCurr.slLiteral);*/
    /*COUT("tNext: ({}) | '{}'\n", tokenStrings[self->tNext.type], self->tNext.slLiteral);*/

    return self->tCurr;
}

size_t
growNode(JSONNode* pNode, size_t cap)
{
    size_t newCap = cap;

    switch (pNode->tagVal.tag)
    {
        default:
            PARSER_ERR(2, "pushNode\n");
            break;

        case JSON_OBJECT:
            {
                if (pNode->tagVal.val.JSON_OBJECT.nodeCount >= cap)
                    reallocNode(pNode, newCap *= 2);
                pNode->tagVal.val.JSON_OBJECT.nodeCount++;
            }
            break;

        case JSON_ARRAY:
            {
                if (pNode->tagVal.val.JSON_ARRAY.tagValueCount >= cap)
                    reallocNode(pNode, newCap *= 2);
                pNode->tagVal.val.JSON_ARRAY.tagValueCount++;
            }
            break;
    }

    return newCap;
}

void
reallocNode(JSONNode* pNode, size_t size)
{
    switch (pNode->tagVal.tag)
    {
        default:
            PARSER_ERR(2, "reallocNode only for JSON_ARRAY and JSON_OBJECT\n");
            break;

        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                obj.aNodes = reallocarray(obj.aNodes, size, sizeof(JSONNode));
            }
            break;

        case JSON_ARRAY:
            {
                struct JSON_ARRAY obj = pNode->tagVal.val.JSON_ARRAY;
                obj.aTagValues = reallocarray(obj.aTagValues, size, sizeof(JSONTagVal));
            }
            break;
    }
}

void
ParserParseObject(Parser* self, JSONNode* pNode)
{
    /*reallocNode(pNode, 2);*/
    pNode->tagVal.tag = JSON_OBJECT;
    pNode->tagVal.val.JSON_OBJECT.aNodes = calloc(10, sizeof(JSONNode));
    pNode->tagVal.val.JSON_OBJECT.nodeCount = 0;
    size_t cap = 2;
    size_t i = 0;


    for (Token t = self->tCurr; t.type != TOK_RBRACE; t = ParserNext(self))
    {
        ParserExpect(self, TOK_IDENT);
        pNode->tagVal.val.JSON_OBJECT.aNodes[i].slKey = t.slLiteral;

        t = ParserNext(self);
        t = ParserNext(self);

        ParserParseNode(self, &pNode->tagVal.val.JSON_OBJECT.aNodes[i]);
        i++;

        if (self->tCurr.type != TOK_COMMA)
            break;
    }

    pNode->tagVal.val.JSON_OBJECT.nodeCount = i;
}

void
ParserParseArray(Parser* self, JSONNode* pNode)
{
}

void
ParserParseNumber(Parser* self, JSONTagVal* pNode)
{
    char buff[255] = {0};
    bool bReal = false;
    for (size_t i = 0; i < self->tCurr.slLiteral.size; i++)
    {
        buff[i] = self->tCurr.slLiteral.data[i];
        if (buff[i] == '.') bReal = true;

    }
    if (bReal)
        *pNode = JSON_NEW_TAGVAL(JSON_FLOAT, atof(buff));
    else
        *pNode = JSON_NEW_TAGVAL(JSON_INT, atoll(buff));

    ParserNext(self);
}

void
ParserParseIdent(Parser* self, JSONTagVal* pNode)
{
    *pNode = JSON_NEW_TAGVAL(JSON_STRING, self->tCurr.slLiteral);
    ParserNext(self);
}

void
ParserParseBool(Parser* self, JSONTagVal* pNode)
{
    bool b = self->tCurr.type == TOK_TRUE ? true : false;
    *pNode = JSON_NEW_TAGVAL(JSON_BOOL, b);
    ParserNext(self);
}

void
ParserParseNode(Parser* self, JSONNode* pNode)
{
    switch (self->tCurr.type)
    {
        case TOK_IDENT:
            ParserParseIdent(self, &pNode->tagVal);
            break;

        case TOK_NUMBER:
            ParserParseNumber(self, &pNode->tagVal);
            break;

        case TOK_LBRACE:
            ParserNext(self); /* skip brace */
            ParserParseObject(self, pNode);
            break;

        case TOK_TRUE:
        case TOK_FALSE:
            ParserParseBool(self, &pNode->tagVal);
            break;
    }
}

void
ParserPrintNode(Parser* self, JSONNode* pNode)
{
    SliceStr key = pNode->slKey;

    switch (pNode->tagVal.tag)
    {
        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                for (size_t i = 0; i < obj.nodeCount; i++)
                    ParserPrintNode(self, &obj.aNodes[i]);
            }
            break;

        case JSON_FLOAT:
            {
                float f = pNode->tagVal.val.JSON_FLOAT.f;
                COUT("{}: {}\n", key, f);
            }
            break;

        case JSON_INT:
            {
                int i = pNode->tagVal.val.JSON_INT.i;
                COUT("{}: {}\n", key, i);
            }
            break;

        case JSON_STRING:
            {
                SliceStr sl = pNode->tagVal.val.JSON_STRING.sl;
                COUT("{}: {}\n", key, sl);
            }
            break;

        case JSON_BOOL:
            {
                bool b = pNode->tagVal.val.JSON_BOOL.b;
                COUT("{}: {}\n", key, b);
            }
            break;
    }
}
