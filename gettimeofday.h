#ifndef _RSK_GETTIMEOFDAY_H
#define _RSK_GETTIMEOFDAY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

int gettimeofday(struct timeval *tv, void *tz);

#endif
