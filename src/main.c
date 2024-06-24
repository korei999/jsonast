#include "parse.h"

int
main()
{
    /*printf("jsonast version: %.1lf\n\n", JSONAST_VERSION);*/

    Parser p = {0};
    ParserLoadJson(&p, "test/t3.json");

    ParserParseNode(&p, p.pHead);

    /*p.l.pos = 0;*/
    /*char* pData = p.l.slData.data;*/
    /*while (p.l.pos < p.l.slData.size)*/
    /*{*/
    /*    Token t = LexNext(&p.l);*/
    /*    COUT("{}: '{}'\n", tokenStrings[t.type], t.slLiteral);*/
    /*}*/
    /*COUT("\n");*/

    ParserPrintJSON(&p);
    ParserClean(&p);
}
