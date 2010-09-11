#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "mathex_util.h"

/* ---
 * windows-specific header info
 * ---------------------------- */
#if defined(WINDOWS)		/* Windows opens stdout in char mode, and */
  #include <fcntl.h>		/* precedes every 0x0A with spurious 0x0D.*/
  #include <io.h>		/* So emitcache() issues a Win _setmode() */
				/* call to put stdout in binary mode. */
  #if defined(_O_BINARY) && !defined(O_BINARY)  /* only have _O_BINARY */
    #define O_BINARY _O_BINARY	/* make O_BINARY available, etc... */
    #define setmode  _setmode
    #define fileno   _fileno
  #endif
  #if defined(_O_BINARY) || defined(O_BINARY)  /* setmode() now available */
    #define HAVE_SETMODE	/* so we'll use setmode() */
  #endif
#endif
static	int iswindows =		/* 1 if running under Windows, or 0 if not */
  #ifdef WINDOWS
    1;				/* 1 when program running under Windows */
  #else
    0;				/* 0 when not running under Windows */
  #endif

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

/* ==========================================================================
 * Function:	advertisement ( expression, message )
 * Purpose:	wrap expression in advertisement message
 * --------------------------------------------------------------------------
 * Arguments:	expression (I/O) pointer to null-terminated char string
 *				containing expression to be "wrapped",
 *				and returning wrapped expression
 *		message (I)	pointer to null-terminated char string
 *				containing template for advertisement
 *				message, or NULL to use default message
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	advertisement ( char *expression, char *message )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- advertisement template --- */
char  *adtemplate =
	#if defined(ADVERTISEMENT)	/* cc -DADVERTISEMENT=\"filename\" */
	  #include ADVERTISEMENT	/* filename with advertisement */
	#else				/* formatted as illustrated below */
	"\\begin{center}\n"
	"\\fbox{$\\mbox{\\footnotesize\\LaTeX{} rendering courtesy of}\\atop\n"
	"\\mbox{\\scriptsize http://www.forkosh.com/mathtex.html}$}\\\\ \n"
	"\\vspace*{-4mm}\n"
	" %%beginmath%% %%expression%% %%endmath%% \n"
	"\\end{center}\n"
	#endif
	;				/* terminating semicolon */
/* --- other variables --- */
char	adbuffer[MAXEXPRSZ+2048];	/*construct wrapped expression here*/
char	*beginmath[] = { "\\[", "$", " " }, /* start math mode */
	*endmath[] =   { "\\]", "$", " " }; /* end math mode */
int	strreplace(char *string, char *from, char *to,
	int iscase, int nreplace);			/* replace %%keywords%% with values*/
/* -------------------------------------------------------------------------
wrap expression in advertisement
-------------------------------------------------------------------------- */
/* --- start with template --- */
if ( isempty(message) )			/* caller didn't supply message */
  message = adtemplate;			/* so use default message */
strcpy(adbuffer,message);		/* copy message template to buffer */
/* --- replace %%beginmath%%...%%endmath%% with \[...\] or with $...$ --- */
  if ( mathmode<0 || mathmode>2 ) mathmode=0; /* out-of-bounds sanity check*/
  if ( isempty(expression) ) mathmode = 2; /*ad only, no need for math mode*/
  strreplace(adbuffer,"%%beginmath%%",beginmath[mathmode],1,0);
  strreplace(adbuffer,"%%endmath%%",endmath[mathmode],1,0);
/* --- replace %%expression%% in template with expression --- */
  strreplace(adbuffer,"%%expression%%",expression,1,0);
/* --- replace original expression --- */
strcpy(expression,adbuffer);		/* expression mow wrapped in ad */
mathmode = 2;				/* \[...\] already in expression */
return ( 1 );				/* always just return 1 */
} /* --- end-of-function advertisement() --- */

/* ==========================================================================
 * Function:	crc16 ( s )
 * Purpose:	16-bit crc of string s
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		pointer to null-terminated char string
 *				whose crc is desired
 * --------------------------------------------------------------------------
 * Returns:	( int )		16-bit crc of s
 * --------------------------------------------------------------------------
 * Notes:     o	From Numerical Recipes in C, 2nd ed, page 900.
 * ======================================================================= */
