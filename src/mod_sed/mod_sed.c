/*-
 * Copyright (c) 2001 Dominic Mitchell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Apache interface.
 */

#ifndef lint
static const char rcsid[] = "@(#) $Id: mod_sed.c,v 1.2 2001/06/12 23:52:25 dom Exp $";
#endif /* lint */

#include <sys/types.h>
#include <sys/stat.h>    
#include <setjmp.h>    
#include <stdarg.h>    

/* Apache includes */
#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_log.h>
#include <http_main.h>
#include <http_protocol.h>
#include <http_request.h>

#include "defs.h"
#include "extern.h"

/* predeclare */
module MODULE_VAR_EXPORT sed_module;

/* For use elsewhere in this program */
request_rec *s_r;

/* For use by err{x,}. */
jmp_buf	err_jmp_buf;

/*------------------------------------------------------------------
 * Configuration implementation.
 */

typedef struct {
	char	*program;		/* -e or filename */
	int	 opt_n;		/* -n */
	int	 opt_E;		/* -E */
	table	*pmap;
} sed_dir_cf;

static void *
sed_mk_dir_cf(pool *p, char *path)
{
	sed_dir_cf *cfg = (sed_dir_cf *)ap_pcalloc(p, sizeof(sed_dir_cf));
	cfg->pmap = ap_make_table(p, 10);
	return (void *)cfg;
}

static void *
sed_merge_dir_cf(pool *p, void *basev, void *addv)
{
	sed_dir_cf *new = (sed_dir_cf *)ap_pcalloc(p, sizeof(sed_dir_cf));
	sed_dir_cf *base = (sed_dir_cf *)basev;
	sed_dir_cf *add = (sed_dir_cf *)addv;

	new->program	= add->program	? add->program	: base->program;
	new->opt_n	= add->opt_n	? add->opt_n	: base->opt_n;
	new->opt_E	= add->opt_E	? add->opt_E	: base->opt_E;
	new->pmap	= add->pmap	? add->pmap	: base->pmap;

	return (void *)new;
}

static const char *
sed_cmd_program(cmd_parms *parms, void *mconfig, char *program)
{
	sed_dir_cf *cfg = (sed_dir_cf *)mconfig;
	cfg->program = (char *)ap_pstrdup(parms->pool, program);
	return NULL;
}

static const char *
sed_cmd_opt_n(cmd_parms *parms, void *mconfig, int flag)
{
	sed_dir_cf *cfg = (sed_dir_cf *)mconfig;
	cfg->opt_n = flag;
	return NULL;
}

static const char *
sed_cmd_opt_E(cmd_parms *parms, void *mconfig, int flag)
{
	sed_dir_cf *cfg = (sed_dir_cf *)mconfig;
	cfg->opt_E = flag;
	return NULL;
}

static const char *
sed_cmd_path_map(cmd_parms *parms, void *mconfig, char *path, char *val)
{
	sed_dir_cf *cfg = (sed_dir_cf *)mconfig;
	if (!cfg->pmap)
		cfg->pmap = ap_make_table(parms->pool, 10);
	ap_table_set(cfg->pmap, path, val);
	return NULL;
}

static command_rec sed_cmds[] = {
	{
		"SedProgram",		/* directive name */
		sed_cmd_program,		/* config action routine */
		NULL,			/* argument to include in call */
		OR_ALL,			/* where available */
		TAKE1,			/* arguments */
		"Sed programram expression or file"	/* description */
	},
	{
		"SedDefaultOutput",	/* directive name */
		sed_cmd_opt_n,		/* config action routine */
		NULL,			/* argument to include in call */
		OR_ALL,			/* where available */
		FLAG,			/* arguments */
		"Should sed output lines by default?"	/* description */
	},
	{
		"SedExtendedRegex",	/* directive name */
		sed_cmd_opt_E,		/* config action routine */
		NULL,			/* argument to include in call */
		OR_ALL,			/* where available */
		FLAG,			/* arguments */
		"Should sed use Extended regex?"	/* description */
	},
	{
		"SedPathMap",		/* directive name */
		sed_cmd_path_map,	/* config action routine */
		NULL,			/* argument to include in call */
		OR_ALL,			/* where available */
		TAKE2,			/* arguments */
		"Map PATH_INFO to a sed expr"	/* description */
	},
	{NULL}
};

