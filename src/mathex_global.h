/* 
 * File:   mathex_global.h
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 11:18
 */

#ifndef MATHEX_GLOBAL_H
#define	MATHEX_GLOBAL_H

#define	showmsg(showlevel,label,data)	/* default message format */ \
	if ( msgfp!=NULL && msglevel>=(showlevel) ) { \
	  fprintf(msgfp,(strlen(label)+strlen(data)<64? \
	  "\nmathTeX> %s: %s\n" : "\nmathTeX> %s:\n         %s\n"), \
	  (label),strwrap((data),64,-9)); fflush(msgfp); } else

#define	min2(x,y)  ((x)<(y)? (x):(y))	/* smaller of 2 arguments */

#define	MAXAGE 7200			/* maxage in cache is 7200 secs */

#endif	/* MATHEX_GLOBAL_H */

