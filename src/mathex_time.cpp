#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ==========================================================================
 * Function:	timestamp ( tzdelta, ifmt )
 * Purpose:	returns null-terminated character string containing
 *		current date:time stamp as ccyy-mm-dd:hh:mm:ss{am,pm}
 * --------------------------------------------------------------------------
 * Arguments:	tzdelta (I)	integer, positive or negative, containing
 *				containing number of hours to be added or
 *				subtracted from system time (to accommodate
 *				your desired time zone).
 *		ifmt (I)	integer containing 0 for default format
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to null-terminated buffer
 *				containing current date:time stamp
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*timestamp( int tzdelta, int ifmt )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char timebuff[256];		/* date:time buffer back to caller */
/*long	time_val = 0L;*/		/* binary value returned by time() */
time_t	time_val = (time_t)(0);		/* binary value returned by time() */
struct tm *tmstruct=(struct tm *)NULL;// *localtime(); /* interpret time_val */
int	year=0, hour=0,ispm=1,		/* adjust year, and set am/pm hour */
	month=0, day=0,			/* adjust day and month for delta  */
	minute=0,second=0;		/* minute and second not adjusted  */
int	tzadjust( int tzdelta, int *year, int *month, int *day, int *hour );			/* time zone adjustment function */
int	daynumber(int year, int month, int day);			/* #days since Jan 1, 1973 */
static	char *daynames[] = { "Monday", "Tuesday", "Wednesday",
	 "Thursday", "Friday", "Saturday", "Sunday" } ;
static	char *monthnames[] = { "?", "January", "February", "March", "April",
	 "May", "June", "July", "August", "September", "October",
	"November", "December", "?" } ;
/* -------------------------------------------------------------------------
get current date:time, adjust values, and and format stamp
-------------------------------------------------------------------------- */
/* --- first init returned timebuff in case of any error --- */
*timebuff = '\000';
/* --- get current date:time --- */
time((time_t *)(&time_val));		/* get date and time */
tmstruct = localtime((time_t *)(&time_val)); /* interpret time_val */
/* --- extract fields --- */
year  = (int)(tmstruct->tm_year);	/* local copy of year,  0=1900 */
month = (int)(tmstruct->tm_mon) + 1;	/* local copy of month, 1-12 */
day   = (int)(tmstruct->tm_mday);	/* local copy of day,   1-31 */
hour  = (int)(tmstruct->tm_hour);	/* local copy of hour,  0-23 */
minute= (int)(tmstruct->tm_min);	/* local copy of minute,0-59 */
second= (int)(tmstruct->tm_sec);	/* local copy of second,0-59 */
/* --- adjust year --- */
year += 1900;				/* set century in year */
/* --- adjust for timezone --- */
tzadjust(tzdelta,&year,&month,&day,&hour);
/* --- check params --- */
if ( hour<0  || hour>23
||   day<1   || day>31
||   month<1 || month>12
||   year<1973 ) goto end_of_job;
/* --- adjust hour for am/pm --- */
switch ( ifmt )
  {
  default:
  case 0:
    if ( hour < 12 )			/* am check */
     { ispm=0;				/* reset pm flag */
       if ( hour == 0 ) hour = 12; }	/* set 00hrs = 12am */
    if ( hour > 12 ) hour -= 12;	/* pm check sets 13hrs to 1pm, etc */
    break;
  case 4: break;			/* numeric result */
  } /* --- end-of-switch(ifmt) --- */
