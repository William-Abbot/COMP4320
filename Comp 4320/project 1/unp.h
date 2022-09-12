/* include unp.h */

#ifndef	__unp_h
#define	__unp_h
#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */

#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include  <cmath>
#include  <cstring>

#define	SERV_PORT	8080	/* TCP and UDP */

#define	MAXLINE		10000	/* max text line length */
#define	BUFFSIZE	10000	/* buffer size for reads and writes */
#endif	/* __unp_h */
