#include "parse.h"

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
    self->pHead->strKey = path;

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

void
ParserClean(Parser* self)
{
    free(self->pHead); /* TODO: recursive free */
    LexClean(&self->l);
}

void
ParserNext(Parser* self)
{
    self->tCurr = self->tNext;
    self->tNext = LexNext(&self->l);
}

void
ParserParse(Parser* self, JSONNode* pNode)
{
    switch (self->tCurr.type)
    {
        case TOK_EOF:
            break;

        case TOK_LBRACE:
            break;
    }
}
