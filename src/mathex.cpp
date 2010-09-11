#include <stdio.h>
#include "mathex.h"


/* ==========================================================================
 * Function:	mathtex ( expression, filename )
 * Purpose:	create image of latex math expression
 * --------------------------------------------------------------------------
 * Arguments:	expression (I)	pointer to null-terminated char string
 *				containing latex math expression
 *		filename (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file (should be the md5 hash
 *				of expression)
 * --------------------------------------------------------------------------
 * Returns:	( int )		imagetype if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o	created file will be filename.gif if imagetype=1
 *		or filename.png if imagetype=2.
 * ======================================================================= */
/* --- entry point --- */
int	mathtex ( char *expression, char *filename )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- latex wrapper document template --- */
char  latexwrapper[MAXEXPRSZ+16384] =
	"\\documentclass[10pt]{article}\n"
	"\\usepackage[latin1]{inputenc}\n"
	"\\usepackage{amsmath}\n"
	"\\usepackage{amsfonts}\n"
	"\\usepackage{amssymb}\n"
	/*"\\usepackage{bm}\n"*/	/* bold math */
	#if defined(USEPACKAGE)		/* cc -DUSEPACKAGE=\"filename\" */
	  #include USEPACKAGE		/* filename with \usepackage{}'s */
	#endif				/* or anything for the preamble */
	"%%usepackage%%\n"
      #if 0
        "\\def\\stackboxes#1{\\vbox{\\def\\\\{\\egroup\\hbox\\bgroup}"
        "\\hbox\\bgroup#1\\egroup}}\n"
        "\\def\\fparbox#1{\\fbox{\\stackboxes{#1}}}\n"
      #endif
	"%%%\\pagestyle{empty}\n"
	"%%pagestyle%%\n"
	"%%previewenviron%%\n"
	"\\begin{document}\n"
	"%%%\\renewcommand{\\input}[1]"	/* don't let users \input{} */
	"{\\mbox{[[$\\backslash$input\\{#1\\} illegal]]}}\n"
	#if defined(NEWCOMMAND)		/* cc -DNEWCOMMAND=\"filename\" */
	  #include NEWCOMMAND		/* filename with \newcommand{}'s */
	#endif				/* or anything for the document */
	"%%fontsize%%\n"
	"%%beginmath%% "
	"%%expression%% \n"		/* \n in case expression contains %*/
	" %%endmath%%\n"
	"\\end{document}\n";
char  errormsg[1024] =			/* latex runs but can't make .dvi */
	"\\fbox{\\footnotesize $\\mbox{Latex failed, probably due to} "
	"\\atop \\mbox{an error in your expression.}$}";
char  usepackage[1024] = "\000";	/* additional \usepackage{}'s */
char  convertargs[1024] =		/* args/switches for convert */
	" -density %%dpi%% -gamma %%gamma%%"
	/*" -border 0% -fuzz 2%"*/
	" -trim -transparent \"#FFFFFF\" ";
char  dvipngargs[1024] =		/* args/switches for dvipng */
	" --%%imagetype%% -D %%dpi%% --gamma %%gamma%%"
	" -bg Transparent -T tight -v"	/* -q for quiet, -v for verbose */
	" -o %%giffile%% ";		/* output filename supplied as -o */
/* --- other variables --- */
static	int iserror = 0;		/* true if procesing error message */
//int	setpaths();			/* set paths for latex,dvipng,etc */
char	*makepath();
char    latexfile[256],giffile[256]="\000"; /*path/filename.ext*/
FILE	*latexfp = NULL;		/* latex wrapper file for expression*/
char	command[2048], subcommand[1024]; /* system(command) runs latex, etc */
char	*beginmath[] = { "\\[", "$", " " }, /* start math mode */
	*endmath[] =   { "\\]", "$", " " }; /* end math mode */
