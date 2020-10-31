
#ifndef _TRACELOG_H
#define _TRACELOG_H



#ifdef  __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include <errno.h>
#include <assert.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>



#define LOG_ERR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4


void tracelog(char* filename, int level,char *str,...);


void trimleft(char *str);
void trimright(char *str);
void trimall(char *str);
//int mkdir_recursive( char* path );

#ifdef  __cplusplus
}
#endif
#endif
