/*********************************************************************
 * nev.h
 *
 * Network EVents library, include file.
 *
 * Copyright 2000 Dominic Mitchell (dom@myrddin.demon.co.uk)
 *
 * @(#) $Id: nev.h,v 1.2 2000/01/13 00:21:59 dom Exp $
 */

#ifndef _NEV_H_
#define _NEV_H 1

/* TYPEDEFS */

typedef void (*nev_cbfn)(int, void *); /* callback function */

/* PROTOTYPES */

/* nev.c */

/* nev_init: initialise an instance of the library. */
int	nev_init(void *data);
/* nev_listen: set up a listener, and a callback function for
 * connections accepted on that socket.  returns socket or -1. */
int	nev_listen(char *addr, char *port,
		   nev_cbfn newconn_fn,
		   void *data);
/* nev_watch_fd: start looking for input (or close) on an fd.
 * returns -1 on error. */
int	nev_watch(int fd,
		  nev_cbfn read_fn,
		  nev_cbfn close_fn);
/* nev_main_loop: iterate over the list of fd's we know about. */
void	nev_main_loop(void);
/* nev_unwatch_fd: stop looking at fd.  will close it. */
void	nev_unwatch(int fd);

#endif /* _NEV_H_ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
