#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifndef S_SPLINT_S
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif /* S_SPLINT_S */
 
#include "gpsd.h"

/*@ -branchstate */
int udp_open(struct gps_context_t *context, const char *port)
/* open a connection to a UDP server */
{
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */
    unsigned short broadcastPort;     /* Port */

    broadcastPort = atoi(port);

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
      return NL_NOSOCK;

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)))
      return 0;

    return sock;
}

/*@ +branchstate */

void udp_report(struct gps_device_t *session)
/* may be time to ship a usage report to the UDP server */
{
    /*
     * 10 is an arbitrary number, the point is to have gotten several good
     * fixes before reporting usage to our UDP server.
     */
    if (session->context->fixcnt > 10 && !session->context->sentdgps) {
	session->context->sentdgps = true;
	if (session->context->dsock > -1) {
	    char buf[BUFSIZ];
	    if (write(session->context->dsock, buf, strlen(buf)) ==
		(ssize_t) strlen(buf))
		gpsd_report(LOG_IO, "=> udp %s\n", buf);
	    else
		gpsd_report(LOG_IO, "write to udp FAILED\n");
	}
    }
}
