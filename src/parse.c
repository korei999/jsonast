#include "parse.h"

#define PARSER_ERR(CODE, ...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        CERR("parser error: {}, {}: ", self->tCurr.slLiteral, self->tNext.slLiteral);                                  \
        CERR(__VA_ARGS__);                                                                                             \
        exit(CODE);                                                                                                    \
    } while (0)

static void JSONParserParseNode(JSONParser* self, JSONNode* pNode);
static void JSONParserParseNumber(JSONParser* self, JSONTagVal* pNode);
static void JSONParserParseIdent(JSONParser* self, JSONTagVal* pNode);
static void JSONParserParseBool(JSONParser* self, JSONTagVal* pNode);
static void JSONParserParseNull(JSONParser* self, JSONTagVal* pNode);

/* TODO: expects are not very useful rn */
static void
JSONParserExpect(JSONParser* self, enum JSONToken t)
{
    if (self->tCurr.type != t)
        PARSER_ERR(2, "next expected: '{}', got '{}' instead\n", tokenStrings[t], tokenStrings[self->tCurr.type]);
}

/*static void*/
/*ParserExpectNext(JSONParser* self, enum JSONToken t)*/
/*{*/
/*    if (self->tNext.type != t)*/
/*        PARSER_ERR(2, "curr expected: '{}', got '{}' instead\n", tokenStrings[t], tokenStrings[self->tNext.type]);*/
/*}*/

JSONParser
JSONParserCreate(Arena* a)
{
    JSONParser p = {0};
    p.pArena = a;
    p.l = LexCreate(a);

    return p;
}

void
JSONParserLoadJSON(JSONParser* self, char* path)
{
    LexLoadFile(&self->l, path);

    if (self->l.slData.size <= 1)
    {
        PARSER_ERR(2, "unable to open file '{}'\n", path);
    }

    self->pHead = ArenaCalloc(self->pArena, 1, sizeof(JSONNode));
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
JSONParserCleanNode(JSONParser* self, JSONNode* pNode)
{
    switch (pNode->tagVal.tag)
    {
        default:
            break;

        case JSON_OBJECT:
            {
                struct JSON_OBJECT obj = pNode->tagVal.val.JSON_OBJECT;
                for (size_t i = 0; i < obj.nodeCount; i++)
                    JSONParserCleanNode(self, &obj.aNodes[i]);
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
                        JSONParserCleanNode(self, arr.aTagValues[i].val.JSON_OBJECT.aNodes);
                        free(arr.aTagValues[i].val.JSON_OBJECT.aNodes);
                    }
                }
                free(arr.aTagValues);
            }
            break;
    }
}

void
JSONParserParse(JSONParser* self)
{
    JSONParserParseNode(self, self->pHead);
}

Token
JSONParserNext(JSONParser* self)
{
    self->tCurr = self->tNext;
    self->tNext = LexNext(&self->l);

    return self->tCurr;
}

void
JSONParserParseObject(JSONParser* self, JSONNode* pNode)
{
    pNode->tagVal.tag = JSON_OBJECT;
    pNode->tagVal.val.JSON_OBJECT.aNodes = ArenaCalloc(self->pArena, 4, sizeof(JSONNode));
    pNode->tagVal.val.JSON_OBJECT.nodeCount = 0;
    size_t cap = 4;
    size_t i = 0;

    /* collect each key/value pair in the object */
    for (Token t = self->tCurr; t.type != TOK_RBRACE; t = JSONParserNext(self))
    {
        if (i >= cap)
            pNode->tagVal.val.JSON_OBJECT.aNodes = ArenaRealloc(self->pArena,
                                                                pNode->tagVal.val.JSON_OBJECT.aNodes,
                                                                i * sizeof(JSONNode),
                                                                (cap *= 2) * sizeof(JSONNode));

        JSONParserExpect(self, TOK_IDENT);
        pNode->tagVal.val.JSON_OBJECT.aNodes[i].slKey = t.slLiteral;

        /* skip identifier and ':' */
        t = JSONParserNext(self);
        JSONParserExpect(self, TOK_ASSIGN);
        t = JSONParserNext(self);

        JSONParserParseNode(self, &pNode->tagVal.val.JSON_OBJECT.aNodes[i]);
        i++;

        if (self->tCurr.type != TOK_COMMA)
        {
            JSONParserNext(self);
            break;
        }
    }

    pNode->tagVal.val.JSON_OBJECT.nodeCount = i;
}