/* --- entry point --- */
int	crc16 ( char *s )
{
/* -------------------------------------------------------------------------
Compute the crc
-------------------------------------------------------------------------- */
unsigned short crc = 0;			/* returned crc */
int	ibit;				/* for(ibit) eight one-bit shifts */
while ( !isempty(s) ) {			/* while there are still more chars*/
  crc = (crc ^ (*s)<<8);		/* add next char */
  for ( ibit=0; ibit<8; ibit++ )	/* generator polynomial */
    if ( crc & 0x8000 ) { crc<<=1; crc=crc^4129; }
    else crc <<= 1;
  s++;					/* next xhar */
  } /* --- end-of-while(!isempty(s)) --- */
return ( (int)crc );			/* back to caller with crc */
} /* --- end-of-function crc16() --- */

/* ==========================================================================
 * Function:	timelimit ( char *command, int killtime )
 * Purpose:	Issues a system(command) call, but throttles command
 *		after killtime seconds if it hasn't already completed.
 * --------------------------------------------------------------------------
 * Arguments:	command (I)	char * to null-terminated string
 *				containing system(command) to be executed
 *		killtime (I)	int containing maximum #seconds
 *				to allow command to run
 *				to array of returned arg strings
 * --------------------------------------------------------------------------
 * Returns:	( int )		return status from command,
 *				or -1 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The timelimit() code is adapted from
 *		   http://devel.ringlet.net/sysutils/timelimit/
 *		Compile with -DTIMELIMIT=\"$(which timelimit)\" to use an
 *		installed copy of timelimit rather than this built-in code.
 *	      o if symbol ISCOMPILETIMELIMIT is false, a stub function
 *		that just issues system(command) is compiled instead.
 * ======================================================================= */
#if !ISCOMPILETIMELIMIT
/* --- entry point for stub timelimit() function --- */
int	timelimit(char *command, int killtime) {
  if ( isempty(command) )		/* no command given */
    return( (killtime==(-99)?991:(-1)) ); /* return -1 or stub identifier */
  return ( system(command) ); }		/* just issue system(command) */
#else
/* --- entry points for signal handlers --- */
void	sigchld(int sig)    { fdone  = 1; }
void	sigalrm(int sig)    { falarm = 1; }
void	sighandler(int sig) { sigcaught = sig; fsig = 1; }

/* --- entry point --- */
int	timelimit(char *command, int killtime)
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
pid_t	pid = 0;
int	killsig = (int)(SIGKILL);
int	setsignal(int sig, void (*handler)(int));
int	status = (-1);
/* -------------------------------------------------------------------------
check args
-------------------------------------------------------------------------- */
if ( isempty(command) )			/* no command given */
  return( (killtime==(-99)?992:(-1)) );	/* return -1 or built-in identifier*/
if ( killtime < 1   ) return ( system(command) ); /* throttling disabled */
if ( killtime > 999 ) killtime = 999;	/* default maximum to 999 seconds */
/* -------------------------------------------------------------------------
install signal handlers
-------------------------------------------------------------------------- */
fdone = falarm = fsig = sigcaught = 0;
if ( setsignal(SIGALRM, sigalrm)    < 0 ) return(-1);
if ( setsignal(SIGCHLD, sigchld)    < 0 ) return(-1);
if ( setsignal(SIGTERM, sighandler) < 0 ) return(-1);
if ( setsignal(SIGHUP,  sighandler) < 0 ) return(-1);
if ( setsignal(SIGINT,  sighandler) < 0 ) return(-1);
if ( setsignal(SIGQUIT, sighandler) < 0 ) return(-1);
/* -------------------------------------------------------------------------
fork off the child process
-------------------------------------------------------------------------- */
fflush(NULL);				/* flush all buffers before fork */
if ( (pid=fork()) < 0 ) return(-1);	/* failed to fork */
if ( pid == 0 ) {			/* child process... */
  status = system(command);		/* ...submits command */
  _exit(status); }			/* and _exits without user cleanup */
/* -------------------------------------------------------------------------
parent process sleeps for allowed time
-------------------------------------------------------------------------- */
alarm(killtime);
while ( !(fdone||falarm||fsig) ) pause();
alarm(0);
/* -------------------------------------------------------------------------
send kill signal if child hasn't completed command
-------------------------------------------------------------------------- */
if ( fsig )  return(-1);		/* some other signal stopped child */
if ( !fdone ) kill(pid, killsig);	/* not done, so kill it */
/* -------------------------------------------------------------------------
return status of child pid
-------------------------------------------------------------------------- */
if ( waitpid(pid, &status, 0) == -1 ) return(-1); /* can't get status */
if ( 1 ) return(status);		/* return status to caller */
#if 0					/* interpret status */
  if ( WIFEXITED(status) )
	return (WEXITSTATUS(status));
  else if ( WIFSIGNALED(status) )
	return (WTERMSIG(status) + 128);
  else
	return (EX_OSERR);
