#include "gettimeofday.h"

int gettimeofday(struct timeval *tv, void *tz) {
	tv->tv_sec = time(NULL);
	tv->tv_usec = 0;
	return(0);
}