static void
JSONParserParseArray(JSONParser* self, JSONNode* pNode)
{
    pNode->tagVal.tag = JSON_ARRAY;
    pNode->tagVal.val.JSON_ARRAY.aTagValues = (JSONTagVal*)ArenaAlloc(self->pArena, 4 * sizeof(JSONTagVal));
    pNode->tagVal.val.JSON_ARRAY.tagValueCount = 0;
    size_t cap = 4;
    size_t i = 0;

    /* collect each key/value pair in the object */
    for (Token t = self->tCurr; t.type != TOK_RBRACKET; t = JSONParserNext(self))
    {
        if (i >= cap)
            pNode->tagVal.val.JSON_ARRAY.aTagValues = ArenaRealloc(self->pArena,
                                                                   pNode->tagVal.val.JSON_ARRAY.aTagValues,
                                                                   i * sizeof(JSONTagVal),
                                                                   (cap *= 2) * sizeof(JSONTagVal));

        auto tt = t.type;
        switch (tt)
        {
            default:
            case TOK_IDENT:
                JSONParserParseIdent(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_NULL:
                JSONParserParseNull(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_TRUE:
            case TOK_FALSE:
                JSONParserParseBool(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_NUMBER:
                JSONParserParseNumber(self, &pNode->tagVal.val.JSON_ARRAY.aTagValues[i]);
                break;

            case TOK_LBRACE:
                t = JSONParserNext(self);
                pNode->tagVal.val.JSON_ARRAY.aTagValues[i].tag = JSON_OBJECT;
                pNode->tagVal.val.JSON_ARRAY.aTagValues[i].val.JSON_OBJECT.aNodes = ArenaCalloc(self->pArena, 1, sizeof(JSONNode));
                JSONParserParseObject(self, pNode->tagVal.val.JSON_ARRAY.aTagValues[i].val.JSON_OBJECT.aNodes);
                break;
        }

        i++;

        if (self->tCurr.type != TOK_COMMA)
        {
            JSONParserNext(self);
            break;
        }
    }

    pNode->tagVal.val.JSON_OBJECT.nodeCount = i;
}

static void
JSONParserParseNumber(JSONParser* self, JSONTagVal* pNode)
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

    JSONParserNext(self);
}

static void
JSONParserParseIdent(JSONParser* self, JSONTagVal* pNode)
{
    *pNode = JSON_NEW_TAGVAL(JSON_STRING, self->tCurr.slLiteral);
    JSONParserNext(self);
}

static void
JSONParserParseBool(JSONParser* self, JSONTagVal* pNode)
{
    bool b = self->tCurr.type == TOK_TRUE ? true : false;
    *pNode = JSON_NEW_TAGVAL(JSON_BOOL, b);
    JSONParserNext(self);
}

static void
JSONParserParseNull(JSONParser* self, JSONTagVal* pNode)
{
    *pNode = JSON_NEW_TAGVAL(JSON_NULL, NULL);
    JSONParserNext(self);
}

static void
JSONParserParseNode(JSONParser* self, JSONNode* pNode)
{
    switch (self->tCurr.type)
    {
        default:
            JSONParserNext(self);
            break;

        case TOK_IDENT:
            JSONParserParseIdent(self, &pNode->tagVal);
            break;

        case TOK_NUMBER:
            JSONParserParseNumber(self, &pNode->tagVal);
            break;

        case TOK_LBRACE:
            JSONParserNext(self); /* skip brace */
            JSONParserParseObject(self, pNode);
            break;

        case TOK_LBRACKET:
            JSONParserNext(self); /* skip bracket */
            JSONParserParseArray(self, pNode);
            break;

        case TOK_NULL:
            JSONParserParseNull(self, &pNode->tagVal);
            break;

        case TOK_TRUE:
        case TOK_FALSE:
            JSONParserParseBool(self, &pNode->tagVal);
            break;
    }
}

static void
JSONParserPrintNode(JSONParser* self, JSONNode* pNode, SliceStr slEnding)
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
                    JSONParserPrintNode(self, &obj.aNodes[i], slE);
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
                        case JSON_STRING:
                            {
                                SliceStr sl = arr.aTagValues[i].val.JSON_STRING.sl;
                                COUT("{}{}", sl, slE);
                            }
                            break;

                        case JSON_NULL:
                                COUT("{}{}", "null", slE);
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

                        case JSON_OBJECT:
                                JSONParserPrintNode(self, arr.aTagValues[i].val.JSON_OBJECT.aNodes, slE);
                            break;
                    }
                }
                COUT("]{}", slEnding);
            }
            break;

        case JSON_FLOAT:
            {
                /* TODO: add some sort formatting for floats */
                double f = pNode->tagVal.val.JSON_FLOAT.f;
                COUT("{}: {}{}", key, f, slEnding);
            }
            break;

        case JSON_INT:
            {
                int i = pNode->tagVal.val.JSON_INT.i;
                COUT("{}: {}{}", key, i, slEnding);
            }
            break;

        case JSON_NULL:
                COUT("{}: {}{}", key, "null", slEnding);
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
JSONParserPrintJSON(JSONParser* self)
{
    JSONParserPrintNode(self, self->pHead, SLSTR_NEW_LIT(""));
    COUT("\n");
}
