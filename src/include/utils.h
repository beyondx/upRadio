#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <assert.h>

#define   CHK_EXIT(expr)	\
	do {	if ((expr) < 0) {\
			fprintf(stderr, "[ERROR](%s:%d:%s)\"%s\" failed,(%s)\n", __FILE__, __LINE__, __FUNCTION__, #expr, strerror(errno));\
		exit(-1); }\
	    } while (0)

#define   CHK_RET_EXIT(iret, expr)	\
	do {	if ((iret = (expr)) < 0) {\
			fprintf(stderr, "[ERROR](%s:%d:%s) %s failed,(%s)\n", __FILE__, __LINE__, __FUNCTION__, #expr, strerror(errno));\
		exit(-1); }\
	    } while (0)


#ifdef _DEBUG
#define   DEBUG(fmt, args...)	\
	fprintf(stderr, "[DEBUG](%s:%d:%s)"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define   SYSLOG(prio, fmt, args...)	\
	syslog(prio, "(%s:%d:%s)"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define  DEBUG(fmt, args...)	
#define   SYSLOG(prio, fmt, args...) syslog(prio, fmt, ##args)
#endif
