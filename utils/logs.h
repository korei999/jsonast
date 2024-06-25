#pragma once
#include "misc.h"

#include <stdarg.h>

#ifdef LOGS

typedef char* PChar;
PAIR_GEN_CODE(SliceStr, PChar, size_t);

#define SLSTR_NEW_LIT(S) (SliceStr){{S}, {LENGTH(S) - 1}}

#define __COUT(FMT, ...) fprintg(stdout, FMT, GAS(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
#define __CERR(FMT, ...) fprintg(stderr, FMT, GAS(VA_NARGS(__VA_ARGS__), __VA_ARGS__))

#define COUTG(...) fprintg(stdout, __VA_ARGS__)
#define CERRG(...) fprintg(stderr, __VA_ARGS__)

/* black magic https://stackoverflow.com/a/32751188 */
#define VA_HAS_COMMA(...) VA_NARGS(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)

/* these are for use */
#define COUT(...) COUT2(VA_HAS_COMMA(__VA_ARGS__), __VA_ARGS__)
#define COUT2(N, ...) COUT3(N, __VA_ARGS__)
#define COUT3(N, ...) COUT##N(__VA_ARGS__)
#define COUT0(...) COUTG(__VA_ARGS__)
#define COUT1(...) __COUT(__VA_ARGS__)

#define CERR(...) CERR2(VA_HAS_COMMA(__VA_ARGS__), __VA_ARGS__)
#define CERR2(N, ...) CERR3(N, __VA_ARGS__)
#define CERR3(N, ...) CERR##N(__VA_ARGS__)
#define CERR0(...) CERRG(__VA_ARGS__)
#define CERR1(...) __CERR(__VA_ARGS__)

/* dumbest shit but works, up to 20 arguments */
#define __ADD_GA1(x, ...) GA(x)
#define __ADD_GA2(x, ...) GA(x), __ADD_GA1(__VA_ARGS__)
#define __ADD_GA3(x, ...) GA(x), __ADD_GA2(__VA_ARGS__)
#define __ADD_GA4(x, ...) GA(x), __ADD_GA3(__VA_ARGS__)
#define __ADD_GA5(x, ...) GA(x), __ADD_GA4(__VA_ARGS__)
#define __ADD_GA6(x, ...) GA(x), __ADD_GA5(__VA_ARGS__)
#define __ADD_GA7(x, ...) GA(x), __ADD_GA6(__VA_ARGS__)
#define __ADD_GA8(x, ...) GA(x), __ADD_GA7(__VA_ARGS__)
#define __ADD_GA9(x, ...) GA(x), __ADD_GA8(__VA_ARGS__)
#define __ADD_GA10(x, ...) GA(x), __ADD_GA9(__VA_ARGS__)
#define __ADD_GA11(x, ...) GA(x), __ADD_GA10(__VA_ARGS__)
#define __ADD_GA12(x, ...) GA(x), __ADD_GA11(__VA_ARGS__)
#define __ADD_GA13(x, ...) GA(x), __ADD_GA12(__VA_ARGS__)
#define __ADD_GA14(x, ...) GA(x), __ADD_GA13(__VA_ARGS__)
#define __ADD_GA15(x, ...) GA(x), __ADD_GA14(__VA_ARGS__)
#define __ADD_GA16(x, ...) GA(x), __ADD_GA15(__VA_ARGS__)
#define __ADD_GA17(x, ...) GA(x), __ADD_GA16(__VA_ARGS__)
#define __ADD_GA18(x, ...) GA(x), __ADD_GA17(__VA_ARGS__)
#define __ADD_GA19(x, ...) GA(x), __ADD_GA18(__VA_ARGS__)
#define __ADD_GA20(x, ...) GA(x), __ADD_GA19(__VA_ARGS__)
#define __ADD_GA(i, ...) __ADD_GA##i(__VA_ARGS__)
#define GAS(count, ...) __ADD_GA(count, __VA_ARGS__)

/* https://gist.github.com/aprell/3722962 */
/* now this is busted */
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define GA_NEW(TAG, VAL) (GenArg){.tag = TAG, .data.TAG = {VAL}}

#define GA(A)                                                                                                          \
    _Generic((A),                                                                                                      \
        SliceStr: GenArgSliceStr,                                                                                      \
        char: GenArgChar,                                                                                              \
        int: GenArgInt,                                                                                                \
        unsigned: GenArgUnsigned,                                                                                      \
        long: GenArgLong,                                                                                              \
        size_t: GenArgSizeT,                                                                                           \
        float: GenArgFloat,                                                                                            \
        double: GenArgDouble,                                                                                          \
        char*: GenArgPChar,                                                                                            \
        const char*: GenArgCPChar,                                                                                     \
        bool: GenArgBool,                                                                                              \
        void*: GenArgPVoid)(A)


typedef struct GenArg
{
    enum
    {
        GA_SLSTR,
        GA_CHAR,
        GA_INT,
        GA_UNSIGNED,
        GA_LONG,
        GA_SIZE_T,
        GA_FLOAT,
        GA_DOUBLE,
        GA_PCHAR,
        GA_CPCHAR,
        GA_BOOL,
        GA_PVOID
    } tag;
    union
    {
        struct GA_SLSTR { SliceStr sl; } GA_SLSTR;
        struct GA_CHAR { char c; } GA_CHAR;
        struct GA_INT { int i; } GA_INT;
        struct GA_UNSIGNED { unsigned u; } GA_UNSIGNED;
        struct GA_LONG { long l; } GA_LONG;
        struct GA_SIZE_T { size_t s; } GA_SIZE_T;
        struct GA_FLOAT { float f; } GA_FLOAT;
        struct GA_DOUBLE { double d; } GA_DOUBLE;
        struct GA_PCHAR { char* p; } GA_PCHAR;
        struct GA_CPCHAR { const char* cp; } GA_CPCHAR;
        struct GA_BOOL { bool b; } GA_BOOL;
        struct GA_PVOID { char* v; } GA_PVOID;
    } data;
} GenArg;

