#ifndef _CONFIG_H
#define _CONFIG_H

/***************** You may have to change these: *****************/

/* What facility of syslog Spider uses */
#define SPIDER_FACIL LOG_LOCAL0

/* A standard limit throughout this program  */
#define LARGE_BUF 	2048	

/* XXX Fix this, to be determined from RCS versions instead */
#define VERSION		"1.0.2"

/***************** Configure should take care of these: *****************/
@TOP@
@BOTTOM@

#endif /* _CONFIG_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