/*------------------------------------------------------------------
 * Tools.
 */

static time_t
mtime(const char *fn)
{
	struct stat sb;

	if (stat(fn, &sb) == -1)
		return 0;
	else
		return sb.st_mtime;
}

static int
isfile(const char *fn) 
{
	struct stat sb;
	int r;

	if (stat(fn, &sb) == -1)
		/* doesn't appear to be accessible to us */
		return 0;
	else
		/* only return true for a regular file */
		return sb.st_mode & S_IFREG;
}

static void
compile_from(const char *program)
{
	/* XXX The following two casts shouldn't be necessary. */
	if (isfile(program)) {
		/* Make If-Modified-Since work */
		ap_update_mtime(s_r, mtime(program));
		ap_set_last_modified(s_r);
		add_compunit(CU_FILE, (char *)program);
	} else
		add_compunit(CU_STRING, (char *)program);
}

/*
 * sed used to have lots of statically initiallized variables.
 * Because we run through the code more than once, we have to do
 * proper initialization and cleanup of these things (which are now
 * plain globals) before each run.
 */
static void
sed_reinit(void)
{
	struct s_flist *fl, *fln;
	struct s_compunit *cu, *cun;

	/* Clean Hold Space, Pattern Space and Substitute Space */
	if (HS.back) free(HS.back);
	memset(&HS, '\0', sizeof(SPACE));
	if (PS.back) free(PS.back);
	memset(&PS, '\0', sizeof(SPACE));
	if (SS.back) free(SS.back);
	memset(&SS, '\0', sizeof(SPACE));

	/* Clean up the list of files to process */
	for (fl = files; fl; ) {
		fln = fl->next;
		/*
		 * We don't clean up the fname because it'll
		 * automatically have been freed by apache at the end
		 * of the previous request.
		 */
		free(fl);
		fl = fln;
	}
	files = NULL;
	fl_nextp = &files;

	/* Clean up the list of compilation units */
	for (cu = script; cu; ) {
		cun = cu->next;
		/*
		 * Again, the string will have come from apache memory
		 * so we don't need to free it here
		 */
		free(cu);
		cu = cun;
	}
	script = NULL;
	cu_nextp = &script;

	/* Restart mf_fgets() */
	if (curfile)
		fclose(curfile);
	curfile = NULL;
	lastline = 0;
}

/* Remap BSD function to apache function */
void
sed_warnx(const char *fmt, ...)
{
	va_list ap;
	char *str;
	
	va_start(ap, fmt);
	/* Thank $DEITY they got one useful function in the API... */
	str = ap_pvsprintf(s_r->pool, fmt, ap);
	str = ap_pstrcat(s_r->pool, "mod_sed: ", str);
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_WARNING, s_r, str);
	va_end(ap);
}

/*
 * Remap BSD function to apache function.  We ignore retval because
 * it's not terribly useful.
 */
void
sed_errx(int retval, const char *fmt, ...)
{
	va_list ap;
	char *str;
	
	va_start(ap, fmt);
	str = ap_pvsprintf(s_r->pool, fmt, ap);
	str = ap_pstrcat(s_r->pool, "mod_sed: ", str);
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, s_r, str);
	va_end(ap);
	longjmp(err_jmp_buf, 1);
}

/*
 * Remap BSD function to apache function.  We ignore retval because
 * it's not terribly useful.  This version appends strerror(errno).
 */
