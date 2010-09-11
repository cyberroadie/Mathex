#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mathex_string.h"

/* ---
 * cache path -DCACHE=\"path/\" specifies directory
 * ------------------------------------------------ */
#if !defined(CACHE)
  #define CACHE "mathtex/"		/* relative to mathtex.cgi */
#endif
#if !defined(CACHELOG)
  #define CACHELOG "mathtex.log"	/* default cache log file */
#endif
#define	MAXAGE 7200			/* maxage in cache is 7200 secs */
static	int  iscaching = 1;		/* true if caching images */
static	char cachepath[256] = CACHE;	/* path to cached image files */

/* ---
 * time zone delta t (in hours)
 * ---------------------------- */
#if !defined(TZDELTA)
  #define TZDELTA 0
#endif

/* ---
 * image type info specifying gif, png
 * ----------------------------------- */
#if defined(GIF)
  #define IMAGETYPE 1
#endif
#if defined(PNG)
  #define IMAGETYPE 2
#endif
#if !defined(IMAGETYPE)
  #define IMAGETYPE 1
#endif
static	int  imagetype = IMAGETYPE;	/* 1=gif, 2=png */
static	char *extensions[] = { NULL,	/* image type file .extensions */
  "gif", "png", NULL };

/* ==========================================================================
 * Function:	mathlog ( expression, filename )
 * Purpose:	write entry in log file
 * --------------------------------------------------------------------------
 * Arguments:	expression (I)	pointer to null-terminated char string
 *				containing latex math expression
 *		filename (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	mathlog ( char *expression, char *filename )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
FILE	*filefp = NULL;			/* fopen(filename) for logfile */
char	*timestamp(int tzdelta, int ifmt);			/* time stamp for logged messages */
char	*makepath(char *path, char *name, char *extension);			/* construct full path/filename.ext*/
char	*http_referer = getenv("HTTP_REFERER"); /* referer using mathTeX */
char	*dashes =			/* separates logfile entries */
 "--------------------------------------------------------------------------";
/* -------------------------------------------------------------------------
write message to cache log file
-------------------------------------------------------------------------- */
if ( !isempty(CACHELOG) ) {		/* if a logfile is given */
  if ( (filefp=fopen(makepath(NULL,CACHELOG,NULL),"a")) /*open logfile*/
  !=   NULL ) {				/* ignore logging if can't open */
    int isreflogged = 0;		/* set true if http_referer logged */
    fprintf(filefp,"%s                 %s\n", /* timestamp, md5 file */
    timestamp(TZDELTA,0),		/* timestamp */
    makepath("",filename,extensions[imagetype])); /* hashed filename */
    fprintf(filefp,"%s\n",expression);	/* expression in filename */
    if ( !isempty(http_referer) ) {	/* show referer if we have one */
      int loglen = strlen(dashes);	/* #chars on line in log file*/
      char *refp = http_referer;	/* line to be printed */
      isreflogged = 1;			/* signal http_referer logged*/
      while ( 1 ) {			/* printed in parts if needed*/
	fprintf(filefp,"%.*s\n",loglen,refp); /* print a part */
	if ( strlen(refp) <= loglen ) break;  /* no more parts */
	refp += loglen; }		/* bump ptr to next part */
      } /* --- end-of-if(!isempty(http_referer)) --- */
    if ( !isreflogged )			/* http_referer not logged */
      fprintf(filefp,"http://none\n");	/* so log dummy referer line */
    fprintf(filefp,"%s\n",dashes);	/* separator line */
    fclose(filefp);			/* close logfile immediately */
    } /* --- end-of-if(filefp!=NULL) --- */
  } /* --- end-of-if(!isempty(CACHELOG)) --- */
return ( 1 );
} /* --- end-of-function mathlog() --- */

