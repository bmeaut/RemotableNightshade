/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Nightshade is a trademark of Digitalis Education Solutions, Inc.
 * See the TRADEMARKS file for trademark usage requirements.
 *
 */


#include <math.h> // fmod
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <limits.h>

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>

#if defined( CYGWIN )
#include <malloc.h>
#endif

#include "utility.h"
#include "nightshade.h"
#include "translator.h"
#include "stellastro.h"
#include "app_settings.h"

#ifdef WIN32
	#include <Windows.h>
#endif

// initialize static variable
string Utility::dataRoot = ".";

string Utility::doubleToString(double d)
{
	std::ostringstream oss;
	oss << d;
	return oss.str();
}

string Utility::intToString(int i)
{
	std::ostringstream oss;
	oss << i;
	return oss.str();
}

int Utility::s_round( float value ) {
	double i_part;

	if (value >= 0) {
		if (modf((double) value, &i_part) >= 0.5)
			i_part += 1;
	} else {
		if (modf((double) value, &i_part) <= -0.5)
			i_part -= 1;
	}

	return ((int) i_part);
}

double Utility::hms_to_rad( unsigned int h, unsigned int m, double s )
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.+s*M_PI/43200.;
}

double Utility::dms_to_rad(int d, int m, double s)
{
	return (double)M_PI/180.*d+(double)M_PI/10800.*m+s*M_PI/648000.;
}

double hms_to_rad(unsigned int h, double m)
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.;
}

double dms_to_rad(int d, double m)
{
	double t = (double)M_PI/180.*d+(double)M_PI/10800.*m;
	return t;
}

void sphe_to_rect(double lng, double lat, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void sphe_to_rect(double lng, double lat, double r, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat * r, sin(lng) * cosLat * r, sin(lat) * r);
}

void sphe_to_rect(float lng, float lat, Vec3f& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void rect_to_sphe(double *lng, double *lat, const Vec3d& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

void rect_to_sphe(float *lng, float *lat, const Vec3f& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}


// Obtains a Vec3f from a string with the form x,y,z
Vec3f Utility::str_to_vec3f(const string& s)
{
	float x, y, z;
	if (s.empty() || (sscanf(s.c_str(),"%f,%f,%f",&x, &y, &z)!=3)) return Vec3f(0.f,0.f,0.f);
	return Vec3f(x,y,z);
}

// Obtains a string from a Vec3f with the form x,y,z
string Utility::vec3f_to_str(const Vec3f& v)
{
	ostringstream os;
	os << v[0] << "," << v[1] << "," << v[2];
	return os.str();
}

// Provide the luminance in cd/m^2 from the magnitude and the surface in arcmin^2
float mag_to_luminance(float mag, float surface)
{
	return expf(-0.4f * 2.3025851f * (mag - (-2.5f * log10f(surface)))) * 108064.73f;
}

// strips trailing whitespaces from buf.
#define iswhite(c)  ((c)== ' ' || (c)=='\t')
static char *trim(char *x)
{
	char *y;

	if (!x)
		return(x);
	y = x + strlen(x)-1;
	while (y >= x && iswhite(*y))
		*y-- = 0; /* skip white space */
	return x;
}



// salta espacios en blanco
static void skipwhite(char **s)
{
	while (iswhite(**s))
		++(*s);
}


double get_dec_angle(const string& str)
{
	const char* s = str.c_str();
	char *mptr, *ptr, *dec, *hh;
	int negative = 0;
	char delim1[] = " :.,;DdHhMm'\n\t\xBA";  // 0xBA was old degree delimiter
	char delim2[] = " NSEWnsew\"\n\t";
	int dghh = 0, minutes = 0;
	double seconds = 0.0, pos;
	short count;

	enum _type {
		HOURS, DEGREES, LAT, LONG
	} type;

	if (s == NULL || !*s)
		return(-0.0);
	count = strlen(s) + 1;
	if ((mptr = (char *) malloc(count)) == NULL)
		return (-0.0);
	ptr = mptr;
	memcpy(ptr, s, count);
	trim(ptr);
	skipwhite(&ptr);

	/* the last letter has precedence over the sign */
	if (strpbrk(ptr,"SsWw") != NULL)
		negative = 1;

	if (*ptr == '+' || *ptr == '-')
		negative = (char) (*ptr++ == '-' ? 1 : negative);
	skipwhite(&ptr);
	if ((hh = strpbrk(ptr,"Hh")) != NULL && hh < ptr + 3)
		type = HOURS;
	else if (strpbrk(ptr,"SsNn") != NULL)
		type = LAT;
	else
		type = DEGREES; /* unspecified, the caller must control it */

	if ((ptr = strtok(ptr,delim1)) != NULL)
		dghh = atoi (ptr);
	else {
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim1)) != NULL) {
		minutes = atoi (ptr);
		if (minutes > 59) {
			free(mptr);
			return (-0.0);
		}
	} else {
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim2)) != NULL) {
		if ((dec = strchr(ptr,',')) != NULL)
			*dec = '.';
		seconds = strtod (ptr, NULL);
		if (seconds >= 60.0) {
			free(mptr);
			return (-0.0);
		}
	}

	if ((ptr = strtok(NULL," \n\t")) != NULL) {
		skipwhite(&ptr);
		if (*ptr == 'S' || *ptr == 'W' || *ptr == 's' || *ptr == 'w') negative = 1;
	}

	free(mptr);

	pos = ((dghh*60+minutes)*60 + seconds) / 3600.0;
	if (type == HOURS && pos > 24.0)
		return (-0.0);
	if (type == LAT && pos > 90.0)
		return (-0.0);
	else if (pos > 180.0)
		return (-0.0);

	if (negative)
		pos = -pos;

	return (pos);

}

