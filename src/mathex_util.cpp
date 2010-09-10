#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "mathex_util.h"

unsigned char *embeddedimages ( int imagenum, int *nbytes, int *imgtype );

/* ==========================================================================
 * Function:	strchange ( nfirst, from, to )
 * Purpose:	Changes the nfirst leading chars of `from` to `to`.
 *		For example, to change char x[99]="12345678" to "123ABC5678"
 *		call strchange(1,x+3,"ABC")
 * --------------------------------------------------------------------------
 * Arguments:	nfirst (I)	int containing #leading chars of `from`
 *				that will be replace by `to`
 *		from (I/O)	char * to null-terminated string whose nfirst
 *				leading chars will be replaced by `to`
 *		to (I)		char * to null-terminated string that will
 *				replace the nfirst leading chars of `from`
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to first char of input `from`
 *				or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	If strlen(to)>nfirst, from must have memory past its null
 *		(i.e., we don't do a realloc)
 * ======================================================================= */
/* --- entry point --- */
char	*strchange ( int nfirst, char *from, char *to )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	tolen = (to==NULL?0:strlen(to)), /* #chars in replacement string */
	nshift = abs(tolen-nfirst);	/*need to shift from left or right*/
/* -------------------------------------------------------------------------
shift from left or right to accommodate replacement of its nfirst chars by to
-------------------------------------------------------------------------- */
if ( tolen < nfirst )			/* shift left is easy */
  strcpy(from,from+nshift);		/* because memory doesn't overlap */
if ( tolen > nfirst )			/* need more room at start of from */
  { char *pfrom = from+strlen(from);	/* ptr to null terminating from */
    for ( ; pfrom>=from; pfrom-- )	/* shift all chars including null */
      *(pfrom+nshift) = *pfrom; }	/* shift chars nshift places right */
/* -------------------------------------------------------------------------
from has exactly the right number of free leading chars, so just put to there
-------------------------------------------------------------------------- */
if ( tolen != 0 )			/* make sure to not empty or null */
  memcpy(from,to,tolen);		/* chars moved into place */
return ( from );			/* changed string back to caller */
} /* --- end-of-function strchange() --- */

/* ==========================================================================
 * Function:	strreplace ( string, from, to, iscase, nreplace )
 * Purpose:	Changes the first nreplace occurrences of 'from' to 'to'
 *		in string, or all occurrences if nreplace=0.
 * --------------------------------------------------------------------------
 * Arguments:	string (I/0)	char * to null-terminated string in which
 *				occurrence of 'from' will be replaced by 'to'
 *		from (I)	char * to null-terminated string
 *				to be replaced by 'to'
 *		to (I)		char * to null-terminated string that will
 *				replace 'from'
 *		iscase (I)	int containing 1 if matches of 'from'
 *				in 'string' should be case-sensitive,
 *				or 0 if matches are case-insensitive.
 *		nreplace (I)	int containing (maximum) number of
 *				replacements, or 0 to replace all.
 * --------------------------------------------------------------------------
 * Returns:	( int )		number of replacements performed,
 *				or 0 for no replacements or -1 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	strreplace ( char *string, char *from, char *to,
	int iscase, int nreplace )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	fromlen = (from==NULL?0:strlen(from)), /* #chars to be replaced */
	tolen = (to==NULL?0:strlen(to)); /* #chars in replacement string */
int	iscommand = (fromlen<2?0:(*from=='\\'?1:0)); /*is from a \command ?*/
char	*pfrom = (char *)NULL,		/*ptr to 1st char of from in string*/
	*pstring = string;		/*ptr past previously replaced from*/
//	*strchange();			/* change 'from' to 'to' */
/*int	iscase = 1;*/			/* true for case-sensitive match */
int	nreps = 0;			/* #replacements returned to caller*/
/* -------------------------------------------------------------------------
repace occurrences of 'from' in string to 'to'
-------------------------------------------------------------------------- */
if ( string == (char *)NULL		/* no input string */
||   (fromlen<1 && nreplace<=0) )	/* replacing empty string forever */
  nreps = (-1);				/* so signal error */
else					/* args okay */
  while (nreplace<1 || nreps<nreplace) { /* up to #replacements requested */
    if ( fromlen > 0 )			/* have 'from' string */
      pfrom =				/*ptr to 1st char of from in string*/
       (iscase>0? strstr(pstring,from):	/* case-sensistive match */
        strcasestr(pstring,from));	/* case-insensistive match */
    else  pfrom = pstring;		/*or empty from at start of string*/
    if ( pfrom == (char *)NULL ) break;	/*no more from's, so back to caller*/
    if ( iscommand )			/* ignore prefix of longer string */
      if ( isalpha((int)(*(pfrom+fromlen))) ) {	/* just a longer string */
        pstring = pfrom+fromlen;	/* pick up search after 'from' */
        continue; }			/* don't change anything */
    if ( iscase > 1 ) { ; }		/* ignore \escaped matches */
    if ( strchange(fromlen,pfrom,to)	/* leading 'from' changed to 'to' */
    ==   (char *)NULL ) { nreps=(-1); break; } /* signal error to caller */
    nreps++;				/* count another replacement */
    pstring = pfrom+tolen;		/* pick up search after 'to' */
    if ( *pstring == '\000' ) break;	/* but quit at end of string */
    } /* --- end-of-while() --- */
return ( nreps );			/* #replacements back to caller */
} /* --- end-of-function strreplace() --- */

/* ==========================================================================
 * Function:	strwrap ( s, linelen, tablen )
 * Purpose:	Inserts \n's and spaces in (a copy of) s to wrap lines
 *		at linelen and indent them by tablen.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		char * to null-terminated string
 *				to be wrapped.
 *		linelen (I)	int containing maximum linelen
 *				between \n's.
 *		tablen (I)	int containing number of spaces to indent
 *				lines.  0=no indent.  Positive means
 *				only indent first line and not others.
 *				Negative means indent all lines except first.
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to "line-wrapped" copy of s
 *				or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The returned copy of s has embedded \n's as necessary
 *		to wrap lines at linelen.  Any \n's in the input copy
 *		are removed first.  If (and only if) the input s contains
 *		a terminating \n then so does the returned copy.
 *	      o	The returned pointer addresses a static buffer,
 *		so don't call strwrap() again until you're finished
 *		with output from the preceding call.
 * ======================================================================= */
/* --- entry point --- */
char	*strwrap ( char *s, int linelen, int tablen )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char sbuff[4096];		/* line-wrapped copy of s */
char	*sol = sbuff;			/* ptr to start of current line*/
char	tab[32] = "                 ";	/* tab string */
//int	strreplace();			/* remove \n's */
//char	*strchange();			/* add \n's and indent space */
int	finalnewline = (lastchar(s)=='\n'?1:0); /*newline at end of string?*/
int	istab = (tablen>0?1:0);		/* init true to indent first line */
int	rhslen  = 0,			/* remaining right hand side length*/
	thislen = 0,			/* length of current line segment */
	thistab = 0,			/* length of tab on current line */
	wordlen = 0;			/* length to next whitespace char */
/* -------------------------------------------------------------------------
Make a clean copy of s
-------------------------------------------------------------------------- */
/* --- check input --- */
*sbuff = '\000';			/* initialize in case of error */
if ( isempty(s) ) goto end_of_job;	/* no input */
if ( tablen < 0 ) tablen = (-tablen);	/* set positive tablen */
if ( tablen >= linelen ) tablen = linelen-1; /* tab was longer than line */
tab[min2(tablen,16)] = '\000';		/* null-terminate tab string */
tablen = strlen(tab);			/* reset to actual tab length */
/* --- start with copy of s --- */
strninit(sbuff,s,3000);			/* leave room for \n's and tabs */
if ( linelen < 1 ) goto end_of_job;	/* can't do anything */
trimwhite(sbuff);			/*remove leading/trailing whitespace*/
strreplace(sbuff,"\n"," ",0,0);		/* remove any original \n's */
strreplace(sbuff,"\r"," ",0,0);		/* remove any original \r's */
strreplace(sbuff,"\t"," ",0,0);		/* remove any original \t's */
/* -------------------------------------------------------------------------
Insert \n's and spaces as needed
-------------------------------------------------------------------------- */
while ( 1 ) {				/* till end-of-line */
  /* --- init --- */
  trimwhite(sol);			/*remove leading/trailing whitespace*/
  thislen = thistab = 0;		/* no chars in current line yet */
  if ( istab && tablen>0 ) {		/* need to indent this line */
    strchange(0,sol,tab);		/* insert indent at start of line */
    thistab = tablen; }			/* line starts with whitespace tab */
  if ( sol == sbuff ) istab = 1-istab;	/* flip tab flag after first line */
  sol += thistab;			/* skip tab */
  rhslen = strlen(sol);			/* remaining right hand side chars */
  if ( rhslen <= linelen ) break;	/* no more \n's needed */
  if ( 0 && msglevel >= 99 ) {
    fprintf(stdout,"strwrap> rhslen=%d, sol=\"\"%s\"\"\n",rhslen,sol);
    fflush(stdout); }
  /* --- look for last whitespace preceding linelen --- */
  while ( 1 ) {				/* till we exceed linelen */
    wordlen = strcspn(sol+thislen,WHITESPACE); /*ptr to next whitespace char*/
    if ( thislen+thistab+wordlen >= linelen ) break; /*next word won't fit*/
    thislen += (wordlen+1); }		/* ptr past next whitespace char */
  if ( thislen < 1 ) break;		/* line will have one too-long word*/
  sol[thislen-1] = '\n';		/* replace last space with newline */
  sol += thislen;			/* next line starts after newline */
  } /* --- end-of-while(1) --- */
