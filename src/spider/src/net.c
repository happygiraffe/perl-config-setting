/*********************************************************************
 * net.c
 *
 * Networking subroutines for Spider.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: net.c,v 1.4 2000/01/14 23:26:29 dom Exp $";

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
#include <string.h>
#include <syslog.h>

#include <nev.h>

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

/***********************************************************************
 * getrname: return remote host name.  also fills in addr.
 */
char *
getrname(int fd, struct sockaddr *addr, int *addrlen)
{
    struct hostent *hp;
    struct sockaddr_in *so = (struct sockaddr_in *)addr;

    getpeername (fd, addr, addrlen);
    hp = gethostbyaddr ((char *)&so->sin_addr.s_addr,
			sizeof so->sin_addr.s_addr, AF_INET);
    return strdup (hp ? hp->h_name : inet_ntoa(so->sin_addr));
}

/***********************************************************************
 * spider_accept: set up a new connection.
 */
void
spider_accept(int fd, void *data)
{
    char *c;
    struct sockaddr_in client;
    int client_len;

    c = getrname (fd, (struct sockaddr *)&client, &client_len);

    /* XXX Check for existing connection? */

    syslog (LOG_INFO, "connect from %s fd %d", c, fd);

    /* XXX global vars must die */
    sender = receiver = fd;

    /* XXX make creator function for Conn */
    SENDER_CONN = malloc (sizeof(Conn));
    if (SENDER_CONN == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    SENDER_CHAN = fdopen (sender, "r+");
    setvbuf(SENDER_CHAN, NULL, _IOLBF, 0);
    SENDER_CONN->type = user;
    SENDER_CONN->det.usr.host = c;
    nev_watch(fd, spider_read);
    /* XXX shouldn't do manual output. */
    fputs(INITIAL_GREETING "\r\n", SENDER_CHAN);
    SENDER_CONN->state = s_init;

    return;
}

/***********************************************************************
 * spider_read: process new data that arrives on an fd.  if possible,
 * dispatch to the correct destination.
 */
void
spider_read(int fd, void *data)
{
    sender = receiver = fd;

    if (conn_read(SENDER_CONN) == 0) {
	term_conn();
    }

/*     if (whole_msg(SENDER_CONN)) { */
/* 	conn_parse_input(SENDER_CONN); */
/* 	receiver = find_dest(SENDER_CONN); */
/* 	conn_send_msg(SENDER_CONN);	     */
/*     } */

    return;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */

