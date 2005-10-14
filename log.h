#ifndef LOG_H
#define LOG_H
#include <stdio.h>


#ifndef STRINGSIZE
#define STRINGSIZE 128 //standard size for string allocation
#endif


#ifdef DEBUG
#define DEBUG_PRINT(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else
#define DEBUG_PRINT(...) do { } while (0)
#endif

#define Log(warn, ...) do {  \
    enum priority a = warn;  \
    switch(a) { \
    case ERROR: \
      fprintf(stderr, "ERROR: (%s, %d): ", __FILE__, __LINE__);\
      fprintf(stderr, __VA_ARGS__);\
      fprintf(stderr, "\n");\
      break;\
    case WARNING:\
      fprintf(stderr, "WARNING: (%s, %d): ", __FILE__, __LINE__);\
      fprintf(stderr, __VA_ARGS__);\
      fprintf(stderr, "\n");\
      break;\
    case NOTICE:\
      fprintf(stderr, __VA_ARGS__);\
      fprintf(stderr, "\n");\
      break;\
    case USEFUL:\
      fprintf(stderr, __VA_ARGS__);\
      fprintf(stderr, "\n");\
      break;\
    case DEBUGGING:				\
      DEBUG_PRINT(__VA_ARGS__);			\
      break;\
    default:\
      break;\
    };\
  } while (0)

      

enum priority { ERROR, WARNING, NOTICE, USEFUL, DEBUGGING };



#endif