//! @brief Print the passed angle with the format ddÃƒÂ‚Ã‚Â°mm'ss(.ss)"
//! @param angle Angle in radian
//! @param decimal Define if 2 decimal must also be printed
//! @param useD Define if letter "d" must be used instead of Â°
//! @return The corresponding string
string Utility::printAngleDMS(double angle, bool decimals, bool useD)
{

	std::ostringstream oss;

	char sign = '+';
	// wchar_t degsign = L'Â°'; ???
	string degsign = "°";
	//char degsign = '\u00B0';
	if (useD) degsign = "d";

	angle *= 180./M_PI;

	if (angle<0) {
		angle *= -1;
		sign = '-';
	}

	if (decimals) {
		int d = (int)(0.5+angle*(60*60*100));
		const int centi = d % 100;
		d /= 100;
		const int s = d % 60;
		d /= 60;
		const int m = d % 60;
		d /= 60;

		oss << sign << d << degsign << m << "\'" << s << "." << centi << "\"";

//		         L"%lc%.2d%lc%.2d'%.2d.%02d\"",
//		         sign, d, degsign, m, s, centi);
	} else {
		int d = (int)(0.5+angle*(60*60));
		const int s = d % 60;
		d /= 60;
		const int m = d % 60;
		d /= 60;

		oss << sign << d << degsign << m << "\'" << s << "\"";
//		         L"%lc%.2d%lc%.2d'%.2d\"",
//		         sign, d, degsign, m, s);
	}
	return oss.str();
}

//! @brief Print the passed angle with the format +hhhmmmss(.ss)"
//! @param angle Angle in radian
//! @param decimals Define if 2 decimal must also be printed
//! @return The corresponding string
string Utility::printAngleHMS(double angle, bool decimals)
{

	std::ostringstream oss;

	angle = fmod(angle,2.0*M_PI);
	if (angle < 0.0) angle += 2.0*M_PI; // range: [0..2.0*M_PI)
	angle *= 12./M_PI; // range: [0..24)
	if (decimals) {
		angle = 0.5+angle*(60*60*100); // range:[0.5,24*60*60*100+0.5)
		if (angle >= (24*60*60*100)) angle -= (24*60*60*100);
		int h = (int)angle;
		const int centi = h % 100;
		h /= 100;
		const int s = h % 60;
		h /= 60;
		const int m = h % 60;
		h /= 60;

		oss << h << "h" << m << "m" << s << "." << centi << "s";
//		         L"%.2dh%.2dm%.2d.%02ds",h,m,s,centi);
	} else {
		angle = 0.5+angle*(60*60); // range:[0.5,24*60*60+0.5)
		if (angle >= (24*60*60)) angle -= (24*60*60);
		int h = (int)angle;
		const int s = h % 60;
		h /= 60;
		const int m = h % 60;
		h /= 60;

		oss << h << "h" << m << "m" << s << "s";
//		         L"%.2dh%.2dm%.2ds",h,m,s);
	}
	return oss.str();
}

double str_to_double(string str)
{

	if (str=="") return 0;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}

double str_to_double(string str, double default_value)
{

	if (str=="") return default_value;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}

// always positive
double str_to_pos_double(string str)
{

	if (str=="") return 0;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	if (dbl < 0 ) dbl *= -1;
	return dbl;
}

bool str_to_bool(string str)
{

	string tmp = str;
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

	if (tmp == "true" || tmp == "1" ) return 1;
	else return 0;

}

bool str_to_bool(string str, bool default_value)
{

	if (str == "" ) return default_value;

	return str_to_bool(str);

}


int str_to_int(string str)
{

	if (str=="") return 0;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}

int str_to_int(string str, int default_value)
{

	if (str=="") return default_value;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}

string double_to_str(double dbl)
{

	std::ostringstream oss;
	oss << dbl;
	return oss.str();

}

long int str_to_long(string str)
{

	if (str=="") return 0;
	long int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}

// convert H:M:S to S
double str_time_to_seconds(string str)
{

	if (str=="") return 0;
	int parsed=0;
	double hours = 0;
	double minutes = 0;
	double seconds = 0;
	std::istringstream dstr( str );
	char tmp;

	if(dstr >> hours) parsed++;
	if(dstr >> tmp);
	if(dstr >> minutes) parsed++;
	if(dstr >> tmp);
	if(dstr >> seconds) parsed++;

	//	cout << "time: " << hours << ":" << minutes << ":" << seconds<< " " << parsed<<endl;
	if( parsed < 2 ) {
		seconds = hours;
	} else if( parsed < 3 ) {
		seconds = hours * 60 + minutes;
	} else {
		seconds = 60*(hours*60 + minutes) + seconds;
	}

	return seconds;
}


int fcompare(const string& _base, const string& _sub)
{
	unsigned int i = 0;
	while (i < _sub.length()) {
		if (toupper(_base[i]) == toupper(_sub[i])) i++;
		else return -1;
	}
	return 0;
}

//! get next larges power of 2
int getNextPowerOf2(int i) {
	for(int n=2; n<INT_MAX; n*=2) {
		if(n > i ) return n;
	}

	return INT_MAX; // failure
}

bool Utility::ProcessExists( const string& procName ){
	//Currently this method supports POSIX OSes only
	// TODO: implementation for Windows
	if( AppSettings::Instance()->Windows() )
		return false;

	boost::filesystem::path proc( "/proc" );
	boost::filesystem::directory_iterator end_itr;

	for( boost::filesystem::directory_iterator itr(proc); itr != end_itr; ++itr ) {
		if( boost::filesystem::is_directory(itr->status()) ) {
			boost::filesystem::path stat( itr->path() / "stat" );
			if( boost::filesystem::exists(stat) ) {
				FILE* fp = fopen( stat.string().c_str(), "r" );
				char buf[256];
			    fread( buf, sizeof(buf), 1, fp );
			    buf[255] = '\0';
			    string procInfo( buf );
			    size_t open = procInfo.find( "(" );
			    size_t close = procInfo.find( ")" );
			    string sub = procInfo.substr( open + 1, close - open - 1);
				fclose(fp);
				if( sub == procName )
					return true;
			}
		}
	}

	return false;
}

// Return the time zone name taken from system locale
string NShadeDateTime::TimeZoneNameFromSystem(double JD)
{
	// Windows will crash if date before 1970
	// And no changes on Linux before that year either
	// TODO: ALSO, on Win XP timezone never changes anyway???
	if (JD < 2440588 ) JD = 2440588;

	// The timezone name depends on the day because of the summer time
	time_t rawtime = TimeTFromJulian(JD);

	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	static char timez[255];
	timez[0] = 0;
	my_strftime(timez, 254, "%Z", timeinfo);
	return timez;
}


