#include "parse.h"

int
main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("jsonast version: %.1lf\n\n", JSONAST_VERSION);
        COUT("usage: {} <path to json>\n", argv[0]);
        exit(3);
    }

    Parser p = {0};

    ParserLoadJSON(&p, argv[1]);

    ParserParse(&p);
    ParserPrintJSON(&p);
    ParserClean(&p);
}