static inline GenArg
GenArgSliceStr(SliceStr sl)
{
    return GA_NEW(GA_SLSTR, sl);
}

static inline GenArg
GenArgChar(char c)
{
    return GA_NEW(GA_CHAR, c);
}

static inline GenArg
GenArgInt(int i)
{
    return GA_NEW(GA_INT, i);
}

static inline GenArg
GenArgUnsigned(int u)
{
    return GA_NEW(GA_UNSIGNED, u);
}

static inline GenArg
GenArgLong(long l)
{
    return GA_NEW(GA_LONG, l);
}

static inline GenArg
GenArgSizeT(size_t s)
{
    return GA_NEW(GA_SIZE_T, s);
}

static inline GenArg
GenArgFloat(float f)
{
    return GA_NEW(GA_FLOAT, f);
}

static inline GenArg
GenArgDouble(double d)
{
    return GA_NEW(GA_DOUBLE, d);
}

static inline GenArg
GenArgPChar(char* p)
{
    return GA_NEW(GA_PCHAR, p);
}

static inline GenArg
GenArgCPChar(const char* cp)
{
    return GA_NEW(GA_CPCHAR, cp);
}

static inline GenArg
GenArgBool(bool b)
{
    return GA_NEW(GA_BOOL, b);
}

static inline GenArg
GenArgPVoid(void* v)
{
    return GA_NEW(GA_PVOID, v);
}

static inline void
fprintg(FILE* fp, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (size_t pos = 0; fmt[pos]; pos++)
    {
        /* TODO: parse formatting 'args' like digits after dot etc... */
        if (fmt[pos] == '{' && fmt[pos + 1] && fmt[pos + 1] == '}')
        {
            GenArg ga = va_arg(args, GenArg);
            switch (ga.tag)
            {
                default:
                    fprintf(fp, "\"unhandled type\"");
                    break;

                case GA_SLSTR:
                    {
                        SliceStr sl = ga.data.GA_SLSTR.sl;
                        for (size_t i = 0; i < sl.size && sl.data[i]; i++)
                            fputc(sl.data[i], fp);
                    }
                    break;

                case GA_CHAR:
                    fprintf(fp, "%c", ga.data.GA_CHAR.c);
                    break;

                case GA_INT:
                    fprintf(fp, "%d", ga.data.GA_INT.i);
                    break;

                case GA_UNSIGNED:
                    fprintf(fp, "%u", ga.data.GA_UNSIGNED.u);
                    break;

                case GA_LONG:
                    fprintf(fp, "%ld", ga.data.GA_LONG.l);
                    break;

                case GA_SIZE_T:
                    fprintf(fp, "%zu", ga.data.GA_SIZE_T.s);
                    break;

                case GA_FLOAT:
                    fprintf(fp, "%f", ga.data.GA_FLOAT.f);
                    break;

                case GA_DOUBLE:
                    fprintf(fp, "%.17lf", ga.data.GA_DOUBLE.d);
                    break;

                case GA_PCHAR:
                    fprintf(fp, "%s", ga.data.GA_PCHAR.p);
                    break;

                case GA_CPCHAR:
                    fprintf(fp, "%s", ga.data.GA_CPCHAR.cp);
                    break;

                case GA_BOOL:
                    fprintf(fp, "%s", ga.data.GA_BOOL.b ? "true" : "false");
                    break;

                case GA_PVOID:
                    fprintf(fp, "%p", ga.data.GA_PVOID.v);
                    break;
            }
            pos++;
        }
        else
        {
            fputc(fmt[pos], fp);
        }
    }

    va_end(args);
}

enum _LOG_SEV
{
    _LOG_SEV_OK,
    _LOG_SEV_GOOD,
    _LOG_SEV_WARN,
    _LOG_SEV_BAD,
    _LOG_SEV_FATAL,
    _LOG_SEV_ENUM_SIZE
};

[[maybe_unused]] static const char* _LOG_SEV_STR[] = {"", "GOOD: ", "WARNING: ", "BAD: ", "FATAL: "};

    #define _LOG(SEV, ...)                                                                                             \
        {                                                                                                              \
            assert(SEV >= 0 && SEV < _LOG_SEV_ENUM_SIZE && "wrong _LOG_SEV*");                                         \
            CERR("({}{}, {}): ", _LOG_SEV_STR[SEV], __FILE__, __LINE__);                                               \
            CERR(__VA_ARGS__);                                                                                         \
            switch (SEV)                                                                                               \
            {                                                                                                          \
                default:                                                                                               \
                    break;                                                                                             \
                case _LOG_SEV_BAD:                                                                                     \
                    exit((int)SEV);                                                                                    \
                case _LOG_SEV_FATAL:                                                                                   \
                    abort();                                                                                           \
            }                                                                                                          \
        }

    #define LOG_OK(...) _LOG(_LOG_SEV_OK, __VA_ARGS__)
    #define LOG_WARN(...) _LOG(_LOG_SEV_WARN, __VA_ARGS__)
    #define LOG_BAD(...) _LOG(_LOG_SEV_BAD, __VA_ARGS__)
    #define LOG_FATAL(...) _LOG(_LOG_SEV_FATAL, __VA_ARGS__)

#endif
