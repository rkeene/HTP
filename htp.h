#ifndef _RSK_HTP_H
#define _RSK_HTP_H

struct timeserver_st {
        char *host;
        unsigned int port;
};

int set_clock(time_t timeval);
time_t htp_calctime(struct timeserver_st *timeservers, unsigned int totaltimeservers, const char *proxyhost, unsigned int proxyport);

#endif