// Return the number of hours to add to gmt time to get the local time in day JD
// taking the parameters from system. This takes into account the daylight saving
// time if there is. (positive for Est of GMT)
// TODO : %z in strftime only works posix compliant systems (not Win32)
// Fixed 31-05-2004 Now use the extern variables set by tzset()
// Revised again on 10-14-2010. Extern variable was not working on MinGW. Now use
// Win32 API GetTimeZoneInformation.
float NShadeDateTime::GMTShiftFromSystem(double JD, bool _local)
{
	if( !AppSettings::Instance()->Windows() ) {
		struct tm * timeinfo;

		if (!_local) {
			// JD is UTC
			struct tm rawtime;
			TimeTmFromJulian(JD, &rawtime);
			time_t ltime = timegm(&rawtime);
			timeinfo = localtime(&ltime);
		} 
		else {
			time_t rtime;
			rtime = TimeTFromJulian(JD);
			timeinfo = localtime(&rtime);
		}

		static char heure[20];
		heure[0] = '\0';

		my_strftime(heure, 19, "%z", timeinfo);
		
		heure[5] = '\0';
		float min = 1.f/60.f * atoi(&heure[3]);
		heure[3] = '\0';
		return min + atoi(heure);
	}
	else {
		// Win32 specific. Stub function and structs exist for compilation on other platforms
		TIME_ZONE_INFORMATION info;
		GetTimeZoneInformation(&info);
		return -(info.Bias + info.DaylightBias) / 60;
	}
}

/* Calculate the day of the week.
 * Returns 0 = Sunday .. 6 = Saturday */
unsigned int NShadeDateTime::DayOfWeek (const ln_date *date)
{
    double JD;
    /* get julian day */
    JD = JulianDay(date) + 1.5;
    return (int)JD % 7;
}

/* Calculate julian day from system time. */
double NShadeDateTime::JulianFromSys(void)
{
	ln_date date;
	/* get sys date */
	LnDateFromSys(&date);
	return JulianDay(&date);
}


/* Calculate gmt date from system date.
 * param : date Pointer to store date. */
void NShadeDateTime::LnDateFromSys(ln_date * date)
{
	time_t rawtime;
	struct tm * ptm;

	/* get current time */
	time ( &rawtime );

	/* convert to gmt time representation */
    ptm = gmtime ( &rawtime );

	/* fill in date struct */
	date->seconds = ptm->tm_sec;
	date->minutes = ptm->tm_min;
	date->hours = ptm->tm_hour;
	date->days = ptm->tm_mday;
	date->months = ptm->tm_mon + 1;
	date->years = ptm->tm_year + 1900;
}


// Calculate time_t from julian day
time_t NShadeDateTime::TimeTFromJulian(double JD)
{
	struct tm loctime;
	ln_date date;

	JulianToDate(JD, &date);

	loctime.tm_sec = floor(date.seconds);
	loctime.tm_min = date.minutes;
	loctime.tm_hour = date.hours;
	loctime.tm_mday =date.days;
	loctime.tm_mon = date.months -1;
	loctime.tm_year = date.years - 1900;
	loctime.tm_isdst = -1;

	return mktime(&loctime);
}

double NShadeDateTime::JulianDay (const ln_date * date)
{
	if( !date )
		return 0;
	else
		return JulianDayFromDateTime(date->years, date->months, date->days,
										date->hours, date->minutes, date->seconds);
}

