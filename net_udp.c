/* 
 * net_udp.c -- gather and dispatch broadcast UDP packets
 */
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
int udp_open(struct gps_device_t *device, const char *bcastport)
/* listen for open a connection to a UDP server */
{
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */
    unsigned short broadcastPort;     /* Port */

    broadcastPort = atoi(bcastport);

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

void udp_report(struct gps_context_t *context,
		   struct gps_device_t *gps,
		   struct gps_device_t *dgpsip)
{
    if (context->fixcnt > 10 && !dgpsip->dgpsip.reported) {
	dgpsip->dgpsip.reported = true;
	if (dgpsip->gpsdata.gps_fd > -1) {
	    char buf[BUFSIZ];
	    (void)snprintf(buf, sizeof(buf), "R %0.8f %0.8f %0.2f\r\n",
			   gps->gpsdata.fix.latitude,
			   gps->gpsdata.fix.longitude,
			   gps->gpsdata.fix.altitude);
	    if (write(dgpsip->gpsdata.gps_fd, buf, strlen(buf)) ==
		(ssize_t) strlen(buf))
		gpsd_report(LOG_IO, "=> dgps %s\n", buf);
	    else
		gpsd_report(LOG_IO, "write to dgps FAILED\n");
	}
    }
}

