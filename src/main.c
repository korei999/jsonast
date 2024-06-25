#include "parse.h"

int
main(int argc, char* argv[])
{
    /*printf("jsonast version: %.1lf\n\n", JSONAST_VERSION);*/

    if (argc != 2)
    {
        COUT("usage: {} <path to json>\n", argv[0]);
        exit(3);
    }

    Parser p = {0};

    ParserLoadJson(&p, argv[1]);
    /*ParserLoadJson(&p, "test/t1.json");*/
    /*ParserLoadJson(&p, "test/t3.json");*/

    /*p.l.pos = 0;*/
    /*char* pData = p.l.slData.data;*/
    /*while (p.l.pos < p.l.slData.size)*/
    /*{*/
    /*    Token t = LexNext(&p.l);*/
    /*    COUT("{}: '{}'\n", tokenStrings[t.type], t.slLiteral);*/
    /*}*/
    /*COUT("\n");*/

    ParserParse(&p);
    ParserPrintJSON(&p);
    ParserClean(&p);
}
