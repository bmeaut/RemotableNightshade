/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003, 208 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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

#include <string>
#include <cstdlib>
#include <algorithm>

#include "nightshade.h"
#include "utility.h"
#include "stellastro.h"
#include "init_parser.h"
#include "observer.h"
#include "solarsystem.h"
#include "planet.h"
#include "translator.h"
#include "shared_data.h"


Observer::Observer(const SolarSystem &ssystem)
	:ssystem(ssystem), planet(0), artificial_planet(0),
	 longitude(0.), latitude(1e-9), altitude(0),
	 defaultLongitude(0), defaultLatitude(1e-9), defaultAltitude(0),
	 leaveArtificialPlanet(false)

{
	name = "Anonymous_Location";
	flag_move_to = false;
}

Observer::~Observer()
{
	if (artificial_planet) delete artificial_planet;
	artificial_planet = NULL;
}

Vec3d Observer::getCenterVsop87Pos(void) const
{
	return getHomePlanet()->get_heliocentric_ecliptic_pos();
}

double Observer::getDistanceFromCenter(void) const
{
	return getHomePlanet()->getRadius() + (altitude/(1000*AU));
}

Mat4d Observer::getRotLocalToEquatorial(double jd) const
{
	double lat = latitude;
	// TODO: Figure out how to keep continuity in sky as reach poles
	// otherwise sky jumps in rotation when reach poles in equatorial mode
	// This is a kludge
	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;
	return Mat4d::zrotation((getHomePlanet()->getSiderealTime(jd)+longitude)*(M_PI/180.))
		* Mat4d::yrotation((90.-lat)*(M_PI/180.));
}

Mat4d Observer::getRotEquatorialToVsop87(void) const
{
	return getHomePlanet()->getRotEquatorialToVsop87();
}

void Observer::load(const string& file, const string& section)
{
	InitParser conf;
	conf.load(file);
	if (!conf.find_entry(section)) {
		cerr << "ERROR : Can't find observer section " << section << " in file " << file << endl;
		assert(0);
	}
	load(conf, section);
}

bool Observer::setHomePlanet(const string &english_name, float transit_seconds)
{
	Planet *p = NULL;

	if (english_name == "default")
		p = ssystem.searchByEnglishName(m_defaultHome);
	else
		p = ssystem.searchByEnglishName(english_name);

	if (p==NULL)
	{
		cerr << "Can't set home planet to " + english_name + " because it is unknown\n";
  		return false;
	}
	setHomePlanet(p, transit_seconds);
	return true;
}

// transit seconds per AU
void Observer::setHomePlanet(Planet *p, float transit_seconds)
{
	assert(p); // Assertion enables to track bad calls.
	if (planet != p)
	{
		if (planet)
		{
			if (!artificial_planet)
			{
				artificial_planet = new ArtificialPlanet(*planet);
				name = "";
			}
			artificial_planet->setDest(*p);
			if(transit_seconds < 0) transit_seconds = 0;
			//			double distanceAU = Vec3d(planet->get_heliocentric_ecliptic_pos() - p->get_heliocentric_ecliptic_pos()).length();
			//time_to_go = transit_duration = (1000.f * transit_seconds * distanceAU); // milliseconds
			time_to_go = transit_duration = (1000.f * transit_seconds); // milliseconds
    	}
		planet = p; 
		ObserverState state;
		string label =  p->getNameI18n();
		if( label.empty() )
			label = p->getEnglishName();
		strncpy( state.home, label.c_str(), sizeof(state.home) );
		state.home[sizeof(state.home)-1] = '\0';

		SharedData::Instance()->Observer( state );
	}
}

const Planet *Observer::getHomePlanet(void) const 
{
		
	return artificial_planet ? artificial_planet : planet;
}


void Observer::load(const InitParser& conf, const string& section)
{
	name = _(conf.get_str(section, "name").c_str());

	for (string::size_type i=0; i<name.length(); ++i) {
		if (name[i]=='_') name[i]=' ';
	}

	m_defaultHome = conf.get_str(section, "home_planet", "Earth");

	if (!setHomePlanet(m_defaultHome)) {
		planet = ssystem.getEarth();
	}

	cout << "Loading location: \"" << string(name) <<"\", on " << planet->getEnglishName();

//    printf("(home_planet should be: \"%s\" is: \"%s\") ",
//           conf.get_str(section, "home_planet").c_str(),
//           planet->getEnglishName().c_str());

	// Added default value tracking 20070215
	defaultLatitude = set_latitude( get_dec_angle(conf.get_str(section, "latitude")) );
	longitude = defaultLongitude = get_dec_angle(conf.get_str(section, "longitude"));
	altitude = defaultAltitude = conf.get_double(section, "altitude");

	ObserverState state;
	state.latitude  = defaultLatitude;
	state.longitude = longitude;
	state.altitude  = altitude;
	SharedData::Instance()->Observer( state );

	// stop moving and stay put
	flag_move_to = 0;

	set_landscape_name(conf.get_str(section, "landscape_name", "sea"));

	printf(" (landscape is: \"%s\")\n", landscape_name.c_str());

}

