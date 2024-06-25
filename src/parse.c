#include "parse.h"

#define PARSER_ERR(CODE, ...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        CERR("parser error: ");                                                                                        \
        CERR(__VA_ARGS__);                                                                                             \
    } while (0)

static void ParserParseNode(Parser* self, JSONNode* pNode);
static void ParserParseNumber(Parser* self, JSONTagVal* pNode);
static void ParserParseIdent(Parser* self, JSONTagVal* pNode);
static void ParserParseBool(Parser* self, JSONTagVal* pNode);

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
        default:
            break;

        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                for (size_t i = 0; i < obj.nodeCount; i++)
                    ParserCleanNode(self, &obj.aNodes[i]);
                free(obj.aNodes);
            }
            break;

        case JSON_ARRAY:
            {
                struct JSON_ARRAY arr = pNode->tagVal.val.JSON_ARRAY;
                for (size_t i = 0; i < arr.tagValueCount; i++)
                {
                    if (arr.aTagValues[i].tag == JSON_OBJECT)
                    {
                        ParserCleanNode(self, arr.aTagValues[i].val.JSON_OBJECT.aNodes);
                        free(arr.aTagValues[i].val.JSON_OBJECT.aNodes);
                    }
                }
                free(arr.aTagValues);
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

void
ParserParse(Parser* self)
{
    ParserParseNode(self, self->pHead);
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

void
ParserParseObject(Parser* self, JSONNode* pNode)
{
    pNode->tagVal.tag = JSON_OBJECT;
    pNode->tagVal.val.JSON_OBJECT.aNodes = (JSONNode*)calloc(4, sizeof(JSONNode));
    pNode->tagVal.val.JSON_OBJECT.nodeCount = 0;
    size_t cap = 4;
    size_t i = 0;

    /* collect each key/value pair in the object */
    for (Token t = self->tCurr; t.type != TOK_RBRACE; t = ParserNext(self))
    {
        if (i >= cap)
            pNode->tagVal.val.JSON_OBJECT.aNodes = reallocarray(pNode->tagVal.val.JSON_OBJECT.aNodes, cap *= 2, sizeof(JSONNode));

        ParserExpect(self, TOK_IDENT);
        pNode->tagVal.val.JSON_OBJECT.aNodes[i].slKey = t.slLiteral;

        /* skip identifier and ':' */
        t = ParserNext(self);
        t = ParserNext(self);

        ParserParseNode(self, &pNode->tagVal.val.JSON_OBJECT.aNodes[i]);
        i++;

        if (self->tCurr.type != TOK_COMMA)
        {
            ParserNext(self);
            break;
        }
    }

    pNode->tagVal.val.JSON_OBJECT.nodeCount = i;
}

static void
ParserParseArray(Parser* self, JSONNode* pNode)
{
    pNode->tagVal.tag = JSON_ARRAY;
    pNode->tagVal.val.JSON_ARRAY.aTagValues = (JSONTagVal*)calloc(4, sizeof(JSONTagVal));
    pNode->tagVal.val.JSON_ARRAY.tagValueCount = 0;
    size_t cap = 4;
    size_t i = 0;

    /* collect each key/value pair in the object */
    for (Token t = self->tCurr; t.type != TOK_RBRACKET; t = ParserNext(self))
    {
        if (i >= cap)
            pNode->tagVal.val.JSON_ARRAY.aTagValues = reallocarray(pNode->tagVal.val.JSON_ARRAY.aTagValues, cap *= 2, sizeof(JSONTagVal));

        auto tt = t.type;
        switch (tt)
        {
            default:
            case TOK_TRUE:
            case TOK_FALSE:
                ParserParseBool(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_NUMBER:
                ParserParseNumber(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_IDENT:
                ParserParseIdent(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_LBRACE:
                t = ParserNext(self);
                pNode->tagVal.val.JSON_ARRAY.aTagValues[i].tag = JSON_OBJECT;
                pNode->tagVal.val.JSON_ARRAY.aTagValues[i].val.JSON_OBJECT.aNodes = (JSONNode*)calloc(1, sizeof(JSONNode));
                ParserParseObject(self, pNode->tagVal.val.JSON_ARRAY.aTagValues[i].val.JSON_OBJECT.aNodes);
                break;
        }

        i++;

        if (self->tCurr.type != TOK_COMMA)
        {
            ParserNext(self);
            break;
        }
    }

    pNode->tagVal.val.JSON_OBJECT.nodeCount = i;
}

static void
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

static void
ParserParseIdent(Parser* self, JSONTagVal* pNode)
{
    *pNode = JSON_NEW_TAGVAL(JSON_STRING, self->tCurr.slLiteral);
    ParserNext(self);
}

static void
ParserParseBool(Parser* self, JSONTagVal* pNode)
{
    bool b = self->tCurr.type == TOK_TRUE ? true : false;
    *pNode = JSON_NEW_TAGVAL(JSON_BOOL, b);
    ParserNext(self);
}

static void
ParserParseNode(Parser* self, JSONNode* pNode)
{
    switch (self->tCurr.type)
    {
        default:
            ParserNext(self);
            break;

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

        case TOK_LBRACKET:
            ParserNext(self); /* skip bracket */
            ParserParseArray(self, pNode);
            break;

        case TOK_TRUE:
        case TOK_FALSE:
            ParserParseBool(self, &pNode->tagVal);
            break;
    }
}

static void
ParserPrintNode(Parser* self, JSONNode* pNode, SliceStr slEnding)
{
    SliceStr key = pNode->slKey;

    switch (pNode->tagVal.tag)
    {
        default:
            break;

        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                SliceStr objName0 = key.size == 0 ? SLSTR_NEW_LIT("") : key;
                SliceStr objName1 = objName0.size > 0 ? SLSTR_NEW_LIT(": ") : SLSTR_NEW_LIT("");

                COUT("{}{}{\n", objName0, objName1);
                for (size_t i = 0; i < obj.nodeCount; i++)
                {
                    SliceStr slE = (i == obj.nodeCount - 1) ? SLSTR_NEW_LIT("\n") : SLSTR_NEW_LIT(",\n");
                    ParserPrintNode(self, &obj.aNodes[i], slE);
                }
                COUT("}{}", slEnding);
            }
            break;

        case JSON_ARRAY:
            {
                struct JSON_ARRAY arr = pNode->tagVal.val.JSON_ARRAY;
                SliceStr arrName0 = key.size == 0 ? SLSTR_NEW_LIT("") : key;
                SliceStr arrName1 = arrName0.size > 0 ? SLSTR_NEW_LIT(": ") : SLSTR_NEW_LIT("");

                COUT("{}{}[\n", arrName0, arrName1);
                for (size_t i = 0; i < arr.tagValueCount; i++)
                {
                    SliceStr slE = (i == arr.tagValueCount - 1) ? SLSTR_NEW_LIT("\n") : SLSTR_NEW_LIT(",\n");

                    switch (arr.aTagValues[i].tag)
                    {
                        default:
                            /*COUT("{}, n: {}\n", tokenStrings[arr.aTagValues[i].tag], arr.tagValueCount);*/
                            break;

                        case JSON_INT:
                            {
                                int num = arr.aTagValues[i].val.JSON_INT.i;
                                COUT("{}{}", num, slE);
                            }
                            break;

                        case JSON_FLOAT:
                            {
                                double dnum = arr.aTagValues[i].val.JSON_FLOAT.f;
                                COUT("{}{}", dnum, slE);
                            }
                            break;

                        case JSON_BOOL:
                            {
                                bool b = arr.aTagValues[i].val.JSON_BOOL.b;
                                COUT("{}{}", b, slE);
                            }
                            break;

                        case JSON_STRING:
                            {
                                SliceStr sl = arr.aTagValues[i].val.JSON_STRING.sl;
                                COUT("{}{}", sl, slE);
                            }
                            break;

                        case JSON_OBJECT:
                            {
                                ParserPrintNode(self, arr.aTagValues[i].val.JSON_OBJECT.aNodes, slE);
                            }
                            break;
                    }
                }
                COUT("]{}", slEnding);
            }
            break;

        case JSON_FLOAT:
            {
                float f = pNode->tagVal.val.JSON_FLOAT.f;
                COUT("{}: {}{}", key, f, slEnding);
            }
            break;

        case JSON_INT:
            {
                int i = pNode->tagVal.val.JSON_INT.i;
                COUT("{}: {}{}", key, i, slEnding);
            }
            break;

        case JSON_STRING:
            {
                SliceStr sl = pNode->tagVal.val.JSON_STRING.sl;
                COUT("{}: {}{}", key, sl, slEnding);
            }
            break;

        case JSON_BOOL:
            {
                bool b = pNode->tagVal.val.JSON_BOOL.b;
                COUT("{}: {}{}", key, b, slEnding);
            }
            break;
    }
}

void
ParserPrintJSON(Parser* self)
{
    ParserPrintNode(self, self->pHead, SLSTR_NEW_LIT(""));
    COUT("\n");
}