int	perm_all = (S_IRWXU|S_IRWXG|S_IRWXO); /* 777 permissions */
int	dir_stat = 0;			/* 1=mkdir okay, 2=chdir okay */
int	sys_stat = 0;			/* system() return status */
int	isdexists(), isfexists(),	/* check if dir, .dvi file created */
	isnotfound();			/* check .err file for "not found" */
int	strreplace();			/* replace template directives */
int	ipackage = 0;			/* packages[] index 0...npackages-1*/
int	rrmdir();			/* rm -r */
int	gifpathlen = 0;			/* ../ or ../../ prefix of giffile */
int	status = 0;			/* imagetype or 0=error */
//int	timelimit();			/* and using built-in timelimit() */
/* -------------------------------------------------------------------------
Make temporary work directory and change to it
-------------------------------------------------------------------------- */
msgnumber = 0;				/* no error to report yet */
if ( !isdexists(tempdir/*filename*/) )	/* if temp directory doesn't exist */
  if ( mkdir(tempdir/*filename*/,perm_all) /* make temp dirextory */
  !=   0 ) {				/* mkdir failed */
    msgnumber = MKDIRFAILED;		/* set corresponding message number*/
    goto end_of_job; }			/* and quit */
dir_stat++;				/* signal mkdir successful */
if ( chdir(tempdir/*filename*/)		/* cd to temp directory */
!=   0 ) {				/* cd  failed */
    msgnumber = CHDIRFAILED;		/* set corresponding message number*/
    goto end_of_job; }			/* and quit */
dir_stat++;				/* signal chdir successful */
/* -------------------------------------------------------------------------
set up latex directives for user-specified additional \usepackage{}'s
-------------------------------------------------------------------------- */
if ( !iserror )				/* don't \usepackage for error */
 if ( npackages > 0 )			/* have additional packages */
  for ( ipackage=0; ipackage<npackages; ipackage++ ) { /*make \usepackage{}*/
    strcat(usepackage,"\\usepackage");	/* start with a directive */
    if ( !isempty(packargs[ipackage]) ) { /* have an optional arg */
      strcat(usepackage,"[");		/* begin optional argument */
      strcat(usepackage,packargs[ipackage]); /* add optional arg */
      strcat(usepackage,"]"); }		/* finish optional arg */
    strcat(usepackage,"{");		/* begin package name argument */
    strcat(usepackage,packages[ipackage]); /* add package name */
    strcat(usepackage,"}\n"); }		/* finish constructing directive */
/* -------------------------------------------------------------------------
Replace "keywords" in latex template with expression and other directives
-------------------------------------------------------------------------- */
/* --- replace %%pagestyle%% with \pagestyle{empty} if not a picture --- */
  if ( !ispicture )			/* not \begin{picture} environment */
    strreplace(latexwrapper,"%%pagestyle%%","\\pagestyle{empty}",1,0);
/* --- replace %%previewenviron%% if a picture --- */
  if ( ispicture )			/* have \begin{picture} environment */
    strreplace(latexwrapper,"%%previewenviron%%",
    "\\PreviewEnvironment{picture}",1,0);
/* --- replace %%beginmath%%...%%endmath%% with \[...\] or with $...$ --- */
  if ( mathmode<0 || mathmode>2 ) mathmode=0; /* mathmode validity check */
  strreplace(latexwrapper,"%%beginmath%%",beginmath[mathmode],1,0);
  strreplace(latexwrapper,"%%endmath%%",endmath[mathmode],1,0);
/* --- replace %%fontsize%% in template with \tiny...\Huge --- */
  strreplace(latexwrapper,"%%fontsize%%",sizedirectives[fontsize],1,0);
/* --- replace %%usepackage%% in template with extra \usepackage{}'s --- */
  strreplace(latexwrapper,"%%usepackage%%",usepackage,1,0);
/* --- replace %%expression%% in template with expression --- */
  strreplace(latexwrapper,"%%expression%%",expression,1,0);
