#pragma once
#include "ast.h"
#include "misc.h"

#define LEX_ERR(CODE, ...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        CERR("({}: {}): ", self->fPath, self->line);                                                                   \
        CERR(__VA_ARGS__);                                                                                             \
        exit(CODE);                                                                                                    \
    } while (0)

enum JSONToken
{
    TOK_LBRACE = '{',
    TOK_RBRACE = '}',
    TOK_LBRACKET = '[',
    TOK_RBRACKET = ']',
    TOK_QUOTE = '"',
    TOK_IDENT = 'I',
    TOK_NUMBER = 'N',
    TOK_TRUE = 'T',
    TOK_FALSE = 'F',
    TOK_NULL = 'n',
    TOK_ASSIGN = ':',
    TOK_COMMA = ',',
    TOK_DOT = '.',
    TOK_UNHANDLED = 'X',
    TOK_EOF = '\0',
};

[[maybe_unused]] static const SliceStr tokenStrings[] = {
    [TOK_LBRACE]    = SLSTR("TOK_LBRACE"),
    [TOK_RBRACE]    = SLSTR("TOK_RBRACE"),
    [TOK_LBRACKET]  = SLSTR("TOK_LBRACKET"),
    [TOK_RBRACKET]  = SLSTR("TOK_RBRACKET"),
    [TOK_QUOTE]     = SLSTR("TOK_QUOTE"),
    [TOK_IDENT]     = SLSTR("TOK_IDENT"),
    [TOK_NUMBER]    = SLSTR("TOK_NUMBER"),
    [TOK_TRUE]      = SLSTR("TOK_TRUE"),
    [TOK_FALSE]     = SLSTR("TOK_FALSE"),
    [TOK_ASSIGN]    = SLSTR("TOK_ASSIGN"),
    [TOK_COMMA]     = SLSTR("TOK_COMMA"),
    [TOK_DOT]       = SLSTR("TOK_DOT"),
    [TOK_UNHANDLED] = SLSTR("TOK_UNHANDLED"),
    [TOK_EOF]       = SLSTR("TOK_EOF"),
};

typedef struct Token Token;
typedef struct Lex Lex;

struct Token
{
    enum JSONToken type;
    SliceStr slLiteral;
};

struct Lex
{
    char* fPath;
    size_t pos;
    SliceStr slData;
    SliceStr slTok;
    size_t line;
    size_t column;
    Arena* pArena;
};

Lex LexCreate(Arena* a);
void LexError(Lex* self);
void LexLoadFile(Lex* self, char* path);
void LexSkipWhiteSpace(Lex* self);
Token LexNext(Lex* self);
Token LexString(Lex* self);
Token LexStringNoQuotes(Lex* self);
Token LexNumber(Lex* self);
Token LexChar(Lex* self, enum JSONToken type);