/* --- format date:time stamp --- */
switch ( ifmt )
  {
  default:
  case 0:  /* --- 2005-03-05:11:49:59am --- */
    sprintf(timebuff,"%04d-%02d-%02d:%02d:%02d:%02d%s",
    year,month,day,hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 1:  /* --- Saturday, March 5, 2005 --- */
    sprintf(timebuff,"%s, %s %d, %d",
    daynames[daynumber(year,month,day)%7],monthnames[month],day,year);
    break;
  case 2: /* --- Saturday, March 5, 2005, 11:49:59am --- */
    sprintf(timebuff,"%s, %s %d, %d, %d:%02d:%02d%s",
    daynames[daynumber(year,month,day)%7],monthnames[month],day,year,
    hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 3: /* --- 11:49:59am --- */
    sprintf(timebuff,"%d:%02d:%02d%s",
    hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 4: /* --- 1231235959 (mmddhhmmss time as integer) --- */
    sprintf(timebuff,"%d%02d%02d%02d%02d",
    month,day,hour,minute,second);
    break;
  } /* --- end-of-switch(ifmt) --- */
end_of_job:
  return ( timebuff );			/* return stamp to caller */
} /* --- end-of-function timestamp() --- */


/* ==========================================================================
 * Function:	tzadjust ( tzdelta, year, month, day, hour )
 * Purpose:	Adjusts hour, and day,month,year if necessary,
 *		by delta increment to accommodate your time zone.
 * --------------------------------------------------------------------------
 * Arguments:	tzdelta (I)	integer, positive or negative, containing
 *				containing number of hours to be added or
 *				subtracted from given time (to accommodate
 *				your desired time zone).
 *		year (I)	addr of int containing        4-digit year
 *		month (I)	addr of int containing month  1=Jan - 12=Dec.
 *		day (I)		addr of int containing day    1-31 for Jan.
 *		hour (I)	addr of int containing hour   0-23
 * Returns:	( int )		1 for success, or 0 for error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	tzadjust ( int tzdelta, int *year, int *month, int *day, int *hour )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	yy = *year, mm = *month, dd = *day, hh = *hour; /*dereference args*/
/* --- calendar data --- */
static	int modays[] =
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
/* --------------------------------------------------------------------------
check args
-------------------------------------------------------------------------- */
if ( mm<1 || mm>12 ) return(-1);	/* bad month */
if ( dd<1 || dd>modays[mm] ) return(-1); /* bad day */
if ( hh<0 || hh>23 ) return(-1);	/* bad hour */
if ( tzdelta>23 || tzdelta<(-23) ) return(-1); /* bad tzdelta */
/* --------------------------------------------------------------------------
make adjustments
-------------------------------------------------------------------------- */
/* --- adjust hour --- */
hh += tzdelta;				/* apply caller's delta */
/* --- adjust for feb 29 --- */
modays[2] = (yy%4==0?29:28);		/* Feb has 29 days in leap years */
/* --- adjust day --- */
if ( hh < 0 )				/* went to preceding day */
  { dd--;  hh += 24; }
if ( hh > 23 )				/* went to next day */
  { dd++;  hh -= 24; }
/* --- adjust month --- */
if ( dd < 1 )				/* went to preceding month */
  { mm--;  dd = modays[mm]; }
if ( dd > modays[mm] )			/* went to next month */
  { mm++;  dd = 1; }
/* --- adjust year --- */
if ( mm < 1 )				/* went to preceding year */
  { yy--;  mm = 12;  dd = modays[mm]; }
if ( mm > 12 )				/* went to next year */
  { yy++;  mm = 1;   dd = 1; }
/* --- back to caller --- */
*year=yy; *month=mm; *day=dd; *hour=hh;	/* reset adjusted args */
return ( 1 );
} /* --- end-of-function tzadjust() --- */


/* ==========================================================================
 * Function:	daynumber ( year, month, day )
 * Purpose:	Returns number of actual calendar days from Jan 1, 1973
 *		to the given date (e.g., bvdaynumber(1974,1,1)=365).
 * --------------------------------------------------------------------------
 * Arguments:	year (I)	int containing year -- may be either 1995 or
 *				95, or may be either 2010 or 110 for those
 *				years.
 *		month (I)	int containing month, 1=Jan thru 12=Dec.
 *		day (I)		int containing day of month, 1-31 for Jan, etc.
 * Returns:	( int )		Number of days from Jan 1, 1973 to given date,
 *				or -1 for error (e.g., year<1973).
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	daynumber ( int year, int month, int day )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- returned value (note: returned as a default "int") --- */
int	ndays;				/* #days since jan 1, year0 */
/* --- initial conditions --- */
static	int year0 = 73, 		/* jan 1 was a monday, 72 was a leap */
	days4yrs = 1461,		/* #days in 4 yrs = 365*4 + 1 */
	days1yr  = 365;
/* --- table of accumulated days per month (last index not used) --- */
static	int modays[] =
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
/* --- variables for #days since day0 --- */
int	nyears, nfouryrs;		/*#years, #4-yr periods since year0*/
/* --------------------------------------------------------------------------
Check input
-------------------------------------------------------------------------- */
if ( month < 1 || month > 12 )		/*month used as index, so must be ok*/
	return ( -1 );			/* otherwise, forget it */
if ( year >= 1900 ) year -= 1900;	/*use two-digit years (3 after 2000)*/
/* --------------------------------------------------------------------------
Find #days since jan 1, 1973
-------------------------------------------------------------------------- */
/* --- figure #complete 4-year periods and #remaining yrs till current --- */
nyears = year - year0;			/* #years since year0 */
if ( nyears < 0 ) return ( -1 );	/* we're not working backwards */
nfouryrs = nyears/4;			/* #complete four-year periods */
nyears -= (4*nfouryrs); 		/* remainder excluding current year*/
/* --- #days from jan 1, year0 till jan 1, this year --- */
ndays = (days4yrs*nfouryrs)		/* #days in 4-yr periods */
      +  (days1yr*nyears);		/* +remaining days */
/*if ( year > 100 ) ndays--;*/		/* subtract leap year for 2000AD */
/* --- add #days within current year --- */
ndays += (modays[month-1] + (day-1));
/* --- may need an extra day if current year is a leap year --- */
if ( nyears == 3 )			/*three preceding yrs so this is 4th*/
    { if ( month > 2 )			/* past feb so need an extra day */
	/*if ( year != 100 )*/		/* unless it's 2000AD */
	  ndays++; }			/* so add it in */
return ( (int)(ndays) );		/* #days back to caller */
} /* --- end-of-function daynumber() --- */