#endif
} /* --- end-of-function timelimit() --- */

/* --- entry point --- */
int	setsignal ( int sig, void (*handler)(int) )
{
 #ifdef HAVE_SIGACTION
    struct  sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    act.sa_flags = 0;
    #ifdef SA_NOCLDSTOP
      act.sa_flags |= SA_NOCLDSTOP;
    #endif
    if (sigaction(sig, &act, NULL) < 0) return(-1);
 #else
    if (signal(sig, handler) == SIG_ERR) return(-1);
 #endif
    return(0);
} /* --- end-of-function setsignal() --- */
#endif /* ISCOMPILETIMELIMIT */


/* ==========================================================================
 * Function:	makepath ( path, name, extension )
 * Purpose:	return string containing path/name.extension
 * --------------------------------------------------------------------------
 * Arguments:	path (I)	pointer to null-terminated char string
 *				containing "" path or path/ or NULL to
 *				use cachepath if caching enabled
 *		name (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file or NULL
 *		extension (I)	pointer to null-terminated char string
 *				containing extension or NULL
 * --------------------------------------------------------------------------
 * Returns:	( char * )	"cachepath/filename.extension" or NULL=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*makepath ( char *path, char *name, char *extension )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char namebuff[512];		/* buffer for constructed filename */
char	*filename = NULL;		/*ptr to filename returned to caller*/
/* -------------------------------------------------------------------------
construct filename
-------------------------------------------------------------------------- */
/* --- validity checks --- */
/*if ( isempty(name) ) goto end_of_job;*/ /* no name supplied by caller */
/* --- start with caller's path/ or default path to cache directory --- */
*namebuff = '\000';			/* re-init namebuff */
if ( path == NULL ) {			/* use default path to cache */
  if ( !isempty(cachepath) )		/* have a cache path */
    strcpy(namebuff,cachepath); }	/* begin filename with path */
else					/* or use caller's supplied path */
  if ( *path != '\000' )		/* if it's not an empty string */
    strcpy(namebuff,path);		/* begin filename with path */
if ( !isempty(namebuff) )		/* have a leading path */
  if ( !isthischar(lastchar(namebuff),"\\/") ) /* no \ or / at end of path */
    strcat(namebuff,(iswindows?"\\":"/")); /* so add windows\ or unix/ */
/* --- add name after path/ (name arg might just be a blank space) --- */
if ( !isempty(name) ) {			/* name supplied by caller */
  if ( !isempty(namebuff) )		/* and if we already have a path */
    if ( isthischar(*name,"\\/") ) name++; /* skip leading \ or / in name */
  strcat(namebuff,name); }		/* name concatanated after path/ */
/* --- add extension after path/name */
if ( !isempty(extension) ) {		/* have a filename extension */
  if ( !isthischar(lastchar(namebuff),".") ) { /* no . at end of name */
    if ( !isthischar(*extension,".") )	/* and extension has no leading . */
      strcat(namebuff,".");		/* so we need to add our own . */
    strcat(namebuff,extension); }	/* add extension after path/name. */
  else					/* . already at end of name */
    strcat(namebuff,			/* add extension without . */
    (!isthischar(*extension,".")?extension:extension+1)); /*skip leading . */
  } /* --- end-of-if(!isempty(extension)) --- */
filename = namebuff;			/* successful name back to caller */
/*end_of_job:*/
  return ( filename );			/* back with name or NULL=error */
} /* --- end-of-function makepath() --- */

/* ==========================================================================
 * Function:	isdexists ( dirname )
 * Purpose:	check whether or not directory exists
 * --------------------------------------------------------------------------
 * Arguments:	dirname (I)	pointer to null-terminated char string
 *				containing directory name to check for
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 = directory exists, 0 = not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isdexists ( char *dirname )
{
int	status = 0;			/* init for non-existant dirname */
if ( !isempty(dirname) ) {		/* must have directory name */
  char	directory[512];			/* local copy of dirname */
  DIR	*dp = NULL;			/* dp=opendir() opens directory */
  strcpy(directory,dirname);		/* start with name given by caller */
  if ( !isthischar(lastchar(directory),"/") ) /* no / at end of directory */
    strcat(directory,"/");		/* so add one ourselves */
  if ( (dp=opendir(directory)) != NULL ) { /* dirname exists */
    status = 1;				/* so flip status */
    closedir(dp); }			/* and close the directory */
  } /* --- end-of-if(!isempty(dirname)) --- */
