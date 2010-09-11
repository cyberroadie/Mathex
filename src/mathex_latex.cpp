#include <string.h>
#include "mathex_util.cpp"

/* ==========================================================================
 * Function:	mathprep ( expression )
 * Purpose:	preprocessor for mathTeX input, e.g.,
 *		(a) removes leading/trailing $'s from $$expression$$
 *		(b) xlates &html; special chars to equivalent latex
 *		(c) xlates &#nnn; special chars to equivalent latex
 *		Should only be called once (after unescape_url())
 * --------------------------------------------------------------------------
 * Arguments:	expression (I/O) char * to first char of null-terminated
 *				string containing mathTeX/LaTeX expression,
 *				and returning preprocessed string
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to input expression,
 *				or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o	The ten special symbols  $ & % # _ { } ~ ^ \  are reserved
 *		for use in LaTeX commands.  The corresponding directives
 *		\$ \& \% \# \_ \{ \}  display the first seven, respectively,
 *		and \backslash displays \.  It's not clear to me whether
 *		or not mathprep() should substitute the displayed symbols,
 *		e.g., whether &#36; better xlates to \$ or to $.
 *		Right now, it's the latter.
 * ======================================================================= */
/* --- entry point --- */
char	*mathprep ( char *expression )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	isym=0, inum=0;			/* symbols[], numbers[] indexes */
char	*strchange(int nfirst, char *from, char *to);			/* change leading chars of string */
char	*strwstr(char *string, char *substr, char *white, int *sublen);			/*use strwstr() instead of strstr()*/
int	strreplace(char *string, char *from, char *to,
	int iscase, int nreplace);			/* substitute/from/to/ */
int	ndollars = 0;			/* #leading/trailing $$...$$'s */
int	explen = (isempty(expression)?0:strlen(expression)); /*#input chars*/
/* ---
 * html special/escape chars converted to latex equivalents
 * -------------------------------------------------------- */
char	*htmlsym=NULL;			/* symbols[isym].html */
static	struct { char *html; char *termchar; char *latex; } symbols[] =
 { /* ---------------------------------------------------------
     user-supplied newcommands (different than -DNEWCOMMAND)
   --------------------------------------------------------- */
 #ifdef NEWCOMMANDS			/* -DNEWCOMMANDS=\"filename.h\" */
   #include NEWCOMMANDS
 #endif
   /* ----------------------------------------
    html char termchar  LaTeX equivalent...
   ---------------------------------------- */
   { "&quot",	";",	"\"" },		/* &quot; is first, &#034; */
   { "&amp",	";",	"&" },
   { "&lt",	";",	"<" },
   { "&gt",	";",	">" },
   { "&backslash",";",	"\\" },
   { "&nbsp",	";",	" " /*"~"*/ },
   { "&iexcl",	";",	"{\\mbox{!`}}" },
   { "&brvbar",	";",	"|" },
   { "&plusmn",	";",	"\\pm" },
   { "&sup2",	";",	"{{}^2}" },
   { "&sup3",	";",	"{{}^3}" },
   { "&micro",	";",	"\\mu" },
   { "&sup1",	";",	"{{}^1}" },
   { "&frac14",	";",	"{\\frac14}" },
   { "&frac12",	";",	"{\\frac12}" },
   { "&frac34",	";",	"{\\frac34}" },
   { "&iquest",	";",	"{\\mbox{?`}}" },
   { "&Acirc",	";",	"{\\rm\\hat A}" },
   { "&Atilde",	";",	"{\\rm\\tilde A}" },
   { "&Auml",	";",	"{\\rm\\ddot A}" },
   { "&Aring",	";",	"{\\overset{o}{\\rm A}}" },
   { "&atilde",	";",	"{\\rm\\tilde a}" },
   { "&yuml",	";",	"{\\rm\\ddot y}" },  /* &yuml; is last, &#255; */
   { "&#",	";",	"{[\\&\\#nnn?]}" },  /* all other &#nnn's */
   /* ----------------------------------------
    html tag  termchar  LaTeX equivalent...
   ---------------------------------------- */
   { "< br >",	NULL,	" \000" /*"\\\\"*/ },
   { "< br / >", NULL,	" \000" /*"\\\\"*/ },
   { "< dd >",	NULL,	" \000" },
   { "< / dd >", NULL,	" \000" },
   { "< dl >",	NULL,	" \000" },
   { "< / dl >", NULL,	" \000" },
   { "< p >",	NULL,	" \000" },
   { "< / p >",	NULL,	" \000" },
   /* ---------------------------------------
    garbage  termchar  LaTeX equivalent...
   --------------------------------------- */
   { "< tex >",	NULL,	"\000" },
   { "< / tex >", NULL,	"\000" },
   { NULL,	NULL,	NULL }
 } ; /* --- end-of-symbols[] --- */
/* ---
 * html &#nn chars converted to latex equivalents
 * ---------------------------------------------- */
int	htmlnum=0;			/* numbers[inum].html */
static	struct { int html; char *latex; } numbers[] =
 { /* ---------------------------------------
    html num  LaTeX equivalent...
   --------------------------------------- */
   { 9,		" " },			/* horizontal tab */
   { 10,	" " },			/* line feed */
   { 13,	" " },			/* carriage return */
   { 32,	" " },			/* space */
   { 33,	"!" },			/* exclamation point */
   { 34,	"\"" },			/* &quot; */
   { 35,	"#" },			/* hash mark */
   { 36,	"$" },			/* dollar */
   { 37,	"%" },			/* percent */
   { 38,	"&" },			/* &amp; */
   { 39,	"\'" },			/* apostrophe (single quote) */
   { 40,	")" },			/* left parenthesis */
   { 41,	")" },			/* right parenthesis */
   { 42,	"*" },			/* asterisk */
   { 43,	"+" },			/* plus */
   { 44,	"," },			/* comma */
   { 45,	"-" },			/* hyphen (minus) */
   { 46,	"." },			/* period */
   { 47,	"/" },			/* slash */
   { 58,	":" },			/* colon */
   { 59,	";" },			/* semicolon */
   { 60,	"<" },			/* &lt; */
   { 61,	"=" },			/* = */
   { 62,	">" },			/* &gt; */
   { 63,	"\?" },			/* question mark */
   { 64,	"@" },			/* commercial at sign */
   { 91,	"[" },			/* left square bracket */
   { 92,	"\\" },			/* backslash */
   { 93,	"]" },			/* right square bracket */
   { 94,	"^" },			/* caret */
   { 95,	"_" },			/* underscore */
   { 96,	"`" },			/* grave accent */
   { 123,	"{" },			/* left curly brace */
   { 124,	"|" },			/* vertical bar */
   { 125,	"}" },			/* right curly brace */
   { 126,	"~" },			/* tilde */
   { 160,	"~" },			/* &nbsp; (use tilde for latex) */
   { 166,	"|" },			/* &brvbar; (broken vertical bar) */
   { 173,	"-" },			/* &shy; (soft hyphen) */
   { 177,	"{\\pm}" },		/* &plusmn; (plus or minus) */
   { 215,	"{\\times}" },		/* &times; (plus or minus) */
   { -999,	NULL }
 } ; /* --- end-of-numbers[] --- */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
if ( explen < 1 ) goto end_of_job;	/* no input expression supplied */
/* -------------------------------------------------------------------------
remove leading/trailing $$...$$'s and set mathmode accordingly
-------------------------------------------------------------------------- */
/* --- count and remove leading/trailing $'s from $$expression$$ --- */
while ( explen > 2 )			/* don't exhaust entire expression */
  if ( expression[0] == '$'		/* have leading $ char */
  &&   expression[explen-1] == '$' ) {	/* and trailing $ char */
    explen -= 2;			/* remove leading and trailing $'s */
    strcpy(expression,expression+1);	/* squeeze out leading $ */
    expression[explen] = '\000';	/* and terminate at trailing $ */
    ndollars++; }			/* count another dollar */
  else break;				/* no more $...$ pairs */
/* --- set mathmode for input $$expression$$ --- */
if ( ndollars > 0 )			/* have $$expression$$ input */
  switch ( ndollars ) {			/* set mathmode accordingly */
    case 1: mathmode = 1; break;	/* $...$ is \textstyle */
    case 2: mathmode = 0; break;	/* $$...$$ is \displaystyle */
    case 3: mathmode = 2; break;	/* $$$...$$$ is \parstyle */
    default: break; }			/* I have no idea what you want */
/* --- check for input \[expression\] if no $$'s --- */
if ( ndollars < 1 )			/* not an $$expression$$ */
  if ( explen > 4 )			/* long enough to contain \[...\] */
    if ( strncmp(expression,"\\[",2) == 0 /* so check for leading \[ */
    &&   strncmp(expression+explen-2,"\\]",2) == 0 ) { /* and trailing \] */
      explen -= 4;			/* remove leading/trailing \[...\] */
      strcpy(expression,expression+2);	/* squeeze out leading \[ */
      expression[explen] = '\000';	/* and terminate at trailing \] */
      mathmode = 0; }			/* set \displaystyle */
/* -------------------------------------------------------------------------
run thru table, converting all occurrences of each html to latex equivalent
-------------------------------------------------------------------------- */
for ( isym=0; (htmlsym=symbols[isym].html)!=NULL; isym++ )
  {
  char	*htmlterm = symbols[isym].termchar, /* &symbol; terminator */
	*latexsym = symbols[isym].latex, /* latex replacement */
	errorsym[256];			/* error message replacement */
  int	htmllen = strlen(htmlsym),	/* length of html token */
	wstrlen = htmllen,		/* token length found by strwstr() */
	latexlen = strlen(latexsym);	/* length of latex replacement */
  int	isstrwstr = 1,			/* true to use strwstr() */
	istag = (isthischar(*htmlsym,"<")?1:0),	/*html <tag> starts with <*/
	isamp = (isthischar(*htmlsym,"&")?1:0);	/*html char starts with & */
  char	wstrwhite[128] = "i";		/* whitespace chars for strwstr() */
  char	*expptr = expression,		/* ptr within expression */
	*tokptr = NULL;			/*ptr to token found in expression*/
  /* ---
   * xlate every occurrence of current htmlsym command
   * ------------------------------------------------- */
  skipwhite(htmlsym);			/*skip any bogus leading whitespace*/
  htmllen = wstrlen = strlen(htmlsym);	/*reset length of html token and...*/
  istag = (isthischar(*htmlsym,"<")?1:0); /* ...html <tag> starts with < */
  isamp = (isthischar(*htmlsym,"&")?1:0); /* ...html char starts with & */
  if ( isamp ) isstrwstr = 0;		/* don't use strwstr() for &char */
  if ( istag ) {			/* use strwstr() for <tag> */
    isstrwstr = 1;			/* make sure flag is set true */
    if ( !isempty(htmlterm) )		/* got a term char string */
      strninit(wstrwhite,htmlterm,64);	/* interpret it as whitespace arg */
    htmlterm = NULL; }			/* rather than as a terminater */
  while ( (tokptr=(!isstrwstr?strstr(expptr,htmlsym):  /*strtsr or strwstr*/
  strwstr(expptr,htmlsym,wstrwhite,&wstrlen))) /*looks for another htmlsym*/
  != NULL ) {				/* we found another htmlsym */
    char termchar = *(tokptr+wstrlen),	/* char terminating html sequence */
         prevchar = (tokptr==expptr?' ':*(tokptr-1)); /*char preceding tok*/
    int  toklen = wstrlen;		/* tot length of token+terminator */
    /* --- ignore match if leading char escaped (not really a match) --- */
    if ( isthischar(prevchar,"\\") ) {	/* inline symbol escaped */
        expptr = tokptr+toklen;		/*just resume search after literal*/
        continue; }			/* but don't replace it */
    /* --- ignore match if it's just a prefix of a longer expression --- */
    if ( !istag )			/*<br>-type command can't be prefix*/
      if ( isalpha((int)termchar) ) {	/*we just have prefix of longer sym*/
        expptr = tokptr+toklen;		/* just resume search after prefix */
        continue; }			/* but don't replace it */
    /* --- check for &# prefix signalling &#nnn --- */
    if ( strcmp(htmlsym,"&#") == 0 ) {	/* replacing special &#nnn; chars */
      /* --- accumulate chars comprising number following &# --- */
      char anum[32];			/* chars comprising number after &# */
      inum = 0;				/* no chars accumulated yet */
      while ( termchar != '\000' ) {	/* don't go past end-of-string */
        if ( !isdigit((int)termchar) ) break; /* and don't go past digits */
        if ( inum > 10 ) break;		/* some syntax error in expression */
        anum[inum] = termchar;		/* accumulate this digit */
        inum++;  toklen++;		/* bump field length, token length */
        termchar = *(tokptr+toklen); }	/* char terminating html sequence */
      anum[inum] = '\000';		/* null-terminate anum */
      /* --- look up &#nnn in number[] table --- */
      htmlnum = atoi(anum);		/* convert anum[] to an integer */
      latexsym = errorsym;		/* init latex replacement for error*/
      strninit(latexsym,symbols[isym].latex,128); /* init error message */
      strreplace(latexsym,"nnn",anum,1,1); /* place actual &#num in message*/
      latexlen = strlen(latexsym);	/* and length of latex replacement */
      for ( inum=0; numbers[inum].html>=0; inum++ ) /* run thru numbers[] */
        if ( htmlnum ==  numbers[inum].html ) { /* till we find a match */
	  latexsym = numbers[inum].latex; /* latex replacement */
	  latexlen = strlen(latexsym);	/* length of latex replacement */
          break; }			/* no need to look any further */
      } /* --- end-of-if(strcmp(htmlsym,"&#")==0) --- */
    /* --- check for optional ; terminator after &symbol --- */
    if ( !istag )			/* html <tag> doesn't have term. */
      if ( termchar != '\000' )		/* token not at end of expression */
        if ( !isempty(htmlterm) )	/* sequence may have terminator */
          toklen += (isthischar(termchar,htmlterm)?1:0); /*add terminator*/
    /* --- replace html command with latex equivalent --- */
    strchange(toklen,tokptr,latexsym);	/* replace html symbol with latex */
    expptr = tokptr + latexlen;		/* resume search after replacement */
    } /* --- end-of-while(tokptr!=NULL) --- */
  } /* --- end-of-for(isymbol) --- */
/* -------------------------------------------------------------------------
back to caller with preprocessed expression
-------------------------------------------------------------------------- */
trimwhite(expression);			/*remove leading/trailing whitespace*/
showmsg(99,"mathprep expression",expression); /*show preprocessed expression*/
end_of_job:
  return ( expression );
} /* --- end-of-function mathprep() --- */

/* ==========================================================================
 * Function:	nomath ( s )
 * Purpose:	Removes/replaces any LaTeX math chars in s
 *		so that s can be rendered in paragraph mode.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		char * to null-terminated string
 *				whose math chars are to be removed/replaced
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to "cleaned" copy of s
 *				or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The returned pointer addresses a static buffer,
 *		so don't call nomath() again until you're finished
 *		with output from the preceding call.
 * ======================================================================= */
/* --- entry point --- */
char	*nomath ( char *s )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char sbuff[4096];		/* copy of s with no math chars */
int	strreplace(char *string, char *from, char *to,
	int iscase, int nreplace);			/* replace _ with -, etc */
/* -------------------------------------------------------------------------
Make a clean copy of s
-------------------------------------------------------------------------- */
/* --- check input --- */
*sbuff = '\000';			/* initialize in case of error */
if ( isempty(s) ) goto end_of_job;	/* no input */
/* --- start with copy of s --- */
strninit(sbuff,s,3000);			/* leave room for replacements */
/* --- make some replacements (*must* replace \ first) --- */
strreplace(sbuff,"\\","\\textbackslash ",0,0); /* change all \'s to text */
strreplace(sbuff,"_","\\textunderscore ",0,0); /* change all _'s to text */
strreplace(sbuff,"<","\\textlangle ",0,0); /* change all <'s to text */
strreplace(sbuff,">","\\textrangle ",0,0); /* change all >'s to text */
strreplace(sbuff,"$","\\textdollar ",0,0); /* change all $'s to text */
strreplace(sbuff,"&","\\&",0,0);           /* change every & to \& */
strreplace(sbuff,"%","\\%",0,0);           /* change every % to \% */
strreplace(sbuff,"#","\\#",0,0);           /* change every # to \# */
strreplace(sbuff,"~","\\~",0,0);           /* change every ~ to \~ */
strreplace(sbuff,"{","\\{",0,0);           /* change every { to \{ */
strreplace(sbuff,"}","\\}",0,0);           /* change every } to \} */
strreplace(sbuff,"^","\\ensuremath{\\widehat{~}}",0,0); /* change every ^ */
end_of_job:
  return ( sbuff );			/* back with clean copy of s */
} /* --- end-of-function nomath() --- */
