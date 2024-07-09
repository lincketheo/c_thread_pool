#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>

// Regular colors
#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"

// Bold colors
#define BOLD_BLACK   "\033[1;30m"
#define BOLD_RED     "\033[1;31m"
#define BOLD_GREEN   "\033[1;32m"
#define BOLD_YELLOW  "\033[1;33m"
#define BOLD_BLUE    "\033[1;34m"
#define BOLD_MAGENTA "\033[1;35m"
#define BOLD_CYAN    "\033[1;36m"
#define BOLD_WHITE   "\033[1;37m"

// Reset
#define RESET   "\033[0m"

#ifndef NLOGS
#define log_common(str, prefix, color, fmt, ...) \
  fprintf(str, BOLD_##color prefix RESET color " [%s(%s:%d)]: " RESET fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
#define log_common(str, prefix, color, fmt, ...)
#endif

#define log_infoln(fmt, ...) log_common(stdout, "INFO", WHITE, fmt "\n", ##__VA_ARGS__)

#ifndef NDEBUG
#define log_debugln(fmt, ...) log_common(stdout, "DEBUG", BLUE, fmt "\n", ##__VA_ARGS__)
#else
#define log_debugln(fmt, ...)
#endif

#define log_warnln(fmt, ...) log_common(stdout, "WARN", YELLOW, fmt "\n", ##__VA_ARGS__)

#define log_errorln_errno(fmt, ...) do {\
  if(errno > 0) { \
    log_common(stderr, "ERROR", RED, fmt " -- errno: %s\n", ##__VA_ARGS__, strerror(errno)); \
  } else { \
    log_common(stderr, "ERROR", RED, fmt "\n", ##__VA_ARGS__); \
  } \
} while (0)

#define log_errorln(fmt, ...) log_common(stderr, "ERROR", RED, fmt "\n", ##__VA_ARGS__);
