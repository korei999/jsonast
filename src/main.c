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

    /* 1 megabyte should be more then enough */
    Arena frameArena = ArenaCreate(ARENA_1M);

    JSONParser p = JSONParserCreate(&frameArena);

    JSONParserLoadJSON(&p, argv[1]);

    JSONParserParse(&p);
    JSONParserPrintJSON(&p);

    /* just one free */
    ArenaFree(&frameArena);
}
