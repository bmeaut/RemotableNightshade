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

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <string>

#include "init_parser.h"
#include "vecmath.h"

class Planet;
class ArtificialPlanet;

class Observer
{
public:

	Observer(const class SolarSystem &ssystem);
	~Observer();
	bool setHomePlanet(const string &english_name, float transit_seconds=0.f);
	void setHomePlanet(Planet *p, float transit_seconds=10.f);
	const Planet *getHomePlanet(void) const;
	string getHomePlanetEnglishName(void) const;
	string getHomePlanetNameI18n(void) const;

	Vec3d getCenterVsop87Pos(void) const;
	double getDistanceFromCenter(void) const;
	Mat4d getRotLocalToEquatorial(double jd) const;
	Mat4d getRotEquatorialToVsop87(void) const;

	void save(const string& file, const string& section);
	void setConf(InitParser &conf, const string& section);
	void load(const string& file, const string& section);
	void load(const InitParser& conf, const string& section);

	string get_name(void) const;

// hack for selecting home planet on equator 20090416
	double set_latitude(double l) {
		latitude=l;
		if ( latitude==0.0 ) {
			latitude=1e-6;
		}
		return latitude;
	}
	double get_latitude(void) const {
		return latitude;
	}
	double set_longitude(double l) {
		longitude=l;
		return(l);
	}
	double get_longitude(void) const;
	double get_realLongitude(void) const {
		return longitude;
	}
	double set_altitude(double a) {
		altitude=a;
		return(a);
	}
	double get_altitude(void) const {
		return altitude;
	}
	void set_landscape_name(const string s);
	string get_landscape_name(void) const {
		return landscape_name;
	}

	double getDefaultLatitude() {
		return defaultLatitude;
	}
	double getDefaultLongitude() {
		return defaultLongitude;
	}
	double getDefaultAltitude() {
		return defaultAltitude;
	}


	void move_to(double lat, double lon, double alt, int duration, const string& _name,  bool calculate_duration=0);  // duration in ms
	void update(int delta_time);  // for moving observing position

private:
	const class SolarSystem &ssystem;
	string name;			// Position name

	Planet *planet;
    ArtificialPlanet *artificial_planet;
    double time_to_go, transit_duration;  // for guided flying to another body

	double longitude;		// Longitude in degree
	double latitude;		// Latitude in degree
	double altitude;			// Altitude in meter

	double defaultLongitude;
	double defaultLatitude;
	double defaultAltitude;
	string m_defaultHome;
	string landscape_name;

	// for changing position
	bool flag_move_to;
	double start_lat, end_lat;
	double start_lon, end_lon;
	double start_alt, end_alt;
	float move_to_coef, move_to_mult;

	bool leaveArtificialPlanet;  // about to end guided travel to another planet
};

time_t my_timegm (struct tm *tm);


#endif // _OBSERVER_H_