return ( status );			/* tell caller if we found dirname */
} /* --- end-of-function isdexists() --- */

/* ==========================================================================
 * Function:	setpaths ( method )
 * Purpose:	try to set accurate paths for
 *		latex,pdflatex,timelimit,dvipng,dvips,convert
 * --------------------------------------------------------------------------
 * Arguments:	method (I)	10*ltxmethod + imgmethod where
 *				ltxmethod =
 *				   1 for latex, 2 for pdflatex,
 *				   0 for both
 *				imgmethod =
 *				   1 for dvipng, 2 for dvips/convert,
 *				   0 for both
 * --------------------------------------------------------------------------
 * Returns:	( int )		1
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	setpaths ( int method )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char	*programpath=NULL, *whichpath(char *, int *); /* look for latex,dvipng,etc */
int	nlocate = 0;			/*if which fails, #locate|grep hits*/
int	ltxmethod = method/10,		/* divide by 10 */
	imgmethod = method%10;		/* remainder after division by 10 */
static	int islatexwhich=0, ispdflatexwhich=0,    /* set true after we try */
	isdvipngwhich=0,    isdvipswhich=0,  /*whichpath() for that program*/
	isconvertwhich=0,   istimelimitwhich=0;
/* ---
 * set paths, either from -DLATEX=\"path/latex\", etc, or call whichpath()
 * ----------------------------------------------------------------------- */
/* --- path for latex program --- */
if ( ltxmethod==0 || ltxmethod==1 )	/* not needed for pdflatex only */
 if ( !ISLATEXSWITCH && !islatexwhich ) { /* no -DLATEX=\"path/latex\" */
  islatexwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("latex",&nlocate)) /* try to find latex path */
  !=   NULL ) {				/* succeeded */
    islatexpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(latexpath,programpath,255); } }/* copy path from whichpath() */
/* --- path for pdflatex program --- */
if ( ltxmethod==0 || ltxmethod==2 )	/* not needed for latex only */
 if ( !ISPDFLATEXSWITCH && !ispdflatexwhich ) { /*no -DPDFLATEX=\"pdflatex\"*/
  ispdflatexwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("pdflatex",&nlocate)) /*try to find pdflatex*/
  !=   NULL ) {				/* succeeded */
    ispdflatexpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(pdflatexpath,programpath,255); } }/*copy path from whichpath()*/
/* --- path for timelimit program --- */
if ( 0 )				/* only use explicit -DTIMELIMIT */
 if ( warntime > 0 )			/* have -DWARNTIME or -DTIMELIMIT */
  if ( !ISTIMELIMITSWITCH		/*no -DTIMELIMIT=\"path/timelimit\"*/
  &&   !istimelimitwhich ) {
    istimelimitwhich = 1;		/* signal that which already tried */
    nlocate = ISLOCATE;			/* use locate if which fails??? */
    if ( (programpath=whichpath("timelimit",&nlocate)) /* try to find path */
    !=   NULL ) {			/* succeeded */
      istimelimitpath = (nlocate==0?2:3); /* set flag signalling which() */
      strninit(timelimitpath,programpath,255); } } /* copy from whichpath()*/
/* --- path for dvipng program --- */
if ( imgmethod != 2 )			/* not needed for dvips/convert */
 if ( !ISDVIPNGSWITCH && !isdvipngwhich ) { /* no -DDVIPNG=\"path/dvipng\" */
  isdvipngwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("dvipng",&nlocate)) /* try to find dvipng */
  !=   NULL ) {				/* succeeded */
    isdvipngpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(dvipngpath,programpath,255); } }/*so copy path from whichpath()*/
/* --- path for dvips program --- */
if ( imgmethod != 1 )			/* not needed for dvipng */
 if ( !ISDVIPSSWITCH && !isdvipswhich ) { /* no -DDVIPS=\"path/dvips\" */
  isdvipswhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("dvips",&nlocate)) /* try to find dvips path */
  !=   NULL ) {				/* succeeded */
    isdvipspath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(dvipspath,programpath,255); } }/* so copy path from whichpath()*/
