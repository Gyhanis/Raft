#pragma once 
#include <stdio.h>
#include <time.h>
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
        #define DEBUG(fmt, ...) {\
                time_t t = time(NULL);\
                struct tm *t2 = localtime(&t);\
                fprintf(stderr, "%02d:%02d:%02d [D] " fmt, \
                        t2->tm_hour, t2->tm_min, t2->tm_sec, ##__VA_ARGS__);}
#else 
        #define DEBUG(fmt, ...) 
#endif 

#if LOG_LEVEL >= 3
        #define INFO(fmt, ...) {\
                time_t t = time(NULL);\
                struct tm *t2 = localtime(&t);\
                fprintf(stderr, "%02d:%02d:%02d [I] " fmt, \
                        t2->tm_hour, t2->tm_min, t2->tm_sec, ##__VA_ARGS__);}
#else 
        #define INFO(fmt, ...) 
#endif 

#if LOG_LEVEL >= 2
        #define WARNING(fmt, ...) {\
                time_t t = time(NULL);\
                struct tm *t2 = localtime(&t);\
                fprintf(stderr, "%02d:%02d:%02d [W] " fmt, \
                        t2->tm_hour, t2->tm_min, t2->tm_sec, ##__VA_ARGS__);}
#else 
        #define WARNING(fmt, ...) 
#endif 

#if LOG_LEVEL >= 1
        #define ERROR(fmt, ...) {\
                time_t t = time(NULL);\
                struct tm *t2 = localtime(&t);\
                fprintf(stderr, "%02d:%02d:%02d [E] " fmt, \
                        t2->tm_hour, t2->tm_min, t2->tm_sec, ##__VA_ARGS__);}
#else 
        #define ERROR(fmt, ...) 
#endif 