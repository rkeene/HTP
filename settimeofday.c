#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "win32.h"

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

/*
 * settimeofday():
 *    Set the system clock.
 * Works on: Win32 (mingw32)
 */
int settimeofday(const struct timeval *tv , void *tz) {
	int retval = -1;
#ifdef _USE_WIN32_
	struct tm *lt = NULL;
	SYSTEMTIME st;

	lt = localtime(&(tv->tv_sec));

	st.wYear = lt->tm_year + 1900;
	st.wMonth = lt->tm_mon;
	st.wDay = lt->tm_mday;
	st.wHour = lt->tm_hour;
	st.wMinute = lt->tm_min;
	st.wSecond = lt->tm_sec;
	st.wMilliseconds = tv->tv_usec / 1000;

	if (SetSystemTime(&st)) {
		retval = 0;
	}
#endif
	return(retval);
}