//-----------------------------------------------------------------------------
// convert calendar to Julian date		Year Zero and 4000-year rule added by SGS
// (Julian day number algorithm adapted from Press et al.)
// Adapted JS source by Steve Glennie-Smith to C++ by Trystan Larey-Williams
// This algorithm supports negative Julian dates
//-----------------------------------------------------------------------------
double NShadeDateTime::JulianDayFromDateTime(const int y, const int m, const int d,
										     const int h, const int mn, const double s) {
	double julianYear, julianMonth, centry;
	int correctedDay = d;

	// Gregorian calendar undefined for given date range. Rather than return error
	// just snap to valid date. Snap to the 'further' valid date to support scrolling
	// through dates linearly.
	if( y == 1582 && m == 10 ) {
		if (d > 4 && d < 8)
			correctedDay = 15;
		else if (d > 4 && d < 15)
			correctedDay = 4;
	}

	// The Julian calendar has 13 months
	if( m > 2 ) {
		julianYear = y;
		julianMonth = m + 1;
	}
	else {
		julianYear = y - 1;
		julianMonth = m + 13;
	}

	// Calculate days (integer) portion of Julian date
	double jdays = floor(365.25 * julianYear) + floor(30.6001 * julianMonth) + correctedDay + 1720995;

	// Correct for 'lost 10 days' in Julian -> Gregorian calendar switch
	if( correctedDay + 31 * (m + 12 * y) >= 588829 ) {
		centry = floor(0.01 * julianYear);
		jdays += 2 - centry + floor(0.25 * centry) - floor(0.025 * centry);
	}

	// Correct for half-day offset
	double dayfrac = h / 24.0 - 0.5;
	if (dayfrac < 0.0) {
		dayfrac += 1.0;
		--jdays;
	}

	// Calculate the time (decimal) portion of Julian date
	double jtime = dayfrac + (mn + s / 60.0) / (60.0 * 24.0);

	// Return composite Julian date
	return jdays + jtime;
}

// convert string int ISO 8601-like format [+/-]YYYY-MM-DDThh:mm:ss (no timzone offset)
// to julian day
int NShadeDateTime::StringToJday(string date, double &rjd)
{
	char tmp;
	int year, month, day, hour, minute;
	double second;
	year = month = day = hour = minute = second = 0;

	std::istringstream dstr( date );

	// TODO better error checking
	dstr >> year >> tmp >> month >> tmp >> day >> tmp >> hour >> tmp >> minute >> tmp >> second;

	if ( 	//year < -100000 || year > 100000 ||
			month < 1 || month > 12 ||
	        day < 1 || day > 31 ||
	        hour < 0 || hour > 23 ||
	        minute < 0 || minute > 59 ||
	        second < 0 || second > 59) {
		return 0;
	}

	rjd = JulianDayFromDateTime( year, month, day, hour, minute, second );

	return 1;
}

void NShadeDateTime::TimeTmFromJulian(double JD, struct tm * tm_time)
{
	if( !tm_time )
		return;

	ln_date date;
	JulianToDate(JD, &date);
	tm_time->tm_sec = floor(date.seconds);
	tm_time->tm_min = date.minutes;
	tm_time->tm_hour = date.hours;
	tm_time->tm_mday = date.days;
	tm_time->tm_mon = date.months - 1;
	tm_time->tm_year = date.years - 1900;
	tm_time->tm_isdst = -1;
}

void NShadeDateTime::JulianToDate(double jd, ln_date *date)
{
	if( !date )
		return;

	DateTimeFromJulianDay( jd, &date->years, &date->months, &date->days,
			&date->hours, &date->minutes, &date->seconds);
}

