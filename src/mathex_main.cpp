/* 
 * File:   mathex_main.cpp
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 10:13
 */

#include <cstdlib>
#include <string.h>
char	*strcasestr();
#include "mathex_main.h"
#include "mathex_string.h"

using namespace std;

/* ==========================================================================
 * Function:	main ( argc, argv )
 * Purpose:	driver for mathtex.c
 *		emits, usually to stdout, a gif or png image
 *		of a LaTeX math expression entered either as
 *		    (1)	html query string from a browser (most typical), or
 *		    (2)	a query string from an html <form method="get">
 *			whose <textarea name=TEXTAREANAME>
 *			(usually for demo), or
 *		    (3)	command-line arguments (usually just to test).
 *		If no input supplied, expression defaults to "f(x)=x^2",
 *		treated as test (input method 3).
 * --------------------------------------------------------------------------
 * Command-Line Arguments:
 *		When running mathTeX from the command-line, rather than
 *		from a browser (usually just for testing), syntax is
 *		     ./mathtex	[-m msglevel]	verbosity of debugging output
 *				[-c cachepath ]	name of cache directory
 *				[expression	expression, e.g., x^2+y^2,
 *		-m   0-99, controls verbosity level for debugging output
 *		     >=9 retains all directories and files created
 *		-c   cachepath specifies relative path to cache directory
 * --------------------------------------------------------------------------
 * Exits:	0=success (always exits 0, regardless of success/failure)
 * --------------------------------------------------------------------------
 * Notes:     o For an executable that emits gif images
 *		     cc mathtex.c -o mathtex.cgi
 *		or, alternatively, for an executable that emits png images
 *		     cc -DPNG mathtex.c -o mathtex.cgi
 *		See Notes at top of file for other compile-line -D options.
 *	      o	Move executable to your cgi-bin directory and either
 *		point your browser to it directly in the form
 *		     http://www.yourdomain.com/cgi-bin/mathtex.cgi?f(x)=x^2
 *		or put a tag in your html document of the form
 *		     <img src="/cgi-bin/mathtex.cgi?f(x)=x^2"
 *		       border=0 align=middle>
 *		where f(x)=x^2 (or any other expression) will be displayed
 *		either as a gif or png image (as per -DIMAGETYPE flag).
 * ======================================================================= */
/* --- entry point --- */
int	main ( int argc, char *argv[] )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- expression to be emitted --- */
static	char exprbuffer[MAXEXPRSZ+1] = "\000"; /* input TeX expression */
char	hashexpr[MAXEXPRSZ+1] = "\000";	/* usually use md5 of original expr*/
char	*expression = exprbuffer;	/* ptr to expression */
char	*query = getenv("QUERY_STRING"); /* getenv("QUERY_STRING") result */
int	isquery = 0;			/* true if input from QUERY_STRING */
/* --- preprocess expression for special mathTeX directives, etc --- */
char	*mathprep();			/* preprocess expression */
int	unescape_url();			/* convert %20 to blank space, etc */
int	strreplace(), irep=0;		/* look for keywords in expression */
char	*strchange();			/* edit expression */
char	*getdirective(), argstring[256]; /* look for \density, \usepackage */
int	validate();			/* remove \input, etc */
int	advertisement(), crc16();	/*wrap expression in advertisement*/
char	*adtemplate = NULL;		/* usually use default message */
char	*nomath();			/* remove math chars from string */
/* --- referer initialization variables --- */
char	*http_referer = getenv("HTTP_REFERER"); /* referer using mathTeX */
int	isvalidreferer = 1;		/* can this referer use mathTeX? */
struct	{ char *deny;			/* http_referer can't contain this */
	int msgnumber; }		/* emit invalid_referer_msg[msg#] */
	htaccess[] = {			/* ".htaccess" table for deny */
	#ifdef HTACCESS			/* eg, -DHTACCESS=\"htaccess.txt\" */
	  #include HTACCESS		/* eg,  {"",1},  for no referer */
	#endif
	{ NULL, -999 } };		/* trailer */
/* --- other initialization variables --- */
char	*whichpath();			/* look for latex,dvipng,etc */
int	setpaths();			/* set paths to latex,dvipng,etc */
int	isstrstr();			/* find any snippet in string */
int	isdexists();			/* check whether cache dir exists */
int	perm_all = (S_IRWXU|S_IRWXG|S_IRWXO); /* 777 permissions */
int	readcachefile(), nbytes=0;	/*read expr for -f command-line arg*/
char	*timestamp();			/*for \time (in addition to \today)*/
int	timelimit();			/* just to check stub or built-in */
int	iscolorpackage = 0,		/* true if \usepackage{color} found*/
	iseepicpackage = 0,		/* true if \usepackage{eepic} found*/
	ispict2epackage = 0,		/* true if \usepackage{pict2e}found*/
	ispreviewpackage = 0;		/* true if \usepackage{preview} */
