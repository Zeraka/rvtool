#ifndef UTIL_DEBUG_HH
#define UTIL_DEBUG_HH

enum
{
    SUCCESS = 0,
    ERROR,
    WORD_ACCEPTANCE_WRONG,
    NOMATCH
};

typedef enum
{
    AOP_LOG_NOTSET = -1,
    AOP_LOG_NONE = 0,
    AOP_LOG_EMERGENCY,
    AOP_LOG_ALERT,
    AOP_LOG_ERROR,
    AOP_LOG_WARING,
    AOP_LOG_NOTICE,
    AOP_LOG_INFO,
    AOP_LOG_PERF,
    AOP_LOG_CONFIG,
    AOP_LOG_DEBUG,
    AOP_LOG_LEVEL_MAX
} AOPLogLevel;

/*color*/
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */
//end

#define TEST_AUTOMONTIRO 1
#define ZMQ 1
#define DEBUG 1
#if DEBUG == 1
#define FuncBegin()                                                                                                      \
    {                                                                                                                    \
        std::cout << "\n"                                                                                                \
                  << BOLDCYAN << "Function " << BOLDRED << __FUNCTION__ << BOLDYELLOW << " begin" << RESET << std::endl; \
    }

#define FuncEnd()                                                                                                     \
    {                                                                                                                 \
        std::cout << BOLDCYAN << "Function " << BOLDRED << __FUNCTION__ << BOLDYELLOW << " end\n" RESET << std::endl; \
    }

#define FuncEnd_print(x)                                     \
    {                                                        \
        std::cout << "Function " << __FUNCTION__ << " end\n" \
                  << std::endl;                              \
        std::cout << x << std::endl;                         \
    }

#else

#define FuncBegin()
#define FuncEnd()

#endif //DEBUG

//判断输入的是否是字符串,待补充
#define INFOPrint(x)                                        \
    {                                                       \
        std::cout << BOLDYELLOW << x << RESET << std::endl; \
    }
/*打印x的值,便于debug*/
#define VePrint(x)                                                                                      \
    {                                                                                                   \
        std::cout << BOLDYELLOW << "The value of " << #x << ": " << BOLDRED << x << RESET << std::endl; \
    }

//测试时间性能的模块

#define AMReturn(x)                                                                             \
    {                                                                                           \
        std::cout << BOLDRED << "Fucntion " << __FUNCTION__ << " Error!" << RESET << std::endl; \
        return x;                                                                               \
    }

#endif