/* --- path for convert program --- */
if ( imgmethod != 1 )			/* not needed for dvipng */
 if ( !ISCONVERTSWITCH && !isconvertwhich ){/*no -DCONVERT=\"path/convert\"*/
  isconvertwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("convert",&nlocate)) /* try to find convert */
  !=   NULL ) {				/* succeeded */
    isconvertpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(convertpath,programpath,255); } } /*copy path from whichpath()*/
/* --- adjust imagemethod to comply with available programs --- */
if ( imgmethod != imagemethod		/* ignore recursive call */
&&   imgmethod != 0 ) goto end_of_job;	/* but 0 is a call for both methods*/
if ( imagemethod == 1 )			/* dvipng wanted */
  if ( !isdvipngpath ) {		/* but we have no real path to it */
    if ( imgmethod == 1 ) setpaths(2);	/* try to set dvips/convert paths */
    if ( isdvipspath && isconvertpath )	{ /* and we do have dvips, convert */
      imagemethod = 2;			/* so flip default to use them */
      if ( !ISGAMMA ) strcpy(gamma,CONVERTGAMMA); } }/*default convert gamma*/
if ( imagemethod == 2 )			/* dvips, convert wanted */
  if ( !isdvipspath || !isconvertpath ) {/*but we have no real paths to them*/
    if ( imgmethod == 2 ) setpaths(1);	/* try to set dvipng path */
    if ( isdvipngpath ) {		/* and we do have dvipng path */
      imagemethod = 1;			/* so flip default to use it */
      if ( !ISGAMMA ) strcpy(gamma,DVIPNGGAMMA); } }/* default dvipng gamma */
/* --- back to caller --- */
end_of_job:
  return ( 1 );
} /* --- end-of-function setpaths() --- */

/* ==========================================================================
 * Function:	whichpath ( program, nlocate )
 * Purpose:	determines the path to program with popen("which 'program'")
 * --------------------------------------------------------------------------
 * Arguments:	program (I)	pointer to null-terminated char string
 *				containing program whose path is desired
 *		nlocate (I/0)	addr of int containing NULL to ignore,
 *				or (addr of int containing) 0 to *not*
 *				use locate if which fails.  If non-zero,
 *				use locate if which fails, and return
 *				number of locate lines (if locate succeeds)
 * --------------------------------------------------------------------------
 * Returns:	( char * )	path to program, or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*whichpath ( char *program, int *nlocate )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char pathbuff[256];		/* buffer for returned path */
char	command[256];			/* which program */
FILE    *whichout = NULL;		/* which's stdout */
int	nchars = 0,			/* read whichout one char at a time*/
	pathchar;			/* fgetc(whichout) */
char	*path = NULL;			/* either pathbuff or NULL=error */
char	*locatepath(char *program, int *nlocate);			/* try locate if which fails */
int	islocate = (nlocate==NULL? 1 : (*nlocate!=0?1:0)); /* 1=use locate*/
/* -------------------------------------------------------------------------
Issue which command and read its output
-------------------------------------------------------------------------- */
/* --- check if which() suppressed --- */
if ( !ISWHICH ) return ( NULL );	/*not running which, return failure*/
/* --- first construct the command --- */
if ( isempty(program) ) goto end_of_job; /* no input */
sprintf(command,"which %s",program);	/* construct command */
/* --- use popen() to invoke which --- */
whichout = popen( command, "r" );	/* issue which and capture stdout */
if( whichout == NULL ) goto end_of_job;	/* failed */
/* --- read the pipe one char at a time --- */
while ( (pathchar=fgetc(whichout))	/* get one more char */
!= EOF ) {				/* not at eof yet */
  pathbuff[nchars] = (char)pathchar;	/* store the char */
  if ( ++nchars >= 255 ) break; }	/* don't overflow buffer */
pathbuff[nchars] = '\000';		/* null-terminate path */
trimwhite(pathbuff);			/*remove leading/trailing whitespace*/
/* --- pclose() waits for command to terminate --- */
pclose( whichout );			/* finish */
if ( nchars > 0 ) {			/* found path with which */
  path = pathbuff;			/* give user successful path */
  if ( islocate && nlocate!=NULL ) *nlocate = 0; } /* signal we used which */
end_of_job:
  if ( path == NULL )			/* which failed to find program */
    if ( islocate )			/* and we're using locate */
      path = locatepath(program,nlocate); /* try locate instead */
  return ( path );			/* give caller path to command */
} /* --- end-of-function whichpath() --- */