/* --- image rendering --- */
int	mathtex();			/* generate new image using LaTeX */
/* --- image caching --- */
int	emitcache();			/* emit cached image if it exists */
int	maxage = MAXAGE;		/* max-age is typically two hours */
char	*makepath();			/* construct full path/filename.ext*/
char	*md5str(), *md5hash=NULL;	/* md5 has of expression */
int	mathlog();			/* log requests */
/* --- messages --- */
char	*copyright1 =			/* copyright, gnu/gpl notice */
 "+-----------------------------------------------------------------------+\n"
 "|mathTeX vers 1.03, Copyright(c) 2007-2009, John Forkosh Associates, Inc|\n"
 "+-----------------------------------------------------------------------+\n"
 "| mathTeX is free software, licensed to you under terms of the GNU/GPL  |\n"
 "|           and comes with absolutely no warranty whatsoever.           |\n",
 *copyright2 =				/* second part of copyright */
 "|     See http://www.forkosh.com/mathtex.html for complete details.     |\n"
 "+-----------------------------------------------------------------------+";
char	*usage1 =			/* usage instructions */
 "Command-line Usage:\n"
 "  ./mathtex.cgi \"expression\"     expression, e.g., \"x^2+y^2\"\n"
 "            | -f input_file      or read expression from input_file\n"
 "            [ -o output_file ]   write image to ouput_file\n"
 "            [ -m msglevel ]      verbosity / message level\n"
 "            [ -c cache ]         path to cache directory\n",
 *usage2 =				/* second part of usage */
 "Example:\n"
 "  ./mathtex.cgi \"x^2+y^2\" -o equation1\n"
 "creates equation1.gif containing image of x^2+y^2\n";
char	*versiontemplate =		/* mathTeX version "adtemplate" */
 "\\begin{center}\n"
 "\\fbox{\\footnotesize mathTeX version 1.03}\\\\ \\vspace*{-.2in}"
 "%%beginmath%% %%expression%% %%endmath%%\n"
 "\\end{center}\n";
char	whichtemplate[512] =		/* mathTeX which "adtemplate" */
 "\\begin{center}\n"
 "\\fbox{\\footnotesize %%whichpath%%}\\\\ \\vspace*{-.2in}"
 "%%beginmath%% %%expression%% %%endmath%%\n"
 "\\end{center}\n";
int	maxinvalidmsg = 2;		/* max invalid_referer_msg[] index */
char	*invalid_referer_msg[] = {	/* invalid referer messages */
 /* --- message#0 (default invalid_referer message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\mbox{\\textbf{%%referer%%}}\\atop "
 "\\mbox{\\textbf{is not authorized to use mathTeX on this server}}$}",
 /* --- message#1 ("provide http_referer" message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\mbox{\\textbf{To use the public math\\TeX{} server,}}\\atop "
 "\\mbox{\\textbf{please provide your HTTP\\_REFERER}}$}",
 /* --- message#2 ("production use" message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\mbox{\\textbf{For production use, please install mathtex.cgi on}}\\atop "
 "\\mbox{\\textbf{your own server.  See www.forkosh.com/mathtex.html}}$}",
 /* --- end-of-table trailer --- */
 NULL } ; /* --- end-of-invalid_referer_msg[] --- */
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
/* --- set global variables --- */
msgfp = NULL;				/* for query mode output */
msgnumber = 0;				/* no errors to report yet */
if ( imagetype < 1 || imagetype > 2 ) imagetype = 1;   /* keep in bounds */
if ( imagemethod<1 || imagemethod>2 ) imagemethod = 1; /* keep in bounds */
/* ---
 * check QUERY_STRING query for expression
 * --------------------------------------- */
if ( query != NULL )			/* check query string from environ */
  if ( strlen(query) >= 1 )		/* caller gave us a query string */
    { strninit(expression,query,MAXEXPRSZ); /* so use it as expression */
      isquery = 1; }			/* and set isquery flag */
if ( !isquery )				/* empty query string */
  { char *host = getenv("HTTP_HOST"),	/* additional getenv("") results */
    *name = getenv("SERVER_NAME"), *addr = getenv("SERVER_ADDR");
    if ( host!=NULL || name!=NULL || addr!=NULL ) /* assume http query */
      {	isquery = 1;			/* set flag to signal query */
	strcpy(expression,"\\fbox{\\rm No expression supplied}"); }
  } /* --- end-of-if(!isquery) --- */
/* ---
 * process command-line args (only if not a query)
 * ----------------------------------------------- */
if ( !isquery				/* don't have an html query string */
/*&&   argc>1*/ ) {			/* and have command-line args */
  int	argnum = 0;			/*argv[] index for command-line args*/
  int	isendofswitches = 0;		/* -- arg signals no more -switches */
  msglevel = 3;				/*adjust minimum shell message level*/
  msgfp = stdout;			/* for comamnd-line mode output */
  while ( argc > ++argnum ) {		/*check for switches and expression*/
    if ( *argv[argnum] == '-'		/* have some -switch */
    &&   !isendofswitches ) {		/* and haven't seen a -- arg yet*/
      char *field = argv[argnum] + 1;	/* ptr to char(s) following - */
      char flag = tolower((int)(*field)); /* single char following - */
      field++;				/* bump past flag */
      trimwhite(field);			/*remove leading/trailing whitespace*/
      if ( isempty(field) ) {		/* no chars after -flag char, so */
        if ( flag == '-' )		/* have -- arg, so */
          isendofswitches = 1;		/* signal no more -switches */
        else				/* have a single char flag, so */
          field = argv[++argnum]; }	/* arg following flag is its value */
      switch ( flag ) {			/* see what user wants to tell us */
	/* --- unrecognized -switch flag --- */
	default: /*argnum--;*/  break;
	/* --- interpret recognized flags --- */
	case 'c': strcpy(cachepath,field);
		if ( strlen(cachepath) < 1  /* path is an empty string */
		||   strcmp(cachepath,"none") == 0 )  /* or keyword "none" */
		  iscaching = 0;	/* so disable caching */
		break;
	case 'm': msglevel = atoi(field);   break;
	case 'f': nbytes = readcachefile(field,(unsigned char *)exprbuffer);
	        exprbuffer[nbytes] = '\000'; /* null-terminate expression */
		if ( !isempty(outfile) ) break; /* default to same outfile */
	case 'o': strcpy(outfile,field); /* output file for image */
	        trimwhite(outfile);	/*remove leading/trailing whitespace*/
	        break;
	} /* --- end-of-switch(flag) --- */
      } /* --- end-of-if(*argv[argnum]=='-') --- */
    else				/* expression if arg not a -flag */
      strcpy(exprbuffer,argv[argnum]);	/* take last unswitched arg */
    } /* --- end-of-while(argc>++argnum) --- */
  if ( isempty(expression) ) {		/*no expression, show usage and quit*/
    if ( msgfp!=NULL && msglevel>=3 )	/* user needs usage instructions */
      fprintf(msgfp,"%s%s\n%s%s\n",copyright1,copyright2, /*show copyright*/
      usage1,usage2);			/* and show usage */
    goto end_of_job; }			/* quit */
  } /* --- end-of-if(!isquery) --- */
/* ---
 * pre-process expression
 * ---------------------- */
if ( !isempty(TEXTAREANAME) )		/* if <form>'s are allowed */
 if ( isquery )				/*check for <form> on query strings*/
  if(memcmp(expression,TEXTAREANAME,strlen(TEXTAREANAME))==0) { /*have form*/
    char *delim = strchr(expression,'='); /* find = following TEXTAREANAME */
    if ( delim != (char *)NULL )	/* found unescaped equal sign */
      strcpy(expression,delim+1);	/* shift name= out of expression */
    while ( (delim=strchr(expression,'+')) != NULL ) /*unescaped plus sign*/
      *delim = ' ';			/* is "shorthand" for blank space */
    unescape_url(expression);		/*convert %20, etc (twice for form)*/
    } /* --- end-of-if(memcmp()==0) --- */
unescape_url(expression);		/* convert %20 to blank space, etc */
mathprep(expression);			/* convert &lt; to <, etc */
validate(expression);			/* remove \input, etc */
/* ---
 * check if this http_referer is allowed to use mathTeX
 * ---------------------------------------------------- */
msgnumber = 0;				/* default invalid message number */
/* --- see if user fails to match -DREFERER list of valid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( !isempty(REFERER) )		/* nor if compiled w/o -DREFERER= */
    if ( !isstrstr(http_referer,REFERER,0) ) /* invalid http_referer */
      isvalidreferer = 0;		/* signal invalid referer */
