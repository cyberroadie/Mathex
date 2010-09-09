/* 
 * File:   mathex.h
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 11:06
 */

#ifndef MATHEX_H
#define	MATHEX_H

#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mathex_string.h"
#include "mathex_global.h"

#define	MKDIRFAILED 4			/* msg# if mkdir tempdir failed */
#define	CHDIRFAILED 5			/* msg# if chdir tempdir failed */
#define	FOPENFAILED 6			/* msg# if fopen(latex.tex) failed */
#define	SYLTXFAILED 7			/* msg# if system(latex) failed */
#define	LATEXFAILED 8			/* msg# if latex failed */
#define	SYPNGFAILED 9			/* msg# if system(dvipng) failed */
#define	DVIPNGFAILED 10			/* msg# if dvipng failed */
#define	SYPSFAILED 11			/* msg# if system(dvips) failed */
#define	DVIPSFAILED 12			/* msg# if dvips failed */
#define	SYCVTFAILED 13			/* msg# if system(convert) failed */
#define	CONVERTFAILED 14		/* msg# if convert failed */
#define	EMITFAILED 15

/* ---
 * internal buffer sizes
 * --------------------- */
#if !defined(MAXEXPRSZ)
  #define MAXEXPRSZ (32767)		/*max #bytes in input tex expression*/
#endif
#if !defined(MAXGIFSZ)
  #define MAXGIFSZ (131072)		/* max #bytes in output GIF image */
#endif

/* ---
 * temporary work directory
 * ------------------------ */
static	char tempdir[256] = "\000";	/* temporary work directory */

/* ---
 * compile (or not) built-in timelimit() code
 * ------------------------------------------ */
#if ISTIMELIMITSWITCH==0 && KILLTIME>0	/*no -DTIMELIMIT, but have KILLTIME*/
  #define ISCOMPILETIMELIMIT 1		/* so we need built-in code */
#else
  #define ISCOMPILETIMELIMIT 0		/* else we don't need built-in code*/
#endif
static	int  iscompiletimelimit=ISCOMPILETIMELIMIT; /* 1=use timelimit() */
#if ISCOMPILETIMELIMIT			/*header files, etc for timelimit()*/
  /* --- header files for timelimit() --- */
  #include <sys/signal.h>
  #include <sys/wait.h>
  #include <unistd.h>
  #include <signal.h>
  #include <sysexits.h>
  /*#define EX_OSERR	71*/		/* system error (e.g., can't fork) */
  #define HAVE_SIGACTION
  /* --- global variables for timelimit() --- */
  volatile int	fdone, falarm, fsig, sigcaught;
#endif

#endif	/* MATHEX_H */

