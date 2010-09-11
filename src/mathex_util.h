/* 
 * File:   mathex_util.h
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 10:14
 */

#ifndef MATHEX_UTIL_H
#define	MATHEX_UTIL_H

#include <stdlib.h>
#include "mathex_string.h"
#include "mathex_global.h"
#include "mathex_main.h"
#include "mathex.h"
#include "mathex_resources.h"

/* ---
 * default uses locatepath() if whichpath() fails
 * ---------------------------------------------- */
#if !defined(NOWHICH)
  #define ISWHICH 1
#else
  #define ISWHICH 0
#endif
#if !defined(NOLOCATE)
  #define ISLOCATE 1
#else
  #define ISLOCATE 0
#endif

#define	MAXEMBEDDED 15			/* 1...#embedded images available */

static	char *embeddedtext[] = { NULL,	/* text of embedded image messages */
  "(1) mathTeX test message:\\"		/* msg#1 mathTeX "okay" test */
  "cgi program running okay.",
  "(2) mathTeX failed: probably due to\\" /* msg#2 general error message */
  "bad paths, permissions, or installation.",
  "(3) Can't mkdir cgi-bin/mathtex/\\"	/* msg#3 can't create cache dir */
  "cache directory: check permissions.",
  "(4) Can't mkdir cgi-bin/tempnam/\\"	/* msg#4 */
  "work directory: check permissions.",
  "(5) Can't cd cgi-bin/tempnam/\\"	/* msg#5 */
  "work directory: check permissions.",
  "(6) Can't fopen(\"latex.tex\") file:\\" /* msg#6 */
  "check permissions.",
  "(7) Can't run latex program:\\"	/* msg#7 */
  "check -DLATEX=\"path\", etc.",
  "(8) latex ran but failed:\\"		/* msg#8 */
  "check your input expression.",
  "(9) Can't run dvipng program:\\"	/* msg#9 */
  "check -DDVIPNG=\"path\", etc.",
  "(10) dvipng ran but failed:",	/* msg#10 */
  "(11) Can't run dvips program:\\"	/* msg#11 */
  "check -DDVIPS=\"path\", etc.",
  "(12) dvips ran but failed:",		/* msg#12 */
  "(13) Can't run convert program:\\"	/* msg#13 */
  "check -DCONVERT=\"path\", etc.",
  "(14) convert ran but failed:",	/* msg#14 */
  "(15) Can't emit cached image:\\"	/* msg#15 */
  "check permissions.",
  NULL } ; /* --- end-of-*embeddedtext[] --- */

#endif	/* MATHEX_UTIL_H */