/* -------------------------------------------------------------------------
Create latex document wrapper file containing expression
-------------------------------------------------------------------------- */
strcpy(latexfile,makepath("","latex",".tex")); /* latex filename latex.tex */
latexfp = fopen(latexfile,"w");		/* open latex file for write */
if ( latexfp == NULL ) {		/* couldn't open latex file */
  msgnumber = FOPENFAILED;		/* set corresponding message number*/
  goto end_of_job; }			/* and quit */
fprintf(latexfp,"%s",latexwrapper);	/* write file */
fclose(latexfp);			/* close file after writing it */
/* -------------------------------------------------------------------------
Set paths to programs we'll need to run
-------------------------------------------------------------------------- */
setpaths(10*latexmethod+imagemethod);
/* -------------------------------------------------------------------------
Execute the latex file
-------------------------------------------------------------------------- */
/* --- initialize system(command); to execute the latex file --- */
*command = '\000';			/* init command as empty string */
/* --- run latex under timelimit if explicitly given -DTIMELIMIT switch --- */
if ( istimelimitpath			/* given explict -DTIMELIMIT path */
&&   warntime > 0			/* and positive warntime, and... */
&&   !iscompiletimelimit ) {		/* not using built-in timelimit() */
  if ( killtime < 1 ) killtime=1;	/* don't make trouble for timelimit*/
  strcat(command,makepath("",timelimitpath,NULL)); /* timelimit program */
  if ( isempty(command) )		/* no path to timelimit */
    warntime = (-1);			/*reset flag to signal no timelimit*/
  else {				/* have path to timelimit program */
    sprintf(command+strlen(command),	/* add timelimit args after path */
      " -t%d -T%d ",warntime,killtime); }
  } /* --- end-of-if(warntime>0) --- */
/* --- path to latex executable image followed by args --- */
if ( latexmethod != 2 )			/* not explicitly using pdflatex */
  strcpy(subcommand,makepath("",latexpath,NULL)); /* running latex program */
else					/* explicitly using pdflatex */
  strcpy(subcommand,makepath("",pdflatexpath,NULL)); /* running pdflatex */
if ( isempty(subcommand) ) {		/* no program path to latex */
  msgnumber = SYLTXFAILED;		/* set corresponding error message */
  goto end_of_job; }			/* signal failure and emit error */
strcat(command,subcommand);		/* add latex path (after timelimit)*/
strcat(command," ");			/* add a blank before latex args */
strcat(command,latexfile);		/* run on latexfile we just wrote */
if ( isquiet > 0 ) {			/* to continue after latex error */
    if ( isquiet > 99 )			/* explicit q requested */
      system("echo \"q\" > reply.txt");	/* reply  q  to latex error prompt */
    else {				/* reply <Enter>'s followed by x */
      int  nquiet = isquiet;		/* this many <Enter>'s before x */
      FILE *freply =fopen("reply.txt","w"); /* open reply.txt for write */
      if ( freply != NULL ) {		/* opened successfully */
	while ( --nquiet >= 0 )		/* nquiet times... */
	  fputs("\n",freply);		/* ...write <Enter> to reply.txt */
	fputs("x",freply);		/* finally followed by an x */
	fclose(freply); } }		/* close reply.txt */
    strcat(command," < reply.txt"); }	/*by redirecting stdin to reply.txt*/
  else strcat(command," < /dev/null");	/* or redirect stdin to /dev/null */
strcat(command," >latex.out 2>latex.err"); /* redirect stdout and stderr */
showmsg(5,"latex command executed",command); /* show latex command executed*/
/* --- execute the latex file --- */
sys_stat = timelimit(command,killtime);	/* throttle the latex command */
if ( msgfp!=NULL && msglevel>=3 )	/* and show command's return status*/
  fprintf(msgfp,"\nmathTeX> system() return status: %d\n", sys_stat);
if ( latexmethod != 2 )			/* ran latex */
  if ( !isfexists(makepath("","latex",".dvi")) )  /* but no latex dvi */
    sys_stat = (-1);			/* signal that latex failed */
