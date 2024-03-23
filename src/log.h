#include <iostream>

#define STDOUT_RESET    "\033[0m"
#define STDOUT_RED      "\033[31m"
#define STDOUT_YELLOW   "\033[33m"
#define STDOUT_GREEN    "\033[32m"

#define ERR(x) std::cout << STDOUT_RED << x << STDOUT_RESET << std::endl;
#define SUCC(x) std::cout << STDOUT_GREEN << x << STDOUT_RESET << std::endl;
#if defined(STAGE_LOGGING_WARN) || defined(STAGE_LOGGING_LOG)
#define WARN(x) std::cout << STDOUT_YELLOW << x << STDOUT_RESET << std::endl;
#else
#define WARN(X)
#endif
#if defined(STAGE_LOGGING_LOG)
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif
#if defined(STAGE_LOGGING_OFF)
#define ERR(x)
#define SUCC(x)
#define WARN(x)
#define LOG(x)
#endif

