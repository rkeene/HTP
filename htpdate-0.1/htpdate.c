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

int main(int argc, char *argv[]) {
	unsigned int argind = 0, timeind = 0;
	unsigned int totaltimeservers = 0;
	struct timeserver_st timeservers[128];
	time_t newtime = 0;
	char *host = NULL, *portstr = NULL;
	int port = -1;

	if ( argc <= 1 ) {
		printf ("Usage: getdate <host> [<host> [<host> [...]]]\n");
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

	newtime = htp_calctime(timeservers, totaltimeservers, NULL, 0);

	if (newtime < 0) {
		return(EXIT_FAILURE);
	}

	set_clock(newtime);

	return(EXIT_SUCCESS);
}
