/*********************************************************************
 * net.c
 *
 * Networking subroutines for Spider.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: net.c,v 1.2 2000/01/06 22:01:41 dom Exp $";

#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "spider.h"

/*********************************************************************
 * spider_listen()
 *
 * Return a listening socket on the correct port number.
 */
int
spider_listen(int port)
{
    int			l_sock;
    Bool		reuse = true;
    struct sockaddr_in	server;

    l_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock < 0) {
        syslog(LOG_ERR, "socket(2): %m");
        exit(1);
    }
    setsockopt(l_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse,
	       sizeof(reuse));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = ntohs(port);
    if (bind(l_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        syslog(LOG_ERR, "bind(2): %m");
        exit(1);
    }
    listen(l_sock, 5);
    return (l_sock);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */

