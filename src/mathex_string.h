/* 
 * File:   mathex_string.h
 * Author: cyberroadie
 *
 * Created on 05 September 2010, 10:29
 */

#ifndef MATHEX_STRING_H
#define	MATHEX_STRING_H

/* --- strncpy() n bytes and make sure it's null-terminated --- */
#define	strninit(target,source,n) if( (target)!=NULL && (n)>=0 ) { \
	  char *thissource = (source); \
	  (target)[0] = '\000'; \
	  if ( (n)>0 && thissource!=NULL ) { \
	    strncpy((target),thissource,(n)); \
	    (target)[(n)] = '\000'; } }

/* --- strip leading and trailing whitespace --- */
#define	trimwhite(thisstr) if ( (thisstr) != NULL ) { \
	int thislen = strlen(thisstr); \
	while ( --thislen >= 0 ) \
	  if ( isthischar((thisstr)[thislen],WHITESPACE) ) \
	    (thisstr)[thislen] = '\000'; \
	  else break; \
	if ( (thislen = strspn((thisstr),WHITESPACE)) > 0 ) \
	  strcpy((thisstr),(thisstr)+thislen); } else

/* --- check if a string is empty --- */
#define	isempty(s)  ((s)==NULL?1:(*(s)=='\000'?1:0))
/* --- last char of a string --- */
#define	lastchar(s) (isempty(s)?'\000':*((s)+(strlen(s)-1)))

/* --- check for thischar in accept --- */
#define	isthischar(thischar,accept) \
	( (thischar)!='\000' && !isempty(accept) \
	&& strchr((accept),(thischar))!=(char *)NULL )

#define	WHITESPACE  " \t\n\r\f\v"	/* skipped whitespace chars */

#endif	/* MATHEX_STRING_H */

