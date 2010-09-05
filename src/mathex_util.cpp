#include "mathex_util.h"

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
int	strreplace();			/* remove \n's */
char	*strchange();			/* add \n's and indent space */
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

/* ==========================================================================
 * Function:	emitembedded ( imagenum, isquery )
 * Purpose:	Emits embedded gif (or png) message to stdout
 * --------------------------------------------------------------------------
 * Arguments:	imagenum (I)	int containing image number desired
 *		isquery (I)	int containing 0 for command-line run,
 *				1 for query mode, 2 for query mode but
 *				suppress msgfp output even if it's enabled.
 * Returns:	int		#bytes emitted (returned by emitcache()),
 *				or 0 for failure
 * --------------------------------------------------------------------------
 * Notes:     o	If mathTeX fails, these images contain
 *		error messages that can be displayed
 * ======================================================================= */
/* --- entry point --- */
int	emitembedded ( int imagenum, int isquery )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes = 0,			/* #bytes in embedded image */
	imgtype = 0;			/* image type 1=gif,2=png */
unsigned char *eimage=NULL, *embeddedimages(); /* retrieve embedded image */
int	emitcache();			/* emit embedded image to stdout */
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
int	nbytes=0, readcachefile();	/* read cache file */
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