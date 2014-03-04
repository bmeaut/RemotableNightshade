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

#ifndef _S_UTILITY_H_
#define _S_UTILITY_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <ctime>
#include "vecmath.h"

#define SPEED_OF_LIGHT 299792.458

typedef struct
{
    int years; 		/*!< Years. All values are valid */
    int months;		/*!< Months. Valid values : 1 (January) - 12 (December) */
    int days; 		/*!< Days. Valid values 1 - 28,29,30,31 Depends on month.*/
    int hours; 		/*!< Hours. Valid values 0 - 23. */
    int minutes; 	/*!< Minutes. Valid values 0 - 59. */
    double seconds;	/*!< Seconds. Valid values 0 - 59.99999.... */
}ln_date;

using namespace std;

template<typename T> class RangeMap {
public:
	RangeMap(T sourceHigh, T sourceLow, T targetHigh, T targetLow) {
		m_sHigh = sourceHigh;
		m_sLow = sourceLow;
		m_tHigh = targetHigh;
		m_tLow = targetLow;
	}
	T Map(T n) {
		return (((n - m_sLow) * (m_tHigh - m_tLow)) / (m_sHigh - m_sLow)) + m_tLow;
	}

private:
	T m_sHigh;
	T m_sLow;
	T m_tHigh;
	T m_tLow;
};

class NShadeDateTime{
public:
	static size_t my_strftime(char *s, size_t max, const char *fmt, const struct tm *tm);

	static string ISO8601TimeUTC(double jd, bool dateOnly = false);
	static void DateTimeFromJulianDay(double jd, int *year, int *month, int *day,
								      int *hour, int *minute, double *second);
	static void JulianToDate(double jd, ln_date *date);
	static void TimeTmFromJulian(double JD, struct tm * tm_time);
	static int StringToJday(string date, double &rjd);
	static double JulianDayFromDateTime(const int, const int, const int,
									    const int, const int, const double);
	static double JulianDay (const ln_date * date);
	static time_t TimeTFromJulian(double JD);
	static void LnDateFromSys(ln_date * date);
	static double JulianFromSys(void);
	static unsigned int DayOfWeek (const ln_date *date);
	static float GMTShiftFromSystem(double JD, bool _local=0);
	static string TimeZoneNameFromSystem(double JD);
	
	// These should probably move elsewhere...
	static double getMaxSimulationJD (void) {
		static double maxJD = JulianDayFromDateTime(1000000, 1, 1, 1, 1, 1);
		return maxJD;
	}

	static double getMinSimulationJD (void) {
		static double minJD = JulianDayFromDateTime(-1000000, 1, 1, 1, 1, 1);
		return minJD;
	}
};

typedef std::map< std::string, std::string > stringHash_t;
typedef stringHash_t::const_iterator stringHashIter_t;

class Utility
{
public:
	static int s_round( float value );

	//! @brief Convert an angle in hms format to radian
	//! @param h hour component
	//! @param m minute component
	//!	@param s second component
	//! @return angle in radian
	static double hms_to_rad(unsigned int h, unsigned int m, double s);

	//! @brief Convert an angle in dms format to radian
	//! @param d degree component
	//! @param m arcmin component
	//!	@param s arcsec component
	//! @return angle in radian
	static double dms_to_rad(int d, int m, double s);

	//! @brief Obtains a Vec3f from a string
	//! @param s the string describing the Vector with the form "x,y,z"
	//! @return The corresponding vector
	static Vec3f str_to_vec3f(const string& s);

	//! @brief Obtains a string from a Vec3f
	//! @param v The vector
	//! @return the string describing the Vector with the form "x,y,z"
	static string vec3f_to_str(const Vec3f& v);

	//! @brief Print the passed angle with the format dd°mm'ss(.ss)"
	//! @param angle Angle in radian
	//! @param decimal Define if 2 decimal must also be printed
	//! @param useD Define if letter "d" must be used instead of °
	//! @return The corresponding string
	static string printAngleDMS(double angle, bool decimals = false, bool useD = false);

	//! @brief Print the passed angle with the format +hh:mm:ss(.ss)"
	//! @param angle Angle in radian
	//! @param decimals Define if 2 decimal must also be printed
	//! @return The corresponding string
	static string printAngleHMS(double angle, bool decimals = false);

	//! @brief Format the double value to a string (with current locale)
	//! @param d The input double value
	//! @return The matching string
	static string doubleToString(double d);

	//! @brief Format the int value to a string (with current locale)
	//! @param i The input int value
	//! @return The matching string
	static string intToString(int i);

	// to get around ridiculous scoping design
	// TODO place at better location

	static string getDataRoot() {
		return Utility::dataRoot;
	}

	static void setDataRoot(const string &root) {
		Utility::dataRoot = root;
	}
	
	static bool ProcessExists( const string& procName );

private:
	static string dataRoot;
};


double hms_to_rad(unsigned int h, double m);
double dms_to_rad(int d, double m);


void sphe_to_rect(double lng, double lat, Vec3d& v);
void sphe_to_rect(double lng, double lat, double r, Vec3d& v);
void sphe_to_rect(float lng, float lat, Vec3f& v);
void rect_to_sphe(double *lng, double *lat, const Vec3d& v);
void rect_to_sphe(float *lng, float *lat, const Vec3f& v);

/* Obtains Latitude, Longitude, RA or Declination from a string. */
double get_dec_angle(const string&);


// Provide the luminance in cd/m^2 from the magnitude and the surface in arcmin^2
float mag_to_luminance(float mag, float surface);

// convert string int ISO 8601-like format [+/-]YYYY-MM-DDThh:mm:ss (no timzone offset)
// to julian day
int string_to_jday(string date, double &jd);

double str_to_double(string str);
double str_to_double(string str, double default_value);

// true, 1 vs false, 0
bool str_to_bool(string str);
bool str_to_bool(string str, bool default_value);

// always positive
double str_to_pos_double(string str);

int str_to_int(string str);
int str_to_int(string str, int default_value);

string double_to_str(double dbl);
long int str_to_long(string str);

double str_time_to_seconds(string str);

int fcompare(const string& _base, const string& _sub);

int getNextPowerOf2(int i);

int copy_file(const std::string src, const std::string to, bool verbose);

#endif
