#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "win32.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#endif

#include "htp.h"

int main(int argc, char *argv[]) {
	unsigned int argind = 0, timeind = 0;
	unsigned int totaltimeservers = 0;
	struct timeserver_st timeservers[128];
	time_t newtime = 0;
	char *host = NULL, *portstr = NULL;
	char *proxyhost = NULL, *proxyportstr = NULL;
	int port = -1, proxyport = -1;

	htp_init();

#ifdef HAVE_OPENLOG
	openlog("htpdate", LOG_PID, LOG_USER);
#endif

	if ( argc <= 1 ) {
		printf("htpdate version " PACKAGE_VERSION "\n");
		printf("Usage: htpdate <host> [<host> [<host> [...]]]\n");
		printf("  Where each `host' is in the format of:\n");
		printf("       hostname[:port]\n");
		return(EXIT_FAILURE);
	}

	/* Process each argument as a host:port pair. */
	for (argind = 1; argind < (unsigned int) argc; argind++) {
		if (argind >= (sizeof(timeservers)/sizeof(timeservers[0]))) {
			break;
		}

		host = argv[argind];

		portstr = strchr(host, ':');
		if (portstr != NULL) {
			/* Here, we modify argv.. good idea? */
			portstr[0] = '\0';

			portstr++;
			port = atoi(portstr);
			if (port <= 0) {
				fprintf(stderr, "Invalid port: %s.\n", portstr);
				continue;
			}
		} else {
			port = 80;
		}

		timeservers[timeind].host = host;
		timeservers[timeind].port = port;

		timeind++;
	}

	totaltimeservers = timeind;

	if (totaltimeservers == 0) {
		fprintf(stderr, "No servers specified.\n");
		return(EXIT_FAILURE);
	}

	proxyhost = getenv("http_proxy");
	if (proxyhost == NULL) {
		proxyhost = getenv("HTTP_PROXY");
	}

	if (proxyhost != NULL) {
		proxyportstr = strrchr(proxyhost, ':');
		if (proxyportstr != NULL) {
			*proxyportstr = '\0';
			proxyportstr++;
			proxyport = atoi(proxyportstr);
			if (proxyport <= 0) {
				printf("Illegal proxy port: %s.\n", proxyportstr);
				return(EXIT_FAILURE);
			}
		}
	}

	newtime = htp_calctime(timeservers, totaltimeservers, proxyhost, proxyport);

	if (newtime < 0) {
		return(EXIT_FAILURE);
	}

	set_clock(newtime, 0);

	printf("new time: %s", asctime(localtime(&newtime)));

	return(EXIT_SUCCESS);
}