if ( latexmethod == 2 )			/* ran pdflatex */
  if ( !isfexists(makepath("","latex",".pdf")) )  /* but no pdflatex pdf */
    sys_stat = (-1);			/* signal that pdflatex failed */
if ( sys_stat == (-1) ) {		/* system() or pdf/latex failed */
  if ( !iserror ) {			/* don't recurse if errormsg fails */
    iserror = 1;			/* set error flag */
    status = mathtex(errormsg,filename); /* recurse just once for error msg*/
    goto end_of_job; }			/* ignore original expression */
  else {				/* ignore 2nd try to recurse */
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYLTXFAILED:	/* system() failed */
      (isnotfound("latex")?SYLTXFAILED:	/* latex program not found */
      LATEXFAILED));			/* latex ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(system()==-1||!isfexists()) --- */
/* -------------------------------------------------------------------------
Construct the output path/filename.[gif,png] for the image file
-------------------------------------------------------------------------- */
if ( isempty(outfile)			/* using default cache directory */
||   !isthischar(*outfile,"/\\") ) {	/* or just given a relative path */
  strcpy(giffile,"../");		/* output file will be in cache */
  if ( iserror ) strcat(giffile,"../");	/* we're in error subdirectory */
  gifpathlen = strlen(giffile); }	/* #chars in ../ or ../../ prefix */
if ( isempty(outfile) )			/* using default output filename */
  strcat(giffile,makepath(NULL,filename,extensions[imagetype]));
else					/* have an explicit output file */
  strcat(giffile,makepath("",outfile,extensions[imagetype]));
showmsg(3,"output image file",giffile+gifpathlen); /* show output filename */
/* -------------------------------------------------------------------------
Run dvipng for .dvi-to-gif/png
-------------------------------------------------------------------------- */
if ( imagemethod == 1 ) {		/*dvipng method requested (default)*/
  /* ---
   * First replace "keywords" in dvipngargs template with actual values
   *------------------------------------------------------------------- */
  /* --- replace %%imagetype%% in dvipng arg template with gif or png --- */
  strreplace(dvipngargs,"%%imagetype%%",extensions[imagetype],1,0);
  /* --- replace %%dpi%% in dvipng arg template with actual dpi --- */
  strreplace(dvipngargs,"%%dpi%%",density,1,0);
  /* --- replace %%gamma%% in dvipng arg template with actual gamma --- */
  strreplace(dvipngargs,"%%gamma%%",gamma,1,0);
  /* --- replace %%giffile%% in dvipng arg template with actual giffile --- */
  strreplace(dvipngargs,"%%giffile%%",giffile,1,0);
  /* ---
   * And run dvipng to convert .dvi file directly to .gif/.png
   *---------------------------------------------------------- */
  strcpy(command,makepath("",dvipngpath,NULL)); /* running dvipng program */
  if ( isempty(command) ) {		/* no program path to dvipng */
    msgnumber = SYPNGFAILED;		/* set corresponding error message */
    goto end_of_job; }			/* signal failure and emit error */
  strcat(command,dvipngargs);		/* add dvipng switches */
  strcat(command,makepath("","latex",".dvi")); /* run dvipng on latex.dvi */
  strcat(command," >dvipng.out 2>dvipng.err"); /* redirect stdout, stderr */
  showmsg(5,"dvipng command executed",command); /* dvipng command executed */
  sys_stat = system(command);		/* execute the dvipng command */
  if ( sys_stat == (-1)			/* system(dvipng) failed */
  ||   !isfexists(giffile) ) {		/*or dvipng failed to create giffile*/
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYPNGFAILED:	/* system() failed */
      (isnotfound("dvipng")?SYPNGFAILED: /* dvipng program not found */
      DVIPNGFAILED));			/* dvipng ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(imagemethod==1) --- */
/* -------------------------------------------------------------------------
Run dvips for .dvi-to-postscript and convert for postscript-to-gif/png
-------------------------------------------------------------------------- */
if ( imagemethod == 2 ) {		/* dvips/convert method requested */
  /* ---
   * First run dvips to convert .dvi file to .ps postscript
   *------------------------------------------------------- */
  if ( latexmethod != 2 ) {		/* only if not using pdflatex */
    strcpy(command,makepath("",dvipspath,NULL)); /* running dvips program */
    if ( isempty(command) ) {		/* no program path to dvips */
      msgnumber = SYPSFAILED;		/* set corresponding error message */
      goto end_of_job; }		/* signal failure and emit error */
    strcat(command," -E ");		/* add -E switch and a blank */
    strcat(command,makepath("","latex",".dvi")); /* run dvips on latex.dvi */
    strcat(command," -o ");		/* to produce output in */
    strcat(command,makepath("","dvips",".ps")); /*dvips.ps postscript file*/
    strcat(command," >dvips.out 2>dvips.err"); /* redirect stdout, stderr */
    showmsg(5,"dvips command executed",command); /* dvips command executed */
    sys_stat = system(command);		/* execute system(dvips) */
    if ( sys_stat == (-1)		/* system(dvips) failed */
    ||   !isfexists(makepath("","dvips",".ps")) ) { /*dvips didn't create .ps*/
      msgnumber =			/* set corresponding message number*/
        (sys_stat==(-1)?SYPSFAILED:	/* system() failed */
        (isnotfound("dvips")?SYPSFAILED: /* dvips program not found */
        DVIPSFAILED));			/* dvips ran but failed */
      goto end_of_job; }		/* and quit */
    } /* --- end-of-if(latexmethod!=2) --- */
  /* ---
   * Then replace "keywords" in convertargs template with actual values
   *------------------------------------------------------------------- */
  /* --- replace %%dpi%% in convert arg template with actual density --- */
  strreplace(convertargs,"%%dpi%%",density,1,0);
  /* --- replace %%gamma%% in convert arg template with actual gamma --- */
  strreplace(convertargs,"%%gamma%%",gamma,1,0);
  /* ---
   * And run convert to convert .ps file to .gif/.png
   *-------------------------------------------------- */
  strcpy(command,makepath("",convertpath,NULL)); /*running convert program*/
  if ( isempty(command) ) {		/* no program path to convert */
    msgnumber = SYCVTFAILED;		/* set corresponding error message */
    goto end_of_job; }			/* signal failure and emit error */
  strcat(command,convertargs);		/* add convert switches */
  if ( latexmethod != 2 )		/* we ran latex and dvips */
    strcat(command,makepath("","dvips",".ps")); /* convert from postscript */
  if ( latexmethod == 2 )		/* we ran pdflatex */
    strcat(command,makepath("","latex",".pdf")); /* convert from pdf */
  strcat(command," ");			/* field separator */
  strcat(command,giffile);		/* followed by ../cache/filename */
  strcat(command," >convert.out 2>convert.err"); /*redirect stdout, stderr*/
  showmsg(5,"convert command executed",command); /*convert command executed*/
  sys_stat = system(command);		/* execute system(convert) command */
  if ( sys_stat == (-1)			/* system(convert) failed */
  ||   !isfexists(giffile) ) {		/* or convert didn't create giffile*/
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYCVTFAILED:	/* system() failed */
      (isnotfound("convert")?SYCVTFAILED: /* convert program not found */
      CONVERTFAILED));			/* convert ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(imagemethod==2) --- */
status = imagetype;			/* signal success */
/* -------------------------------------------------------------------------
Back to caller
-------------------------------------------------------------------------- */
end_of_job:
  if ( dir_stat >= 2 ) chdir("..");	/* return up to original dir */
  if ( dir_stat >= 1 ) rrmdir(tempdir/*filename*/); /*rm -r temp working dir*/
  iserror = 0;				/* always reset error flag */
  return ( status );
} /* --- end-of-function mathtex() --- */
