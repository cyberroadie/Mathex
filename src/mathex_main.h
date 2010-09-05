/* 
 * File:   mathex_main.h
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 10:16
 */

#ifndef MATHEX_MAIN_H
#define	MATHEX_MAIN_H

/* ---
 * internal buffer sizes
 * --------------------- */
#if !defined(MAXEXPRSZ)
#define MAXEXPRSZ (32767)		/*max #bytes in input tex expression*/
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mathex_global.h"

static	FILE *msgfp = NULL;

static	int  msgnumber = 0;		/* embeddedimages() in query mode */


#define	CACHEFAILED 3			/* msg# if mkdir cache failed */

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

/* ---
 * image method info specifying dvipng or dvips/convert
 * use dvipng if -DDVIPNG supplied (or -DDVIPNGMETHOD specified),
 * else use dvips/convert if -DDVIPS supplied (or -DDVIPSMETHOD specified)
 * ----------------------------------------------------------------------- */
#if    defined(DVIPNGMETHOD) || ISDVIPNGSWITCH==1
  #define IMAGEMETHOD 1
#elif  defined(DVIPSMETHOD)  || (ISDVIPSSWITCH==1 && ISDVIPNGSWITCH==0)
  #define IMAGEMETHOD 2
#elif !defined(IMAGEMETHOD)
  #define IMAGEMETHOD 1
#endif
static	int  imagemethod = IMAGEMETHOD;	/* 1=dvipng, 2=dvips/convert */

/* ---
 * debugging and error reporting
 * ----------------------------- */
#if !defined(MSGLEVEL)
  #define MSGLEVEL 1
#endif
#if !defined(MAXMSGLEVEL)
  #define MAXMSGLEVEL 999999
#endif
static	int  msglevel = MSGLEVEL;	/* message level for verbose/debug */

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
 * output file (from shell mode)
 * ----------------------------- */
static	char outfile[256] = "\000";	/* output file, or empty for default*/

/* ---
 * <textarea name=TEXTAREANAME ...> in a <form>
 * -------------------------------------------- */
#if !defined(TEXTAREANAME)
  #define TEXTAREANAME "formdata"
#endif

/* ---
 * comma-separated list of HTTP_REFERER's allowed/denied to use mathTeX
 * -------------------------------------------------------------------- */
#if !defined(REFERER)
  #define REFERER "\000"
#endif
#if !defined(DENYREFERER)
  #define DENYREFERER "\000"
#endif
#if !defined(MAXINVALID)		/* longest length expression */
  #define MAXINVALID 0			/* from an invalid referer */
#endif					/* that will be rendered w/o error */

/* -------------------------------------------------------------------------
Information adjustable by -D switches on compile line
-------------------------------------------------------------------------- */
/* ---
 * executable paths (e.g., path/latex including filename of executable image)
 * ----------------------------------------------------------------------- */
/* --- determine which switches have been explicitly specified --- */
#if defined(LATEX)
  #define ISLATEXSWITCH 1		/* have -DLATEX=\"path/latex\" */
#else
  #define ISLATEXSWITCH 0		/* no -DLATEX switch */
  #define LATEX "/usr/share/texmf/bin/latex" /* default path to latex */
#endif
#if defined(PDFLATEX)
  #define ISPDFLATEXSWITCH 1		/* have -PDFDLATEX=\"path/latex\" */
#else
  #define ISPDFLATEXSWITCH 0		/* no -PDFDLATEX switch */
  #define PDFLATEX "/usr/share/texmf/bin/pdflatex" /* default pdflatex path*/
#endif
#if defined(DVIPNG)
  #define ISDVIPNGSWITCH 1		/* have -DDVIPNG=\"path/dvipng\" */
#else
  #define ISDVIPNGSWITCH 0		/* no -DDVIPNG switch */
  #define DVIPNG "/usr/share/texmf/bin/dvipng"/* default path to dvipng */
#endif
#if defined(DVIPS)
  #define ISDVIPSSWITCH 1		/* have -DDVIPS=\"path/dvips\" */
#else
  #define ISDVIPSSWITCH 0		/* no -DDVIPS switch */
  #define DVIPS "/usr/share/texmf/bin/dvips" /* default path to dvips */
#endif
#if defined(CONVERT)
  #define ISCONVERTSWITCH 1		/* have -DCONVERT=\"path/convert\" */
#else
  #define ISCONVERTSWITCH 0		/* no -DCONVERT switch */
  #define CONVERT "/usr/bin/convert"	/* default path to convert */
#endif
#if defined(TIMELIMIT)
  #define ISTIMELIMITSWITCH 1	    /* have -DTIMELIMIT=\"path/timelimit\" */
#else
  #define ISTIMELIMITSWITCH 0		/* no -DTIMELIMIT switch */
  #define TIMELIMIT "/usr/local/bin/timelimit" /* default path to timelimit*/
#endif
/* --- paths, as specified by -D switches, else from whichpath() --- */
static	char latexpath[256] = LATEX,    pdflatexpath[256] = PDFLATEX,
	dvipngpath[256] = DVIPNG,       dvipspath[256] = DVIPS,
	convertpath[256] = CONVERT,     timelimitpath[256] = TIMELIMIT;
/* --- source of path info: 0=default, 1=switch, 2=which, 3=locate --- */
static	int  islatexpath=ISLATEXSWITCH, ispdflatexpath=ISPDFLATEXSWITCH,
	isdvipngpath=ISDVIPNGSWITCH,    isdvipspath=ISDVIPSSWITCH,
	isconvertpath=ISCONVERTSWITCH,  istimelimitpath=ISTIMELIMITSWITCH;

/* ---
 * latex method info specifying latex,pdflatex
 * ------------------------------------------- */
