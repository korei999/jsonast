#include <ctype.h>
#include <string.h>

#include "lex.h"
#include "misc.h"

void
LexLoadFile(Lex* self, char* path)
{
    self->fPath = path;
    Slice f = loadFile(path);
    self->slData.data = (char*)f.data;
    self->slData.size = f.size;
    self->pos = 0;
    self->column = self->line = 1;
}

void
LexClean(Lex* self)
{
    free(self->slData.data);
}

void
LexSkipWhiteSpace(Lex* self)
{
    size_t i = self->pos;
    size_t size = self->slData.size;
    char* pData = self->slData.data;

    while (i < size && (pData[i] == ' ' || pData[i] == '\t' || pData[i] == '\n'))
    {
        self->column++;
        if (pData[i] == '\n')
        {
            self->line++;
            /*self->column = 1;*/
        }
        else if (pData[i] == '\t')
        {
            /*self->column += 3;*/
        }

        i++;
    }

    self->pos = i;
}

Token
LexNext(Lex* self)
{
    Token r = {0};

    if (self->pos >= self->slData.size)
        return r; /* will return TOK_EOF */

    char* pData = self->slData.data;

    LexSkipWhiteSpace(self);

    switch (pData[self->pos])
    {
        default:
            r = LexStringNoQuotes(self);
            if (r.slLiteral.size == 0)
                self->pos++;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            r = LexNumber(self);
            break;

        case TOK_QUOTE:
            r = LexString(self);
            break;

        case TOK_COMMA:
            r = LexChar(self, TOK_COMMA);
            break;

        case TOK_ASSIGN:
            r = LexChar(self, TOK_ASSIGN);
            break;

        case TOK_LBRACE:
            r = LexChar(self, TOK_LBRACE);
            break;

        case TOK_RBRACE:
            r = LexChar(self, TOK_RBRACE);
            break;

        case TOK_RBRACKET:
            r = LexChar(self, TOK_RBRACKET);
            break;

        case TOK_LBRACKET:
            r = LexChar(self, TOK_LBRACKET);
            break;

        case TOK_EOF:
            r.type = TOK_EOF;
            break;
    }

    self->pos++;
    /*self->column++;*/
    return r;
}

Token
LexString(Lex* self)
{
    Token r = {0};
    char* pData = self->slData.data;
    size_t start = self->pos;
    size_t i = start + 1;

    while (pData[i] != '"')
    {
        if (pData[i] == TOK_EOF)
            LEX_ERR(1, "Unterminated string\n");
        else if (pData[i] == '\n')
            LEX_ERR(1, "Next line within strings are not allowed\n");

        /*self->column++;*/
        i++;
    }

    r.type = TOK_IDENT;
    r.slLiteral.data = &pData[start];
    r.slLiteral.size = (i - start) + 1;

    self->pos = i;
    return r;
}

Token
LexNumber(Lex* self)
{
    Token r = {0};

    char* pData = self->slData.data;
    size_t start = self->pos;
    size_t i = start;
    size_t ni = 0;

    while (isxdigit(pData[i]) || pData[i] == '.') /* floats */
    {
        /*self->column++;*/
        i++;
    }

    r.type = TOK_NUMBER;
    r.slLiteral.data = &pData[start];
    r.slLiteral.size = i - start;

    self->pos = i - 1; /* might skip comma */
    /*self->column--;*/
    return r;
}

Token
LexStringNoQuotes(Lex* self)
{
    Token r = {0};

    char* pData = self->slData.data;
    size_t start = self->pos;
    size_t i = start;
    size_t ni = 0;

    while (isalpha(pData[i]))
    {
        /*self->column++;*/
        i++;
    }

    r.type = TOK_IDENT;
    r.slLiteral.data = &pData[start];
    r.slLiteral.size = i - start;

    if (strncmp("false", r.slLiteral.data, r.slLiteral.size) == 0)
        r.type = TOK_FALSE;
    else if (strncmp("true", r.slLiteral.data, r.slLiteral.size) == 0)
        r.type = TOK_TRUE;
    else
        r.type = TOK_IDENT;

    self->pos = i - 1; /* might skip comma */
    /*self->column--;*/
    return r;
}

Token
LexChar(Lex* self, enum JSONToken type)
{
    return (Token) {
        .type = type,
        .slLiteral.data = &self->slData.data[self->pos],
        .slLiteral.size = 1
    };
}