/*
getdate v0.1

Extract date/time stamp from web server response
This program works with the most common date/time stamps format specified in
RFC 822 updated by RFC 1123.

Example usage. 
Synchronize local workstation with time offered by remote web server

	~> date -s "`getdate www.microsoft.com`"

*/


#include <stdio.h>		/* Needed for printf() */
#include <string.h>		/* Needed for strstr() and strcpy() */
#include <netinet/in.h>		/* Needed for internet address structure. */
#include <sys/socket.h>		/* Needed for socket(), bind(), etc... */
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>		/* Needed for exit() */
#include <stdlib.h>		/* Needed for atoi() */

#define  BUFFER 		2048	/* Allocate buffer for requests and responses */
#define  HEAD 			"HEAD / HTTP/1.0\r\nPragma: no-cache\r\nCache-Control: Max-age=0\r\n\r\n"
#define  DATE_LINE		"Date: "

/*	Prototype functions */
void PrintErrorAndExit	( char *msg, short code );

time_t mktime_from_rfc1111(const char *date) {
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

time_t get_gmtoffset(void) {
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

int main(int argc, char *argv[]) {

	unsigned int         server_s;		/* Server socket descriptor */
	struct sockaddr_in   server_addr;	/* Server Internet address */
	char                 out_buf[BUFFER];	/* Output buffer for HEAD request */
	char                 in_buf[BUFFER];	/* Input buffer for response */
	int 		     retcode;		/* Return code */

	char			remote_time[32];
	char			*host, *pdate;
	int			port;
	struct			hostent	*hostinfo;
	time_t			remote_timeval = -1, gmtoffset = -1;

	gmtoffset = get_gmtoffset();

	if ( argc <= 1 ) {
		printf ("Usage: getdate <host> [<port>]\n");
		exit(0);
	} else {
		host = argv[1];
		hostinfo = gethostbyname(host);
		if (!hostinfo) {
			PrintErrorAndExit( "Host not found", 1 );
		}
		/* port argument */
		if ( argv[2] ) {
			port = atoi ( argv[2] );
			if (port <= 0) {
				PrintErrorAndExit( "Invalid port specified", EXIT_FAILURE );
			}
		} else {
			port = 80;
		}
	}

	/* Create a socket */
	server_s = socket(AF_INET, SOCK_STREAM, 0);

	/* The web server socket's address information */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;

	/* Do a connect (connect() blocks) */
	retcode = connect(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if ( retcode < 0 )
	{
		PrintErrorAndExit( "Connection to host failed", 2 );
	}

	/* Send the most simple HEAD request */
	strcpy(out_buf, HEAD);
	send(server_s, out_buf, strlen(out_buf), 0);

	/* Receive data from the web server */
	/* The return code from recv() is the number of bytes received */
	retcode = recv(server_s, in_buf, BUFFER, 0);
	if ( retcode < 0 ) {
		PrintErrorAndExit("No data received", 3);
	}

	if ((pdate = (char *) strstr(in_buf, DATE_LINE)) != NULL) {
		strncpy(remote_time, pdate +  6, 29);
		remote_timeval = mktime_from_rfc1111(remote_time);
		if (remote_timeval >= 0) {
			set_clock(remote_timeval + gmtoffset);
		}
		printf ("%i: %s\n", (int) (remote_timeval + gmtoffset), remote_time);
	} else {
		PrintErrorAndExit("Date couldn't be retreived", 4);
	}

	close(server_s);

	return(EXIT_SUCCESS);
}

void PrintErrorAndExit ( char *msg, short code ) {
	fprintf (stderr, "%s\n", msg);
	exit( code );
}