#if    ISLATEXSWITCH==0 && ISPDFLATEXSWITCH==1
  #define LATEXMETHOD 2
#elif !defined(LATEXMETHOD)
  #define LATEXMETHOD 1
#endif
static	int  latexmethod = LATEXMETHOD;	/* 1=latex, 2=pdflatex */
static	int  ispicture = 0;		/* true for picture environment */

/* ---
 * one image in every ADFREQUENCY will be wrapped inside "advertisement"
 * --------------------------------------------------------------------- */
#if !defined(ADFREQUENCY)
  #define ADFREQUENCY 0			/* never show advertisement if 0 */
#endif
static	int  adfrequency = ADFREQUENCY;	/* advertisement frequency */

/* ---
 * \[ \displaystyle \]  or  $ \textstyle $  or  \parstyle
 * ------------------------------------------------------ */
#if defined(DISPLAYSTYLE)
  #define MATHMODE 0
#endif
#if defined(TEXTSTYLE)
  #define MATHMODE 1
#endif
#if defined(PARSTYLE)
  #define MATHMODE 2
#endif
#if !defined(MATHMODE)
  #define MATHMODE 0
#endif
static	int  mathmode = MATHMODE; /* 0=display 1=text 2=paragraph */

/* ---
 * latex -halt-on-error or quiet
 * ----------------------------- */
#if defined(QUIET)
  #define ISQUIET 99999			/* reply q */
#elif  defined(NOQUIET)
  #define ISQUIET 0			/* reply x (-halt-on-error) */
#elif  defined(NQUIET)
  #define ISQUIET NQUIET		/* reply NQUIET <Enter>'s then x */
#else
  #define ISQUIET 3			/* default reply 3 <Enter>'s then x*/
#endif
static	int  isquiet = ISQUIET;		/* >99=quiet, 0=-halt-on-error */

/* ---
 * time zone delta t (in hours)
 * ---------------------------- */
#if !defined(TZDELTA)
  #define TZDELTA 0
#endif

/* ---
 * font size info 1=\tiny ... 10=\Huge
 * ----------------------------------- */
#if !defined(FONTSIZE)
  #define FONTSIZE 5
#endif
static	int  fontsize = FONTSIZE;	/* 1=tiny ... 10=Huge */
static	char *sizedirectives[] = { NULL, /* fontsize directives */
  "\\tiny", "\\scriptsize", "\\footnotesize", "\\small", "\\normalsize",
  "\\large", "\\Large", "\\LARGE", "\\huge", "\\Huge", NULL };

/* ---
 * additional latex \usepackage{}'s
 * -------------------------------- */
static	int  npackages = 0;		/* number of additional packages */
static	char packages[9][128];		/* additional package names */
static	char packargs[9][128];		/* optional arg for package */

static	int  noptional   = 0;		/* # optional [args] found */

/* ---
 * default -gamma for convert is 0.5, or --gamma for dvipng is 2.5
 * --------------------------------------------------------------- */
#define	DVIPNGGAMMA "2.5"		/* default gamma for dvipng */
#define	CONVERTGAMMA "0.5"
#if !defined(GAMMA)
  #define ISGAMMA 0			/* no -DGAMMA=\"gamma\" switch */
  #if IMAGEMETHOD == 1			/* for dvipng... */
    #define GAMMA DVIPNGGAMMA		/* ...default gamma is 2.5 */
  #elif IMAGEMETHOD == 2		/* for convert... */
    #define GAMMA CONVERTGAMMA		/* ...default gamma is 0.5 */
  #else					/* otherwise... */
    #define GAMMA "1.0"			/* ...default gamma is 1.0 */
  #endif
#else
  #define ISGAMMA 1			/* -DGAMMA=\"gamma\" supplied */
#endif
static	char gamma[256] = GAMMA;	/* -gamma arg for convert() */

/* ---
 * dpi/density info for dvipng/convert
 * ----------------------------------- */
#if !defined(DPI)
  #define DPI "120"
#endif
static	char density[256] = DPI;	/*-D/-density arg for dvipng/convert*/

/* ---
 * timelimit -tWARNTIME -TKILLTIME
 * ------------------------------- */
#if !defined(KILLTIME)			/* no -DKILLTIME given... */
  #define NOKILLTIMESWITCH		/* ...remember that fact below */
  #if ISTIMELIMITSWITCH == 0		/* standalone timelimit disabled */
    #if defined(WARNTIME)		/* have WARNTIME but not KILLTIME */
      #define KILLTIME (WARNTIME)	/* so use WARNTIME for KILLTIME */
    #else				/* neither WARNTIME nor KILLTIME */
      #define KILLTIME (10)		/* default for built-in timelimit()*/
      /*#define KILLTIME (0)*/		/* disable until debugged */
    #endif
  #else					/* using standalone timelimit */
    #define KILLTIME (1)		/* always default -T1 killtime */
  #endif
#endif
#if !defined(WARNTIME)			/*no -DWARNTIME given for timelimit*/
  #if ISTIMELIMITSWITCH == 0		/* and no -DTIMELIMIT path either */
    #define WARNTIME (-1)		/* so standalone timelimit disabled*/
  #else					/*have path to standalone timelimit*/
    #if !defined(NOKILLTIMESWITCH)	/* have KILLTIME but not WARNTIME */
      #define WARNTIME (KILLTIME)	/* so use KILLTIME for WARNTIME */
      #undef  KILLTIME			/* but not for standalone killtime */
      #define KILLTIME (1)		/* default -T1 killtime instead */
    #else				/* neither KILLTIME nor WARNTIME */
      #define WARNTIME (10)		/* so default -t10 warntime */
    #endif
  #endif
#endif
static	int  warntime=WARNTIME, killtime=KILLTIME; /* -twarn -Tkill values */

#endif	/* MATHEX_MAIN_H */

