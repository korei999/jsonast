#include "parse.h"

int
main()
{
    printf("jsonast version: %.1lf\n\n", JSONAST_VERSION);

    Parser p = {0};
    ParserLoadJson(&p, "test/t3.json");

    ParserClean(&p);
}