//-----------------------------------------------------------------------------
// convert Julian date to calendar date
// (algorithm adapted from Hatcher, D.A., 1984, QJRAS 25, 53)
// (algorithm adapted from Press et al.)
// Adapted JS source by Steve Glennie-Smith to C++ by Trystan Larey-Williams
// This algorithm supports negative Julian dates
//-----------------------------------------------------------------------------
void NShadeDateTime::DateTimeFromJulianDay(double jd, int *year, int *month, int *day, int *hour, int *minute, double *second)
{
	double j1, j2, j3, j4, j5;

	// get the date from the Julian day number
	double jday = floor(jd);
	double jtime = jd - jday;

	// correction for half day offset
	// SGS: Bug - this was originally after Gregorian date check.  Add 5ms to correct rounding errors
	double dayfract = jtime + 0.500000058;
	if (dayfract >= 1.0)
	{
		dayfract -= 1.0;
		++jday;
	}

	// Gregorian calendar correction.  4000 year correction added by SGS
	if( jday >= 2299161 ){
		// centry is the number of complete *Gregorian centuries* since 2nd March 0000
		double centry = floor( (jday - 1721119.25) / 36524.225 );
		j1 = jday - 2 + centry - floor(0.25*centry) + floor(0.025*centry);
	}
	else
		j1 = jday;

	// Calculate year, month, day
	j2 = j1 + 1524.0;
	j3 = floor(6680.0 + (j2 - 2439992.1) / 365.25);
	j4 = floor(j3 * 365.25);
	j5 = floor((j2 - j4) / 30.6001);
	*day = floor(j2 - j4 - floor (j5*30.6001));
	*month = floor(j5 - 1.0);
	*year = floor(j3 - 4715.0);

	// Back to 12 mo. from 13 mo calendar
	if( *month > 12 )
		*month -= 12;
	if( *month > 2 )
		--(*year);

	// Calculate Hours, minutes, seconds
	int ms  = floor(dayfract * 8640000);
	*second  = ms % 6000;
	ms = (ms - *second) / 6000;
	*second /= 100;
	*minute = ms % 60;
	*hour  = (ms - *minute) / 60;
}

// Return the time in ISO 8601 format that is : %Y-%m-%dT%H:%M:%S
string NShadeDateTime::ISO8601TimeUTC(double jd, bool dateOnly)
{
	int year, month, day, hour, minute;
	double second;
	DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);

	char isotime[255];
	
	if( dateOnly )
		sprintf( isotime, "%d/%d/%d", year, month, day );
	else
		sprintf( isotime, "%d-%d-%dT%d:%d:%d", year, month, day, hour, minute, (int)floor(second) );

	return isotime;
}

// Wrapper around strftime to force ISO format on systems that don't support negative timestamps 
// and/or years with more that four digits. Windows supports neither, and probably never will.
// Note that when compiling on MinGW for a Windows build, the MSCRT implementation of strftime is used.
size_t NShadeDateTime::my_strftime(char *s, size_t max, const char *fmt, const struct tm *tm) {
	if( !tm || !fmt || !s )
		return 0;

	if( AppSettings::Instance()->Windows() ) {
		string sfmt(fmt);
		int size;
		size_t pos = sfmt.find("%Y");

		// Date only in current locale. Can only handle on Windows by returning ISO date.
		if( sfmt == "%x" ) {
			string iso = ISO8601TimeUTC(JulianDayFromDateTime(tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
															  tm->tm_hour, tm->tm_min, tm->tm_sec), true);
			size = std::min(max, iso.length());
			strncpy( s, iso.c_str(), size );
			s[size] = '\0';
		}
		else if( pos != string::npos ){
			// If we're given an explicit format with a year specified, we can splice it out and place it back
			// in after the strftime call. Not looking for little 'y' (two digit year) specifier since it's
			// nonsensical in this application.
			sfmt = sfmt.replace(pos, 2, "YY");
			size = strftime(s, max, sfmt.c_str(), tm);
			string ret(s);
			pos = ret.find("YY");
			ret = ret.replace( pos, 2, boost::lexical_cast<string>(tm->tm_year+1900) );
			size = std::min(max, ret.length());
			strncpy( s, ret.c_str(), size );
			s[size] = '\0';
		}
		else
			size = strftime(s, max, fmt, tm);

		return size;
	}
	else // Non-windows OS
		return strftime(s, max, fmt, tm);
}


// A very basic function to copy file.
// Note: 'to' must contain file name.
// Returns 0 on success, otherwise a non-zero value.
int copy_file(const std::string src, const std::string to, bool verbose) {
	std::ifstream ifs(src.c_str(), std::ios::binary);
	if (!ifs.is_open()) {
		if (verbose)
			std::cerr << "Cannot open " << src << std::endl;
		return -1;
	}
	std::ofstream ofs(to.c_str(), std::ios::binary);
	if (!ofs.is_open()) {
		if (verbose)
			std::cerr << "Cannot open " << to << std::endl;
		ifs.close();
		return -2;
	}

	ofs << ifs.rdbuf();
	ifs.close();
	ofs.close();
	return 0;
}