end_of_job:
  if ( finalnewline ) strcat(sbuff,"\n"); /* replace final newline */
  return ( sbuff );			/* back with clean copy of s */
} /* --- end-of-function strwrap() --- */




/* --- entry point --- */
int	emitembedded ( int imagenum, int isquery )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes = 0,			/* #bytes in embedded image */
	imgtype = 0;			/* image type 1=gif,2=png */
unsigned char *eimage=NULL; //, *embeddedimages(); /* retrieve embedded image */
int	emitcache(char *cachefile, int maxage, int isbuffer);			/* emit embedded image to stdout */
int	maxage = MAXAGE;		/* maxage is typically 7200 secs */
/* --------------------------------------------------------------------------
Retrieve and emit embedded image
-------------------------------------------------------------------------- */
/* --- retrieve image --- */
if ( imagenum<1 || imagenum>MAXEMBEDDED ) imagenum=1; /*out-of-bounds check*/
eimage = embeddedimages(imagenum,&nbytes,&imgtype); /*retrieve image*/
/* --- display message image on stdout --- */
if ( isquery )				/* don't emit error to shell stdout*/
  if ( eimage != NULL ) {		/* have embedded image */
    imagetype = imgtype;		/* set embedded image type */
    nbytes = emitcache((char *)eimage,maxage,nbytes); } /* and emit image */
/* --- show message text on msgfp --- */
if ( isquery != 2 )			/* msgfp output enabled */
  if ( msgfp!=NULL && msglevel>=1 )	/* display on msgfp, too */
    fprintf(msgfp,"\nmathTeX> message#%d %s:\n         %s\n",
    imagenum,(nbytes>0?"succeeded":"failed"),
    strwrap(embeddedtext[imagenum],64,-9));
/* --- back to caller --- */
return ( nbytes );			/* return #bytes emitted, 0=error */
} /* --- end-of-function emitembedded() --- */

/* ==========================================================================
 * Function:	readcachefile ( cachefile, buffer )
 * Purpose:	read cachefile into buffer
 * --------------------------------------------------------------------------
 * Arguments:	cachefile (I)	pointer to null-terminated char string
 *				containing full path to file to be read
 *		buffer (O)	pointer to unsigned char string
 *				returning contents of cachefile
 * --------------------------------------------------------------------------
 * Returns:	( int )		#bytes read (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	readcachefile ( char *cachefile, unsigned char *buffer )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
FILE	*cacheptr = fopen(cachefile,"rb"); /*open cachefile for binary read*/
unsigned char cachebuff[512];		/* bytes from cachefile */
int	buflen = 256,			/* #bytes we try to read from file */
	nread = 0,			/* #bytes actually read from file */
	maxbytes = MAXGIFSZ,		/* max #bytes returned in buffer */
	nbytes = 0;			/* total #bytes read */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- check that files opened okay --- */
if ( cacheptr == (FILE *)NULL ) goto end_of_job; /*failed to open cachefile*/
/* --- check that output buffer provided --- */
if ( buffer == (unsigned char *)NULL ) goto end_of_job; /* no buffer */
/* -------------------------------------------------------------------------
read bytes from cachefile
-------------------------------------------------------------------------- */
while ( 1 )
  {
  /* --- read bytes from cachefile --- */
  nread = fread(cachebuff,sizeof(unsigned char),buflen,cacheptr); /* read */
  if ( nbytes + nread > maxbytes )	/* block too big for buffer */
    nread = maxbytes - nbytes;		/* so truncate it */
  if ( nread < 1 ) break;		/* no bytes left in cachefile */
  /* --- store bytes in buffer --- */
  memcpy(buffer+nbytes,cachebuff,nread); /* copy current block to buffer */
  /* --- ready to read next block --- */
  nbytes += nread;			/* bump total #bytes emitted */
  if ( nread < buflen ) break;		/* no bytes left in cachefile */
  if ( nbytes >= maxbytes ) break;	/* avoid buffer overflow */
  } /* --- end-of-while(1) --- */
