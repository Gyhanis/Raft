#pragma once 
#include <stdio.h>
// NONE         0
// ERROR        1
// WARNING      2
// INFOMATION   3
// DEBUG        4

#ifndef LOG_LEVEL
        #warning setting LOG_LEVEL to 0 by default.
        #define LOG_LEVEL 0
#endif 

#if LOG_LEVEL >= 4
        #define DEBUG(fmt, ...) fprintf(stderr, "[D] " fmt, ##__VA_ARGS__)
#else 
        #define DEBUG(fmt, ...) 
#endif 

#if LOG_LEVEL >= 3
        #define INFO(fmt, ...) fprintf(stderr, "[I] " fmt, ##__VA_ARGS__)
#else 
        #define INFO(fmt, ...) 
#endif 

#if LOG_LEVEL >= 2
        #define WARNING(fmt, ...) fprintf(stderr, "[W] " fmt, ##__VA_ARGS__)
#else 
        #define WARNING(fmt, ...) 
#endif 

#if LOG_LEVEL >= 1
        #define ERROR(fmt, ...) fprintf(stderr, "[E] " fmt, ##__VA_ARGS__)
#else 
        #define ERROR(fmt, ...) 
#endif 