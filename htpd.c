#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "win32.h"

#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
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
#ifdef HAVE_LIBCONFIG
#include <libconfig.h>
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

static struct timeserver_st timeservers[128];
static unsigned int timeind = 0;
char *http_proxy = NULL;
uint16_t http_proxy_port = 0;

static void daemonize(void) {
	pid_t pid;

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	setsid();

	pid = fork();

	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	return;
}

int set_proxy(const char *shortvar, const char *var, const char *arguments, const char *value, lc_flags_t flags, void *extra) {
	char *host = NULL, *portstr = NULL;
	char *real_value = NULL, *end_ptr = NULL;
	size_t hostlen = 0;
	int port = -1;

	if (value == NULL) {
		fprintf(stderr, "Error: Argument required.\n");
		return(LC_CBRET_ERROR);
	}

	real_value = strstr(value, "://");
	if (real_value != NULL) {
		value = real_value + 3;
	}

	hostlen = strlen(value) + 1;
	if (hostlen == 1) {
		fprintf(stderr, "Error: Argument required.\n");
		return(LC_CBRET_ERROR);
	}

	host = malloc(hostlen);
	if (host == NULL) {
		fprintf(stderr, "Error: Could not allocate memory.\n");
		return(LC_CBRET_ERROR);
	}
	memcpy(host, value, hostlen);

	end_ptr = strchr(host, '/');
	if (end_ptr != NULL) {
		*end_ptr = '\0';
	}

	portstr = strchr(host, ':');
	if (portstr != NULL) {
		portstr[0] = '\0';

		portstr++;
		port = atoi(portstr);
		if (port <= 0) {
			fprintf(stderr, "Error: Invalid port specified: %s:%s.\n", host, portstr);
			free(host);
			return(LC_CBRET_ERROR);
		}
	} else {
		port = 8080;
	}

	http_proxy = host;
	http_proxy_port = port;

	return(LC_CBRET_OKAY);
}

int add_server(const char *shortvar, const char *var, const char *arguments, const char *value, lc_flags_t flags, void *extra) {
	char *host = NULL, *portstr = NULL;
	char *real_value = NULL, *end_ptr = NULL;
	size_t hostlen = 0;
	int port = -1;
	unsigned int  timechk = 0;

	if (value == NULL) {
		fprintf(stderr, "Error: Argument required.\n");
		return(LC_CBRET_ERROR);
	}

	real_value = strstr(value, "://");
	if (real_value != NULL) {
		value = real_value + 3;
	}

	hostlen = strlen(value) + 1;
	if (hostlen == 1) {
		fprintf(stderr, "Error: Argument required.\n");
		return(LC_CBRET_ERROR);
	}

	host = malloc(hostlen);
	if (host == NULL) {
		fprintf(stderr, "Error: Could not allocate memory.\n");
		return(LC_CBRET_ERROR);
	}
	memcpy(host, value, hostlen);

	end_ptr = strchr(host, '/');
	if (end_ptr != NULL) {
		*end_ptr = '\0';
	}

	portstr = strchr(host, ':');
	if (portstr != NULL) {
		portstr[0] = '\0';

		portstr++;
		port = atoi(portstr);
		if (port <= 0) {
			fprintf(stderr, "Error: Invalid port specified: %s:%s.\n", host, portstr);
			free(host);
			return(LC_CBRET_ERROR);
		}
	} else {
		port = 80;
	}

	for (timechk = 0; timechk < timeind; timechk++) {
		if (strcasecmp(timeservers[timechk].host, host) == 0 && timeservers[timechk].port == port) {
			return(LC_CBRET_OKAY);
		}
	}

	timeservers[timeind].host = host;
	timeservers[timeind].port = port;

	timeind++;

	return(LC_CBRET_OKAY);
}

int print_help(const char *shortvar, const char *var, const char *arguments, const char *value, lc_flags_t flags, void *extra) {
	printf("Usage: htpd [-P <proxy>] [-H <host> [-H <host> [...]]]\n");
	printf("   Where each `host' is in format of:\n");
	printf("       hostname[:port]\n");

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	unsigned int totaltimeservers = 0;
	unsigned int maxsleeptime = 43200, minsleeptime = 4096, sleeptime = minsleeptime;
	double delta;
	time_t newtime = 0;
	int lc_p_ret = 0;

	htp_init();

	lc_register_callback("Host", 'H', LC_VAR_STRING, add_server, NULL);
	lc_register_callback("ProxyHost", 'P', LC_VAR_STRING, set_proxy, NULL);
	lc_register_callback(NULL, 'h', LC_VAR_NONE, print_help, NULL);
	lc_p_ret = lc_process(argc, argv, "htpd", LC_CONF_SPACE, SYSCONFDIR "/htpd.conf");
	if (lc_p_ret < 0) {
		fprintf(stderr, "Error processing configuration information: %s.\n", lc_geterrstr());
		return(EXIT_FAILURE);
	}

	totaltimeservers = timeind;

	if (totaltimeservers == 0) {
		fprintf(stderr, "No servers specified.\n");
		return(EXIT_FAILURE);
	}

	daemonize();

	openlog("htpd", LOG_PID, LOG_DAEMON);

	while (1) {
		newtime = htp_calctime(timeservers, totaltimeservers, http_proxy, http_proxy_port);

		if (newtime >= 0) {
			delta = set_clock(newtime, 1);
		} else {
			delta = 0.0;
		}

		/*
		 *  Divide sleep time by 0.5 if change == 0  (check less frequently)
		 *  Divide sleep time by 2   if change == 10 (check more frequently)
		 *  Divide sleep time by 1   if change == 5  (check as frequently)
		 */
		delta = fabs(delta);
		sleeptime = ((double) sleeptime) / ((1.0 + (delta / 10.0)) / (2.0 - (delta / 10.0)));

		if (sleeptime < minsleeptime) {
			sleeptime = minsleeptime;
		}
		if (sleeptime > maxsleeptime) {
			sleeptime = maxsleeptime;
		}

		syslog(LOG_INFO, "Sleeping for %u seconds.", sleeptime);

		sleep(sleeptime);
	}

	return(EXIT_SUCCESS);
}
