/***********************************************************************
 * nev.c
 *
 * Network EVents library.
 *
 * TODO:
 *
 * . Should use an array, not a list for fd_list.  Then we can use the
 *   size of the array as Nev.nfds.
 * . Must get signals and select right.
 * . Should move list bits out into separate functions.
 * . Must adapt so it can use poll as well.
 * . Should make listen backlog configurable.
 * . Should have option to link in libwrap stuff.
 *
 * Copyright 2000 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>		/* required by use of bzero in FD_ZERO... */
#include <unistd.h>

#include "nev.h"

static const char rcsid[]="@(#) $Id: nev.c,v 1.5 2000/01/13 01:16:23 dom Exp $";

/* PROTOTYPES */

static void nev_accept();

/* GLOBALS */

struct fd_info 
{
    struct fd_info *next;
    int fd;
    nev_cbfn user_read;
};

static struct 
{
    void	*data;		/* for use by callbacks */
    nev_cbfn	user_accept;	/* what to call upon accept */
    struct fd_info *fd_list;	/* a list of details about fd's */
    int		nfds;		/* how many fd's in rset */
    fd_set	rset;		/* fd's to watch for reads */
} Nev;

/*
 * Expected flow of control:
 *
 * nev_init(&globalvar)
 * nev_listen(NULL, "4321", my_accept);
 * nev_main_loop();
 *
 * Then, we get an incoming connection.
 *
 * my_accept(int fd, void *data) {
 *     nev_watch(fd, my_read_data);
 * }
 */

/***********************************************************************
 * nev_init: initialise an instance of the library.
 */
int
nev_init(void *data)
{
    Nev.data = data;
    Nev.user_accept = NULL;
    Nev.fd_list = NULL;
    Nev.nfds = 0;
    FD_ZERO(&Nev.rset);

    return 1;
}

/***********************************************************************
 * nev_listen: set up a listener, and a callback function for
 * connections accepted on that socket.  returns socket or -1.
 */
int
nev_listen(char *addr, char *port, nev_cbfn newconn_fn, void *data)
{
    int			lsock;
    int			reuse = 1;
    struct sockaddr_in	lsaddr;

    Nev.user_accept = newconn_fn;

    lsock = socket (AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
	return -1;

    /* allow us to restart immediately. */
    setsockopt (lsock, SOL_SOCKET, SO_REUSEADDR,
		(void *)&reuse, sizeof(reuse));

    lsaddr.sin_family = AF_INET;
    lsaddr.sin_addr.s_addr = addr == NULL ? INADDR_ANY : inet_addr(addr);
    lsaddr.sin_port = ntohs(atoi(port));

    if (bind (lsock, (struct sockaddr *)&lsaddr, sizeof(lsaddr)) < 0)
	return -1;

    listen (lsock, 5);

    /* start watching lsock now. */
    nev_watch (lsock, nev_accept);

    return lsock;
}

/***********************************************************************
 * nev_accept: pass on the new fd down to the user's functions.
 */
static void
nev_accept (int fd, void *data)
{
    int newfd;
    int addrlen;
    struct sockaddr_in addr;

    newfd = accept(fd, (struct sockaddr *)&addr, &addrlen);
    if (newfd < 0)
	return;

    (*Nev.user_accept)(newfd, data);
    return;
}

/***********************************************************************
 * nev_watch: start looking for input on an fd.
 */
int
nev_watch(int fd, nev_cbfn read_fn)
{
    struct fd_info *f;

    f = malloc (sizeof(struct fd_info));
    if (f == NULL)
	return -1;

    f->next = Nev.fd_list;
    f->fd = fd;
    f->user_read = read_fn;
    Nev.fd_list = f;

    FD_SET(fd, &Nev.rset);
    Nev.nfds = fd > Nev.nfds ? fd : Nev.nfds;

    return fd;
}

/***********************************************************************
 * nev_main_loop: iterate over the list of fd's we know about.
 */
void
nev_main_loop(void)
{
    fd_set readers;
    int i;
    struct fd_info *f;

    while (1) {
	memmove (&readers, &Nev.rset, sizeof Nev.rset);
	i = select (Nev.nfds+1, &readers, (fd_set *)NULL, (fd_set *)NULL,
		    (struct timeval *)NULL);
	if (i < 0) {
	    ;			/* XXX panic properly */
	}
	for (f = Nev.fd_list; f != NULL; f = f->next) {
	    if (FD_ISSET(f->fd, &readers))
		f->user_read(f->fd, Nev.data);
	}
    }
    return;
}

/***********************************************************************
 * nev_unwatch: stop watching for input data on fd and close it.
 */
void
nev_unwatch(int fd)
{
    struct fd_info *f, *prev;

    prev = NULL;
    for (f = Nev.fd_list; f != NULL; f = f->next) {
	if (f->fd == fd) {
	    if (prev == NULL)
		Nev.fd_list = f->next;
	    else
		prev->next = f->next;
	    free (f);		    
	}
    }

    /* Adjust the select parameters */
    FD_CLR(fd, &Nev.rset);
    if (fd == Nev.nfds)
	Nev.nfds--;

    close (fd);
    return;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