/* --- see if user matches -DDENYREFERER list of invalid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( isvalidreferer )			/* or if alteady invalid */
    if ( !isempty(DENYREFERER) )	/*nor if compiled w/o -DDENYREFERER*/
      if ( isstrstr(http_referer,DENYREFERER,0) ) /* invalid http_referer */
        isvalidreferer = 0;		/* signal invalid referer */
/* --- see if user matches -DHTACCESS list of invalid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( isvalidreferer ) {		/* or if alteady invalid */
    int	iaccess=0;			/* htaccess[] index */
    msgnumber = (-999);			/* 0 or positive if match found */
    for ( iaccess=0; msgnumber<0; iaccess++ ) { /*run thru htaccess[] table*/
      char *deny = htaccess[iaccess].deny; /* referer to be denied */
      if ( deny == NULL ) break;	/* null signals end-of-table */
      if ( isempty(deny) )		/* signal to check for no referer */
	{ if ( isempty(http_referer) )	/* http_referer not supplied */
	   msgnumber = htaccess[iaccess].msgnumber; } /* so set message# */
      else				/* have referer to check for */
       if ( !isempty(http_referer) )	/* and have referer to be checked */
	if ( isstrstr(http_referer,deny,0) ) /* invalid http_referer */
	 msgnumber = htaccess[iaccess].msgnumber; /* so set message# */
      } /* --- end-of-for(iaccess) --- */
    if ( msgnumber >= 0 )		/* deny access to this referer */
      isvalidreferer = 0; }		/* signal invalid referer */
/* --- render short expressions even for invalid referers --- */
if ( !isvalidreferer )			/* have an invalied referer */
  if ( MAXINVALID > 0 )			/* but short expressions allowed */
    if ( strlen(expression) <= MAXINVALID ) /* and this one is short enough*/
      isvalidreferer = 1;		/* so let it through */
/* --- check for invalidreferer directive (user wants to see message)  --- */
if ( getdirective(expression,"\\invalidreferer",1,0,1,argstring)
!=   NULL ) {				/* found \invalidreferer directive */
  isvalidreferer = 0;			/* signal invalid referer */
  msgnumber = atoi(argstring); }	/* requested message number */
/* --- substitute invalid referer message if referer is indeed invalid --- */
if ( !isvalidreferer ) {		/* invalid referer detected */
  if ( msgnumber<0 || msgnumber>maxinvalidmsg ) /* check message range */
    msgnumber = 0;			/* default if out-of-bounds */
  strcpy(expression,invalid_referer_msg[msgnumber]); /* choose message */
  strreplace(expression,"%%referer%%",	/* replace actual referer */
    (isempty(http_referer)?"your domain":nomath(http_referer)),0,0); }
msgnumber = 0;				/* reset global message number */
/* ---
 * check for embedded image \message directive (which supercedes everything)
 * ----------------------------------------------------------------------- */
if ( getdirective(expression,"\\message",1,0,1,argstring)
!=   NULL ) {				/* found \message directive */
  if ( msgfp!=NULL && msglevel>=1 )	/* copyright notice */
    fprintf(msgfp,"%s%s\n",copyright1,copyright2); /* always show copyright*/
  msgnumber = atoi(argstring);		/* requested message number */
  emitembedded(msgnumber,(isquery?2:0)); /* emit image, suppress msgfp */
  goto end_of_job; }			/*nothing to do after emitting image*/
/* ---
 * check for \switches directive (which supercedes everything else)
 * ---------------------------------------------------------------- */
