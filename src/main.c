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

    ParserParse(&p);
    ParserPrintJSON(&p);
    ParserClean(&p);
}