void
sed_err(int retval, const char *fmt, ...)
{
	va_list ap;
	char *str;
	int errno_sv = errno;
	
	va_start(ap, fmt);
	str = ap_pvsprintf(s_r->pool, fmt, ap);
	/* XXX Is this order of evaluation guaranteed? */
	str = ap_pstrcat(s_r->pool, "mod_sed: ", str, ": ", 
			 strerror(errno_sv), NULL);
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, s_r, str);
	va_end(ap);
	longjmp(err_jmp_buf, 1);
}

/*------------------------------------------------------------------
 * Main handler.
 */
 
static int
sed_handler(request_rec *r)
{
	int retval;
	sed_dir_cf *cf = ap_get_module_config(r->per_dir_config,
					      &sed_module);
	
	/* let the rest of sed at this request */
	s_r = r;

	/* clean up globals */
	sed_reinit();

	/* nobody told us what to do */
	if (cf->program == NULL) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_WARNING, r,
			      "sed_handler: no program for %s",
			      r->the_request);
		return DECLINED;
	}

	/* copied from sendfile.c */
	if (r->method_number == M_INVALID) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
			      "Invalid method in request %s", r->the_request);
		return NOT_IMPLEMENTED;
	}
	if (r->method_number == M_OPTIONS) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_DEBUG, r,
			      "sed_handler: can't do OPTIONS for %s",
			      r->the_request);
		return DECLINED;
	}
	if (r->method_number == M_PUT) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_DEBUG, r,
			      "sed_handler: no PUT allowed for %s",
			      r->the_request);
		return METHOD_NOT_ALLOWED;
	}

	/* getopt  :) */
	if (cf->opt_n)
		nflag = 1;
	if (cf->opt_E)
		rflags = REG_EXTENDED;

	/* do we have a file to process? */
	if (r->finfo.st_mode == 0) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_WARNING, r,
			      "sed_handler: file does not exist: %s",
			      r->the_request);
		return HTTP_NOT_FOUND;
	}

	/* Compile the program supplied to us. */
	if (*(r->path_info)) {
		const char *program;

		program = ap_table_get(cf->pmap, r->path_info);
		if (program) {
			compile_from(program);
		} else {
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_NOTICE,
				      r, "no mapping for %s", r->path_info);
			compile_from(cf->program);
		}
	} else {
		compile_from(cf->program);
	}

	/* Basically an mtime check for If-Modified-Since */
	if ((retval = ap_meets_conditions(r)) != OK)
	    return retval;

	/* XXX Should do caching of compiled bits... */
	/* XXX This lot needs to be watched for thrown errors. */
	if (setjmp(err_jmp_buf)) {
		return SERVER_ERROR;
	} else {
		compile();

		ap_chdir_file(r->filename);

		add_file(r->filename);
		if (!r->header_only)	/* cater for a HEAD request. */
			process();
		cfclose(prog, NULL);

		return OK;
	}
}

/* list of handlers we wish to publish */
static handler_rec sed_handlers[] = {
	{"sed-handler", sed_handler},
	{NULL}
};

/* what phases of the request we handle */
module MODULE_VAR_EXPORT sed_module = {
	STANDARD_MODULE_STUFF,
	NULL,		/* module initializer */
	sed_mk_dir_cf,	/* per-directory config creator */
	sed_merge_dir_cf, /* dir config merger */
	NULL,		/* server config creator */
	NULL,		/* server config merger */
	sed_cmds,	/* command table */
	sed_handlers,	/* [7] content handlers */
	NULL,		/* [2] URI-to-filename translation */
	NULL,		/* [5] check/validate user_id */
	NULL,		/* [6] check user_id is valid *here* */
	NULL,		/* [4] check access by host address */
	NULL,		/* [7] MIME type checker/setter */
	NULL,		/* [8] fixups */
	NULL,		/* [9] logger */
	NULL,		/* [3] header parser */
	NULL,		/* process initialization */
	NULL,		/* process exit/cleanup */
	NULL		/* [1] post read_request handling */
};