if ( strreplace(expression,"\\switches","",0,0) /* remove \switches */
>=   1 ) {				/* found \switches */
  char	*pathsource[] = { "default", "switch", "which", "locate" };
  /*iscaching = 0;*/			/* don't cache \switches image */
  *expression = '\000';			/* reset expression */
  /*setpaths(imagemethod);*/		/* set paths */
  setpaths(0);				/* show _all_ set paths */
  strcat(expression,"\\parstyle");	/* set paragraph mode */
  strcat(expression,"\\small\\tt");	/* set font,size */
  strcat(expression,"\\fparbox{");	/* emit -Dswitches in framed box */
  strcat(expression,"Program image...\\\\\n"); /* image */
  sprintf(expression+strlen(expression),"%s\\\\",argv[0]);
  strcat(expression,"Paths...\\\\\n");	/* paths */
  sprintf(expression+strlen(expression), /* latex path */
    "-DLATEX=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    latexpath,pathsource[islatexpath]);
  sprintf(expression+strlen(expression), /* pdflatex path */
    "-DPDFLATEX=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    pdflatexpath,pathsource[ispdflatexpath]);
  sprintf(expression+strlen(expression), /* dvipng path */
    "-DDVIPNG=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    dvipngpath,pathsource[isdvipngpath]);
  sprintf(expression+strlen(expression), /* dvips path */
    "-DDVIPS=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    dvipspath,pathsource[isdvipspath]);
  sprintf(expression+strlen(expression), /* convert path */
    "-DCONVERT=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    convertpath,pathsource[isconvertpath]);
  strcat(expression,"}");		/* end-of-\fparbox{} */
  } /* --- end-of-if(strreplace("\\switches")>=1) --- */
else					/* no \switches in expression */
 /* ---
  * check for \environment directive (which supercedes everything else)
  * ------------------------------------------------------------------- */
 if ( strreplace(expression,"\\environment","",0,0) /* remove \environment */
 >=   1 ) {				/* found \environment */
  int	ienv = 0;			/* environ[] index */
  /*iscaching = 0;*/			/* don't cache \environment image */
  *expression = '\000';			/* reset expression */
  setpaths(10*latexmethod+imagemethod);	/* set paths */
  strcat(expression,"\\parstyle");	/* set paragraph mode */
  strcat(expression,"\\scriptsize\\tt"); /* set font,size */
  strcat(expression,"\\noindent");	/* don't indent first line */
  strcat(expression,"\\begin{verbatim}"); /* begin verbatim environment */
  for ( ienv=0; ; ienv++ ) {		/* loop over environ[] strings */
    if ( environ[ienv] == (char *)NULL ) break; /* null terminates list */
    if ( *(environ[ienv]) == '\000' ) break; /* double-check empty string */
    sprintf(expression+strlen(expression), /* display environment string */
    "  %2d. %s \n", ienv+1,strwrap(environ[ienv],50,-6));
    /* "%2d.\\ \\ %s\\ \\\\ \n", ienv+1,nomath(environ[ienv])); */
    } /* --- end-of-for(ienv) --- */
  strcat(expression,"\\end{verbatim}");	/* end verbatim environment */
  } /* --- end-of-if(strreplace("\\environment")>=1) --- */
/* ---
 * save copy of expression before further preprocessing for MD5 hash
 * ------------------------------------------------------------------ */
strcpy(hashexpr,expression);		/* save unmodified expr for hash */
/* ---
 * check for \which directive (supercedes everything not above)
 * ------------------------------------------------------------ */
if ( getdirective(expression,"\\which",1,0,1,argstring)
!=   NULL ) {				/* found \which directive */
  int	ispermitted = 1;		/* true if a legitimate request */
  int	nlocate = 1;			/* use locate if which fails */
  char	*path = NULL;			/* whichpath() to argstring program*/
  char	whichmsg[512];			/* displayed message */
  trimwhite(argstring);			/*remove leading/trailing whitespace*/
  if ( isempty(argstring) ) ispermitted = 0; /* arg is an empty string */
  else {				/* have non-empty argstring */
    int	arglen = strlen(argstring);	/* #chars in argstring */
    if ( strcspn(argstring,WHITESPACE) < arglen /* embedded whitespace */
    ||   strcspn(argstring,"{}[]()<>") < arglen /* illegal char */
    ||   strcspn(argstring,"|/\"\'\\") < arglen /* illegal char */
    ||   strcspn(argstring,"`!@%&*+=^")< arglen /* illegal char */
    ) ispermitted = 0;			/*probably not a legitimate request*/
    } /* --- end-of-if/else(isempty(argstring)) --- */
  if ( ispermitted ) {			/* legitimate request */
    path = whichpath(argstring,&nlocate); /* path to argstring program */
    sprintf(whichmsg,			/* display path or "not found" */
      "%s(%s) = %s", (path==NULL||nlocate<1?"which":"locate"),
       argstring,(path!=NULL?path:"not found")); }
  else					/* display "not permitted" message */
    sprintf(whichmsg,"which(%s) = not permitted", argstring);
  strreplace(whichtemplate,"%%whichpath%%",whichmsg,0,0); /*insert message*/
  adtemplate = whichtemplate;		/* set which message */
  adfrequency = 1;			/* force its display */
  /*iscaching = 0;*/			/* don't cache it */
  if ( path != NULL )			/* change \ to $\backslash$ */
    /*strreplace(path,"\\","$\\backslash$",0,0)*/; /* make \ displayable */
  /*strcpy(expression,"\\parstyle\\small\\tt ");*/ /* re-init expression */
  /*strcat(expression,"\\fparbox{");*/	/* emit path in framed box */
  /*sprintf(expression+strlen(expression),*/ /* display path or "not found" */
    /*"which(%s) = %s",argstring,(path!=NULL?path:"not found"));*/
  /*strcat(expression,"}");*/		/* end-of-\fparbox{} */
  } /* --- end-of-if(getdirective("\\which")!=NULL) --- */
/* ---
 * check for embedded directives, remove them and set corresponding values
 * ----------------------------------------------------------------------- */
/* --- check for displaystyle/textstyle/parstyle directives --- */
if ( strreplace(expression,"\\displaystyle","",0,0) /*remove \displaystyle*/
>=   1 ) mathmode = 0;			/* found \displaystyle so set flag*/
if ( strreplace(expression,"\\textstyle","",0,0) /* remove \textstyle */
>=   1 ) mathmode = 1;			/* found \textstyle so reset flag */
if ( strreplace(expression,"\\parstyle","",0,0) /* remove \parstyle */
>=   1 ) mathmode = 2;			/* found \parstyle so reset flag */
if ( strreplace(expression,"\\parmode","",0,0) /*\parmode same as \parstyle*/
>=   1 ) mathmode = 2;			/* found \parmode so reset flag */
/* --- check for quiet/halt directives (\quiet or \halt) --- */
if ( strreplace(expression,"\\quiet","",0,0) /*remove occurrences of \quiet*/
>=   1 ) isquiet = 64;			/* found \quiet so set isquiet flag*/
if ( strreplace(expression,"\\noquiet","",0,0) /* remove \noquiet */
>=   1 ) isquiet = 0;			/* found \noquiet so reset flag */
if ( getdirective(expression,"\\nquiet",1,0,1,argstring) /*\nquiet{#Enters}*/
!=   NULL ) {				/* found \nquiet{#<Enter>'s} */
  isquiet = atoi(argstring); }		/* interpret arg as isquiet value */
/* --- replace \time directive with timestamp (similar to \today) --- */
if ( strstr(expression,"\\time") != NULL ) { /* user wants a timestamp */
  strreplace(expression,"\\time",timestamp(TZDELTA,3),0,0); /* insert time */
  maxage = 5;				/* re-render after 5 secs */
  iscaching = 0; }			/* don't cache \time */
if ( strstr(expression,"\\today") != NULL ) { /* user has a datestamp */
  iscaching = 0; }			/* don't cache \today either */
/* --- check for fontsize directives (\tiny ... \Huge) --- */
if ( mathmode != 2 )			/*latex sets font size in \parstyle*/
  for (irep=1; !isempty(sizedirectives[irep]); irep++) /*1=\tiny...10=\Huge*/
    if ( strreplace(expression,sizedirectives[irep],"",1,0) /*remove \size*/
    >=   1 ) fontsize = irep;		/* found \size so set fontsize */
/* --- check for usepackage directives --- */
while ( npackages < 9 )			/* no more than 9 extra packages */
  if ( getdirective(expression,"\\usepackage",1,0,-1,packages[npackages])
  ==   NULL )  break;			/* no more \usepackage directives */
  else {				/* found another \usepackage */
    /* --- check for optional \usepackage [args] --- */
    *(packargs[npackages]) = '\000';	/* init for no optional args */
    if ( noptional > 0 ) {		/* but we found an optional arg */
      strninit(packargs[npackages],optionalargs[0],255); } /*copy the first*/
    /* --- check for particular packages --- */
    if ( strstr(packages[npackages],"color") != NULL ) /*\usepackage{color}*/
      iscolorpackage = 1;		/* set color package flag */
    if ( strstr(packages[npackages],"eepic") != NULL ) /*\usepackage{eepic}*/
      iseepicpackage = 1;		/* set eepic package flag */
    if ( strstr(packages[npackages],"pict2e") != NULL ) /* {pict2e} */
      ispict2epackage = 1;		/* set pict2e package flag */
    if ( strstr(packages[npackages],"preview") != NULL ) /* {preview} */
      ispreviewpackage = 1;		/* set preview package flag */
    npackages++; }			/* bump package count */
/* --- see if we need any packages not already \usepackage'd by user --- */
if ( npackages < 9			/* have room for one more package */
&&   !iscolorpackage )			/* \usepackage{color} not specified*/
  if ( strstr(expression,"\\color") != NULL ) { /* but \color is used */
    strcpy(packages[npackages],"color"); /* so \usepackage{color} is needed*/
    *(packargs[npackages]) = '\000';	/* no optional args */
    npackages++; }			/* bump package count */
if ( npackages < 8 )			/* have room for two more packages */
  if ( strstr(expression,"picture") != NULL ) { /*picture environment used*/
    ispicture = 1;			/* signal picture environment */
    if ( !ispict2epackage ) {		/*\usepackage{pict2e} not specified*/
      strcpy(packages[npackages],"pict2e"); /* \usepackage{pict2e} needed */
      *(packargs[npackages]) = '\000';	/* no optional args */
      npackages++; }			/* bump package count */
    if ( !ispreviewpackage ) {		/*\usepackage{preview} not specified*/
      strcpy(packages[npackages],"preview"); /*\usepackage{preview} needed*/
      strcpy(packargs[npackages],"active,tightpage"); /* set optional args */
      npackages++; }			/* bump package count */
    } /* --- end-of-if(strstr(expression,"picture")!=NULL) --- */
/* --- check for advertisement directive (\advertisement) --- */
if ( strreplace(expression,"\\advertisement","",0,0) /*remove \advertisement*/
>=   1 ) adfrequency = 1;		/* force advertisement display */
/* --- check for version directive (\version) --- */
if ( strreplace(expression,"\\version","",0,0) /* remove \version */
>=   1 ) { adtemplate = versiontemplate; /* set version message */
  adfrequency = 1; }			/* and force its display */
/* --- check for image type directives (\gif or \png) --- */
if ( strreplace(expression,"\\png","",0,0) /* remove occurrences of \png */
>=   1 ) imagetype = 2;			/* found -png so set png imagetype */
if ( strreplace(expression,"\\gif","",0,0) /* remove occurrences of \gif */
>=   1 ) imagetype = 1;			/* found -gif so set gif imagetype */
/* --- check for latex method directives (\latex or \pdflatex) --- */
if ( strreplace(expression,"\\latex","",0,0) /*remove occurrences of \latex*/
>=   1 ) latexmethod = 1;		/* found \latex so set latex method */
if ( strreplace(expression,"\\pdflatex","",0,0) /* remove \pdflatex */
>=   1 ) latexmethod = 2;		/* found \pdflatex so set pdflatex */
/* --- check for image method directives (\dvips or \dvipng) --- */
if ( strreplace(expression,"\\dvipng","",0,0) /*remove occurrences of \dvipng*/
>=   1 ) {				/* found -dvipng in expression */
  imagemethod = 1;			/* set dvipng imagemethod */
  if ( !ISGAMMA ) strcpy(gamma,DVIPNGGAMMA); } /* default dvipng gamma */
if ( strreplace(expression,"\\dvips","",0,0) /* remove occurrences of -dvips*/
>=   1 ) {				/* found -dvips in expression */
  imagemethod = 2;			/* set dvips/convert imagemethod */
  if ( !ISGAMMA ) strcpy(gamma,CONVERTGAMMA); } /* default convert gamma */
/* --- check for convert/dvipng command's -density/-D parameter --- */
if ( getdirective(expression,"\\density",1,1,1,density) /*look for \density*/
==   NULL )				/* no \density directive */
  getdirective(expression,"\\dpi",1,1,1,density); /* so try \dpi instead */
/* --- check for convert command's \gammacorrection parameter --- */
getdirective(expression,"\\gammacorrection",1,1,1,gamma); /*look for \gamma*/
/* --- check for \cache or \nocache directive --- */
if ( strreplace(expression,"\\cache","",0,0) /* remove \cache */
>=   1 ) iscaching = 1;			/* cache this image */
if ( strreplace(expression,"\\nocache","",0,0) /* remove \nocache */
>=   1 ) iscaching = 0;			/* don't cache this image */
/* ---
 * check for empty expression
 * -------------------------- */
trimwhite(expression);			/*remove leading/trailing whitespace*/
if ( isempty(expression) )		/* no expression supplied */
  if ( adfrequency != 1 )		/* and not a forced message */
    strcpy(expression,"\\fbox{\\rm No expression supplied}"); /* error msg */
/* ---
 * check for "advertisement"
 * ------------------------- */
if ( adfrequency > 0 ) {		/* advertising enabled */
  int	npump = crc16(expression)%16;	/* #times, 0-15, to pump rand() */
  srand(atoi(timestamp(TZDELTA,4)));	/* init rand() with mmddhhmmss */
  while ( npump-- >= 0 ) rand();	/* pre-pump rand() before use */
  if ( rand()%adfrequency == 0 ) {	/* once every adfrequency calls */
    advertisement(expression,adtemplate); /*wrap expression in advertisement*/
    if ( strcasestr(hashexpr,"\\advertisement") == NULL ) /* random ad */
      strchange(0,hashexpr,"\\advertisement "); } } /* signal ad in expr */
/* ---
 * hash the expression for name of cached image file
 * ------------------------------------------------- */
md5hash = md5str(hashexpr);		/* md5 hash of original expression */
/* ---
 * check for \msglevel{msglevel} for debugging
 * ------------------------------------------- */
if ( getdirective(expression,"\\msglevel",1,0,1,argstring)/*look for \msglvl*/
!=   NULL )				/* found \msglevel directive */
  if ( (msglevel = min2(atoi(argstring),MAXMSGLEVEL)) /*assign new msglevel*/
  >    0 )				/* user wants messages */
  if ( isquery )			/* can't write messages to stdout */
    msgfp = fopen(makepath(NULL,md5hash,".out"),"w"); /* file md5hash.out */
/* ---
 * emit initial messages
 * --------------------- */
if ( msgfp!=NULL && msglevel>=1 ) {	/*copyright notice and path to image*/
  fprintf(msgfp,"%s%s\n",copyright1,copyright2); /* always show copyright */
  showmsg(3,"running image",argv[0]);	/* and executable image if verbose */
  showmsg(2,"input expression",hashexpr); /* and input expression */
  if ( msglevel >= 8 ) {		/* and if very verbose */
    fprintf(msgfp,			/* timelimit info */
      "\nmathTeX> %s timelimit: warn/killtime=%d/%d, path=%s\n",
      (timelimit("",-99)==992?"Built-in":"Stub"), warntime, killtime,
      (istimelimitpath?timelimitpath:"none"));
    fflush(msgfp); }			/* flush message buffer */
  } /* --- end-of-if(msgfp!=NULL&&msglevel>=1) --- */
/* -------------------------------------------------------------------------
Emit cached image or render the expression
-------------------------------------------------------------------------- */
if ( md5hash != NULL ) {		/* md5str() almost surely succeeded*/
  /* ---
   * check for cache directory, and create it if it doesn't already exist
   * -------------------------------------------------------------------- */
  if ( isempty(outfile) )		/* no explicit output file given */
    if ( !isdexists(makepath(NULL,NULL,NULL)) ) /* and no cache directory */
      if ( mkdir(makepath(NULL,NULL,NULL),perm_all) /* so make cache dir */
      !=   0 ) {			/* failed, emit embedded error image*/
        emitembedded(CACHEFAILED,isquery); /* emit CACHEFAILED message */
        goto end_of_job; }		/* quit if failed to mkdir cache */
  /* ---
   * emit cached image if it already exists
   * -------------------------------------- */
  if ( isquery )			/* re-render if running from shell */
    if ( iscaching )			/* or if not caching image */
      if(emitcache(makepath(NULL,md5hash,extensions[imagetype]),maxage,0) > 0)
        goto end_of_job;		/* done if cached image emitted */
  /* ---
   * first log caching request for new image
   * --------------------------------------- */
  if ( msglevel >= 1 )			/* check if logging */
    if ( iscaching )			/* only log images being cached */
      if ( isempty(outfile) )		/* and not saved to explicit outfile*/
        mathlog(hashexpr,md5hash);	/* log request for new image */
  /* ---
   * now generate the new image and emit it
   * -------------------------------------- */
  /* --- set up name for temporary work directory --- */
  /*strninit(tempdir,tmpnam(NULL),255);*/ /* maximum name length is 255 */
  strninit(tempdir,md5hash,255);	/* maximum name length is 255 */
  /* --- additional message --- */
  showmsg(9,"working directory",tempdir); /* working directory */
  /* --- mathtex() generates the image and saves it to file --- */
  if ( mathtex(expression,md5hash)	/* generate new image */
  ==   imagetype ) {			/* and emit cache if succeeded */
    /* --- succeeded, so emit the newly-generated image --- */
    if ( isquery )			/* don't emit cache to shell stdout*/
      emitcache(makepath(NULL,md5hash,extensions[imagetype]),maxage,0); }
  else {				/* latex failed to run */
    /* --- failed, so emit the embedded image of an error message --- */
    if ( msgnumber < 1 ) msgnumber = 2;	/* general failure message */
    emitembedded(msgnumber,isquery); }	/* emit message image */
  /* ---
   * remove images not being cached
   * ------------------------------ */
  if ( !iscaching )			/* don't want this image cached */
    if ( isempty(outfile) )		/* unless explicit outfile provided*/
      remove(makepath(NULL,md5hash,extensions[imagetype])); /* remove file */
  } /* --- end-of-if(md5hash!=NULL) --- */
end_of_job:
  if ( msgfp!=NULL && msgfp!=stdout )	/* have an open message file */
    fclose(msgfp);			/* so close it at eoj */
  exit ( 0 );
} /* --- end-of-function main() --- */