end_of_job:
  if ( cacheptr != NULL ) fclose(cacheptr); /* close file if opened */
  return ( nbytes );			/* back with #bytes emitted */
} /* --- end-of-function readcachefile() --- */

/* ==========================================================================
 * Function:	emitcache ( cachefile, maxage, isbuffer )
 * Purpose:	dumps bytes from cachefile to stdout
 * --------------------------------------------------------------------------
 * Arguments:	cachefile (I)	pointer to null-terminated char string
 *				containing full path to file to be dumped,
 *				or contains buffer of bytes to be dumped
 *		maxage (I)	int containing maxage. in seconds, for
 *				http header, or -1 to not emit headers
 *		isbuffer (I)	>=1 if cachefile is buffer of bytes to be
 *				dumped, and if isbuffer>=9 it's the #bytes
 *				in buffer to be dumped.  Otherwise,
 *				strlen(cachefile) determines #bytes.
 * --------------------------------------------------------------------------
 * Returns:	( int )		#bytes dumped (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	emitcache ( char *cachefile, int maxage, int isbuffer )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes=0;// readcachefile();	/* read cache file */
FILE	*emitptr = stdout;		/* emit cachefile to stdout */
unsigned char buffer[MAXGIFSZ+1];	/* bytes from cachefile */
unsigned char *buffptr = buffer;	/* ptr to buffer */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- check that files opened okay --- */
if ( emitptr == (FILE *)NULL )		/* failed to open emit file */
  goto end_of_job;			/* so return 0 bytes to caller */
/* --- read the file if necessary --- */
if ( isbuffer ) {			/* cachefile is buffer */
 buffptr = (unsigned char *)cachefile;	/* so reset buffer pointer */
 nbytes = (isbuffer<9?strlen((char *)buffptr):isbuffer); }/*determine #bytes*/
else					/* cachefile is file name */
 if ( (nbytes = readcachefile(cachefile,buffer)) /* read the file */
 < 1 ) {				/* file not read */
   msgnumber = EMITFAILED;		/* signal error */
   goto end_of_job; }			/* quit if file not read */
/* --- first emit http headers if requested --- */
if ( /*isemitcontenttype*/		/* content-type lines enabled */
/*&&*/ maxage >= 0 )			/* caller wants http headers */
 { /* --- emit mime content-type line --- */
   fprintf( emitptr, "Cache-Control: max-age=%d\n",maxage );
   fprintf( emitptr, "Content-Length: %d\n",nbytes );
   fprintf( emitptr, "Content-type: image/%s\n\n",extensions[imagetype] ); }
/* -------------------------------------------------------------------------
set stdout to binary mode (for Windows)
-------------------------------------------------------------------------- */
/* emitptr = fdopen(STDOUT_FILENO,"wb"); */  /* doesn't work portably, */
#ifdef WINDOWS				/* so instead... */
  #ifdef HAVE_SETMODE			/* prefer (non-portable) setmode() */
    if ( setmode ( fileno (stdout), O_BINARY) /* windows specific call */
    == -1 ) ; /* handle error */	/* sets stdout to binary mode */
  #else					/* setmode() not available */
    #if 1
      freopen ("CON", "wb", stdout);	/* freopen() stdout binary */
    #else
      stdout = fdopen (STDOUT_FILENO, "wb"); /* fdopen() stdout binary */
    #endif
  #endif
#endif
/* -------------------------------------------------------------------------
emit bytes from cachefile
-------------------------------------------------------------------------- */
/* --- write bytes to stdout --- */
if ( fwrite(buffptr,sizeof(unsigned char),nbytes,emitptr) /* write buffer */
<    nbytes )				/* failed to write all bytes */
  nbytes = 0;				/* reset total count to 0 */
end_of_job:
  return ( nbytes );			/* back with #bytes emitted */
} /* --- end-of-function emitcache() --- */



