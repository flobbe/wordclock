#ifndef __ASSERTIONS_H
#define __ASSERTIONS_H

#include "logging.h"


#ifdef DEBUG
    #define ASSERT(cond)  do if (!(cond)) while (1) { \
                              LOG_ERROR("ASSERTION FAILED (%s:%d): '%s'\n", __FILE__, __LINE__, #cond); \
                              delay(1000); \
                          } while (0)
#else
    #define ASSERT(cond)
#endif


#endif  // __ASSERTIONS_H