void Observer::set_landscape_name(const string s)
{

	// need to lower case name because config file parser lowercases section names
	string x = s;
	transform(x.begin(), x.end(), x.begin(), ::tolower);
	landscape_name = x;

	ReferenceState state;
	strcpy( state.selected_landscape, s.c_str() );
	SharedData::Instance()->References( state );
}

void Observer::save(const string& file, const string& section)
{
	printf("Saving location %s to file %s\n",string(name).c_str(), file.c_str());

	InitParser conf;
	conf.load(file);

	setConf(conf,section);

	conf.save(file);
}


// change settings but don't write to files
void Observer::setConf(InitParser & conf, const string& section)
{

	conf.set_str(section + ":name", string(name));
	conf.set_str(section + ":home_planet", planet->getEnglishName());
	conf.set_str(section + ":latitude",
				 Utility::printAngleDMS(latitude*M_PI/180.0,
										true, true));
	conf.set_str(section + ":longitude",
				 Utility::printAngleDMS(get_longitude()*M_PI/180.0,
										true, true));

	conf.set_double(section + ":altitude", altitude);
	conf.set_str(section + ":landscape_name", landscape_name);

	// saving values so new defaults to track
	defaultLatitude = latitude;
	defaultLongitude = longitude;
	defaultAltitude = altitude;
	m_defaultHome = planet->getEnglishName();

	// TODO: clear out old timezone settings from this section
	// if still in loaded conf?  Potential for confusion.
}


// for platforms without built in timegm function
// taken from the timegm man page
time_t my_timegm (struct tm *tm)
{
	time_t ret;
	char *tz;
	char tmpstr[255];
	tz = getenv((char *)"TZ");
	putenv((char *)"TZ=");
	tzset();
	ret = mktime(tm);
	if (tz) {
		snprintf(tmpstr, 255, "TZ=%s", tz);
		putenv(tmpstr);
	} else
		putenv((char *)"");
	tzset();
	return ret;
}


// move gradually to a new observation location
void Observer::move_to(double lat, double lon, double alt, int duration, const string& _name, bool calculate_duration)
{

	name = _name;

	// If calculate_duration is true, scale the duration based on the amount of change
	// Note: Doesn't look at altitude change
	if( calculate_duration ) {
		float deltaDegrees = abs(latitude - lat) + abs(longitude - lon);
		if(deltaDegrees > 1) duration *= deltaDegrees/10.;
		else duration = 250;  // Small change should be almost instantaneous
	}

	if (duration==0) {
		set_latitude(lat);
		set_longitude(lon);
		set_altitude(alt);

		ObserverState state;
		state.latitude  = lat;
		state.longitude = lon;
		state.altitude  = alt;
		SharedData::Instance()->Observer( state );

		flag_move_to = 0;
		return;
	}

	start_lat = latitude;
	end_lat = lat;

	start_lon = longitude;
	end_lon = lon;

	start_alt = altitude;
	end_alt = alt;

	flag_move_to = 1;

	move_to_coef = 1.0f/duration;
	move_to_mult = 0;

}


string Observer::getHomePlanetEnglishName(void) const 
{
	const Planet *p = getHomePlanet();
	return p ? p->getEnglishName() : "";
}

string Observer::getHomePlanetNameI18n(void) const 
{
	const Planet *p = getHomePlanet();
	return p ? p->getNameI18n() : "";
}

string Observer::get_name(void) const
{
	return name;
}

// for moving observer position gradually
// TODO need to work on direction of motion...
void Observer::update(int delta_time)
{

	if (artificial_planet) {

		if (leaveArtificialPlanet) {

			delete artificial_planet;
			artificial_planet = NULL;
			leaveArtificialPlanet=false;

		} else {

			time_to_go -= delta_time;
			
			if (time_to_go <= 0) {
				time_to_go = 0;
				leaveArtificialPlanet=true;
			}
			if(transit_duration <= 0 ) artificial_planet->computeAverage(0.0);
			else artificial_planet->computeAverage((transit_duration-time_to_go)/transit_duration);
		}
	}

	if (flag_move_to) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_move_to = 0;
		}

		set_latitude( start_lat - move_to_mult*(start_lat-end_lat) );
		longitude = start_lon - move_to_mult*(start_lon-end_lon);
		altitude  = start_alt - move_to_mult*(start_alt-end_alt);

		ObserverState state;
		state.latitude  = latitude;
		state.longitude = longitude;
		state.altitude  = altitude;
		SharedData::Instance()->Observer( state );
	}
}


double Observer::get_longitude(void) const
{

// wrap to proper range
	double tmp = longitude;
	while (tmp > 180) {
		tmp -= 360;
	}
	while (tmp < -180 ) {
		tmp += 360;
	}

	return tmp;

}
