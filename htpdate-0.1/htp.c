#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "htp.h"

static time_t mktime_from_rfc2616(const char *date) {
	struct tm timeinfo;
	time_t currtime = -1;
	char *monthinfo[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	unsigned int i;

	/* Fill in values from RFC-compliant string. */
	timeinfo.tm_mday = atoi(date + 5);
	timeinfo.tm_year = atoi(date + 12) - 1900;
	timeinfo.tm_hour = atoi(date + 17);
	timeinfo.tm_min = atoi(date + 20);
	timeinfo.tm_sec = atoi(date + 23);

	/* Determine month. */
	timeinfo.tm_mon = -1;
	for (i = 0; i < (sizeof(monthinfo) / sizeof(monthinfo[0])); i++) {
		if (strncmp(date + 8, monthinfo[i], 3) == 0) {
			timeinfo.tm_mon = i;
			break;
		}
	}

	/* Create GMT value */
	currtime = mktime(&timeinfo);

	return(currtime);
}

int set_clock(time_t timeval) {
	struct timeval tvinfo;
	int retval = -1;

	tvinfo.tv_sec = timeval;
	tvinfo.tv_usec = 0;

	retval = settimeofday(&tvinfo, NULL);

	return(retval);
}

/* Calculate the difference between localtime and GMT/UT/UTC. */
static time_t get_gmtoffset(void) {
	static time_t retval = -1;
	time_t dummytime = 604800;
	struct tm *timeinfo = NULL;

	/* Return saved value. */
	if (retval != -1) {
		return(retval);
	}

	timeinfo = localtime(&dummytime);
	if (timeinfo == NULL) {
		return(-1);
	}
	if (timeinfo->tm_hour == 0) {
		timeinfo->tm_hour = 24;
	}
	retval = (timeinfo->tm_hour * 3600) + (timeinfo->tm_min * 60);

	timeinfo = gmtime(&dummytime);
	if (timeinfo == NULL) {
		retval = -1;
		return(-1);
	}
	if (timeinfo->tm_hour == 0) {
		timeinfo->tm_hour = 24;
	}
	retval -= (timeinfo->tm_hour * 3600) + (timeinfo->tm_min * 60);

	return(retval);
}

static time_t get_server_time(const char *host, unsigned int port, const char *proxyhost, unsigned int proxyport) {
	unsigned int         server_s;		/* Server socket descriptor */
	struct sockaddr_in   server_addr;	/* Server Internet address */
	char                 out_buf[2048];	/* Output buffer for HEAD request */
	char                 in_buf[2048];	/* Input buffer for response */
	int 		     retcode;		/* Return code */

	char			remote_time[32] = {0};
	char			*pdate;
	const char		*connect_host = NULL;
	unsigned int		connect_port = 0;
	struct			hostent	*hostinfo;
	time_t			remote_timeval = -1, retval = -1;

	if (proxyhost == NULL) {
		connect_host = host;
		connect_port = port;
	} else {
		connect_host = proxyhost;
		connect_port = proxyport;
	}

	hostinfo = gethostbyname(connect_host);
	if (!hostinfo) {
		return(-1);
	}

	/* Create a socket */
	server_s = socket(AF_INET, SOCK_STREAM, 0);

	/* The web server socket's address information */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;

	/* Do a connect (connect() blocks) */
	retcode = connect(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if ( retcode < 0 ) {
		return(-1);
	}

	/* Send the most simple HEAD request */
	if (proxyhost == NULL) {
		snprintf(out_buf, sizeof(out_buf), "HEAD / HTTP/1.0\r\nPramga: no-cache\r\nCache-Control: Max-age=0\r\n\r\n");
	} else {
		snprintf(out_buf, sizeof(out_buf), "HEAD http://%s:%i/ HTTP/1.0\r\nPramga: no-cache\r\nCache-Control: Max-age=0\r\n\r\n", host, port);
	}
	send(server_s, out_buf, strlen(out_buf), 0);

	/* Receive data from the web server */
	/* The return code from recv() is the number of bytes received */
	retcode = recv(server_s, in_buf, sizeof(in_buf), 0);
	if ( retcode <= 0 ) {
		close(server_s);
		return(-1);
	}

	if ((pdate = (char *) strstr(in_buf, "Date: ")) != NULL) {
		strncpy(remote_time, pdate +  6, 29);

		remote_timeval = mktime_from_rfc2616(remote_time);

		retval = remote_timeval;
	} else {
	}

	close(server_s);

	return(retval);
}

int compare_time(const void *a, const void *b) {
	const time_t *tmpptr = NULL;
	time_t  timea = 0, timeb = 0;

	tmpptr = a;
	timea = *tmpptr;
	tmpptr = b;
	timeb = *tmpptr;

	if (timea < timeb) {
		return(-1);
	}
	if (timea > timeb) {
		return(1);
	}

	return(0);
}

time_t htp_calctime(struct timeserver_st *timeservers, unsigned int totaltimeservers, const char *proxyhost, unsigned int proxyport) {
	unsigned int timeind = 0, meantimeind = 0;
	unsigned int totaltimes = 0, totalstddevtimes = 0;
	unsigned int port = 0;
	time_t timevals[128] = {0}, timeval = -1;
	time_t mintime, maxtime = 0, avgtime = 0, meantime = 0;
	time_t stddevavgtime = 0;
	time_t tmptime = 0;
	time_t newtimeval = 0;
	time_t gmtoffset = 0;
	time_t start_time = 0, offset_time = 0;
	char *host = NULL;
	int stddevtimes = 0;

	gmtoffset = get_gmtoffset();

	start_time = time(NULL);

	mintime = start_time * 2;

	for (timeind = 0; timeind < totaltimeservers; timeind++) {	
		host = timeservers[timeind].host;
		port = timeservers[timeind].port;
		timeval = get_server_time(host, port, proxyhost, proxyport);
		if (timeval < 0) {
			continue;
		}

		/* Remove the amount of time spent so far. */
		offset_time = time(NULL) - start_time;

		timevals[timeind] = timeval - offset_time;
	}

	totaltimes = timeind;
	meantimeind = totaltimes / 2;

	if (totaltimes == 0) {
		return(-1);
	}

	/* If there's only one time, don't bother with anything else. */
	if (totaltimes == 1) {
		newtimeval = timevals[0];
	} else {
		qsort(&timevals, totaltimes, sizeof(timevals[0]), compare_time);
		mintime = timevals[0];
		maxtime = timevals[totaltimes - 1];
		meantime = timevals[meantimeind];

		for (timeind = 0; timeind < totaltimes; timeind++) {
			tmptime = timevals[timeind];
			avgtime += tmptime - mintime; /* This trickery is to avoid overflowing the type. */
		}
		avgtime /= totaltimes;
		avgtime += mintime; /* Part of the same silly trickery. */

		/* If there are only 2 values, we can do no more. */
		if (totaltimes == 2) {
			newtimeval = avgtime;
		} else {
//			printf("mintime = %li\n", mintime);
//			printf("maxtime = %li\n", maxtime);
//			printf("avgtime = %li\n", avgtime);
//			printf("meantime = %li\n", meantime);

			stddevtimes = ((double) totaltimes) * 0.34; /* One standard deviation, +/- 34% about the mean. */

			for (timeind = (meantimeind - stddevtimes); timeind < (meantimeind + stddevtimes); timeind++) {
				tmptime = timevals[timeind];
				stddevavgtime += tmptime - mintime;
				totalstddevtimes++;
			}
			stddevavgtime /= totalstddevtimes;
			stddevavgtime += mintime;

//			printf("stddevavgtime: %li (currtime = %li)\n", stddevavgtime, time(NULL));

			newtimeval = stddevavgtime;
		}
	}

	if (newtimeval <= 0) {
//		fprintf(stderr, "Failed to calculate new time.\n");
		return(-1);
	}

	offset_time = time(NULL) - start_time;

	/* Add back the time this process took. */
	newtimeval += offset_time;

	printf("new time: %li + %li\n", newtimeval, gmtoffset);

	return(newtimeval + gmtoffset);
}
