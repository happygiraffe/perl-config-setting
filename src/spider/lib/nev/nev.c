/*********************************************************************
 * nev.c
 *
 * Network EVents library.
 *
 * Copyright 2000 Dominic Mitchell (dom@myrddin.demon.co.uk)
 *
 * @(#) $Id: nev.c,v 1.2 2000/01/12 21:41:32 dom Exp $
 */

#include <sys/types.h>
#include <unistd.h>

#include "nev.h"

static const char rcsid[]="@(#) $Id: nev.c,v 1.2 2000/01/12 21:41:32 dom Exp $";

int
nev_listen(char *addr, char *port, nev_cbfn newconn_fn, void *data)
{
    return 0;
}

int
nev_watch(int fd, nev_cbfn read_fn, nev_cbfn close_fn)
{
    return 0;
}

void
nev_main_loop(void)
{
    return;
}

void
nev_unwatch(int fd)
{
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
