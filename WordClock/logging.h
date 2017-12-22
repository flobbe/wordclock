#ifndef __LOGGING_H
#define __LOGGING_H

#include <Arduino.h>


#define LOG_PRINTER Serial

#ifdef DEBUG
    #define LOG_LEVEL LOG_LEVEL_VERBOSE
#else
    #define LOG_LEVEL LOG_LEVEL_NONE
#endif


#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4
#define LOG_LEVEL_VERBOSE 5


#if (LOG_LEVEL >= LOG_LEVEL_ERROR)
    #define LOG_ERROR(...)   LOG_PRINTER.printf(__VA_ARGS__)
#else
    #define LOG_ERROR(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_WARN)
    #define LOG_WARN(...)    LOG_PRINTER.printf(__VA_ARGS__)
#else
    #define LOG_WARN(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_INFO)
    #define LOG_INFO(...)    LOG_PRINTER.printf(__VA_ARGS__)
#else
    #define LOG_INFO(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_DEBUG)
    #define LOG_DEBUG(...)   LOG_PRINTER.printf(__VA_ARGS__)
#else
    #define LOG_DEBUG(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_VERBOSE)
    #define LOG_VERBOSE(...) LOG_PRINTER.printf(__VA_ARGS__)
#else
    #define LOG_VERBOSE(...)
#endif


#endif  // __LOGGING_H
