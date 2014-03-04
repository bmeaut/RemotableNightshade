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

#ifndef _SOLARSYSTEM_H_
#define _SOLARSYSTEM_H_

#include <vector>
#include <functional>

#include "planet.h"
#include "nightshade.h"
#include "object.h"
#include "orbit.h"
#include "translator.h"
#include "shared_data.h"
#include "loadingbar.h"

#define SOLAR_MASS 1.989e30
#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22
#define MARS_MASS  0.64185e24

using namespace std;

typedef std::map< std::string, Planet * > planetHash_t;
typedef planetHash_t::const_iterator planetHashIter_t;


class SolarSystem
{
public:
	SolarSystem();
	virtual ~SolarSystem();

	void update(int delta_time, Navigator* nav);

	// Draw all the elements of the solar system
	// Return the maximum squared distance in pixels that any planet
	// has travelled since the last update.
	double draw(Projector * du, const Navigator * nav,
	            const ToneReproductor* eye,
	            bool flag_point,
	            bool drawHomePlanet );

	// Load the bodies data from a file
	void load(const string& planetfile,  LoadingBar& lb);

	// load one object from a hash (returns error message if any)
	// this public method always adds bodies as deletable
	string addBody(stringHash_t & param) { return addBody(param, true); }

	// remove an object (returns error message if any)
	// only allows removal of non-ini file objects
	string removeBody(string name);

	// remove all bodies added after ssystem.ini file was read
	string removeSupplementalBodies(const Planet *home_planet);

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	string getPlanetHashString();  // locale and ssystem.ini names, newline delimiter, for tui

	void setFont(float font_size, const string& font_name);
	void setLabelColor(const Vec3f& c) {
		Planet::set_label_color(c);
		SettingsState state;
		state.m_state.planet_names[0] = c[0];
		state.m_state.planet_names[1] = c[1];
		state.m_state.planet_names[2] = c[2];
		SharedData::Instance()->Settings(state);
	}
	const Vec3f& getLabelColor(void) const {
		return Planet::getLabelColor();
	}
	void setPlanetOrbitColor(const Vec3f& c) {
		Planet::set_planet_orbit_color(c);
		SettingsState state;
		state.m_state.planet_orbits[0] = c[0];
		state.m_state.planet_orbits[1] = c[1];
		state.m_state.planet_orbits[2] = c[2];
		SharedData::Instance()->Settings(state);
	}
	void setSatelliteOrbitColor(const Vec3f& c) {
		Planet::set_satellite_orbit_color(c);
		SettingsState state;
		state.m_state.sat_orbits[0] = c[0];
		state.m_state.sat_orbits[1] = c[1];
		state.m_state.sat_orbits[2] = c[2];
		SharedData::Instance()->Settings(state);
	}
	Vec3f getPlanetOrbitColor(void) const {
		return Planet::getPlanetOrbitColor();
	}
	Vec3f getSatelliteOrbitColor(void) const {
		return Planet::getSatelliteOrbitColor();
	}

	// Compute the position for every elements of the solar system.
	// home_planet is needed for light travel time computation
	void computePositions(double date,const Planet *home_planet);

	// Compute the transformation matrix for every elements of the solar system.
	// home_planet is needed for light travel time computation
	void computeTransMatrices(double date,const Planet *home_planet);

	// Search if any Planet is close to position given in earth equatorial position.
	Object search(Vec3d, const Navigator * nav, const Projector * prj) const;

	// Return a stl vector containing the planets located inside the lim_fov circle around position v
	vector<Object> search_around(Vec3d v,
	                             double lim_fov,
	                             const Navigator * nav,
	                             const Projector * prj,
	                             bool *default_last_item,
	                             bool aboveHomePlanet ) const;

	//! Return the matching planet pointer if exists or NULL
	Planet* searchByEnglishName(string planetEnglishName) const;

	//! Return the matching planet pointer if exists or NULL
	//! @param planetNameI18n The case sensistive translated planet name
	Object searchByNamesI18(string planetNameI18n) const;

	Planet* getSun(void) const {
		return sun;
	}
	Planet* getEarth(void) const {
		return earth;
	}
	Planet* getMoon(void) const {
		return moon;
	}

	//! Activate/Deactivate planets display
	void setFlagPlanets(bool b) {
		Planet::setFlagShow(b);
	}
	bool getFlagPlanets(void) const {
		return Planet::getFlagShow();
	}

	//! Activate/Deactivate planets trails display
	void setFlagTrails(bool b);
	bool getFlagTrails(void) const;

	//! Activate/Deactivate planets hints display
	void setFlagHints(bool b);
	bool getFlagHints(void) const;

	//! Activate/Deactivate planet orbits display
	void setFlagOrbits(bool b);
	bool getFlagOrbits(void) const {
		return flagOrbits;
	}

	void setFlagLightTravelTime(bool b) {
		flag_light_travel_time = b;
		SettingsState state;
		state.m_state.light_travel_time = b;
		SharedData::Instance()->Settings( state );
	}
	bool getFlagLightTravelTime(void) const {
		return flag_light_travel_time;
	}

	//! Start/stop accumulating new trail data (clear old data)
	void startTrails(bool b);

	void setTrailColor(const Vec3f& c)  {
		Planet::set_trail_color(c);
		SettingsState state;
		state.m_state.planet_trails[0] = c[0];
		state.m_state.planet_trails[1] = c[1];
		state.m_state.planet_trails[2] = c[2];
		SharedData::Instance()->Settings(state);
	}
	Vec3f getTrailColor(void) const {
		return Planet::getTrailColor();
	}
	void updateTrails(const Navigator* nav);

	//! Set/Get base planets display scaling factor
	void setScale(float scale) {
		Planet::setScale(scale);
	}
	float getScale(void) const {
		return Planet::getScale();
	}

	//! Set/Get base planets display limit in pixels
	void setSizeLimit(float scale) {
		Planet::setSizeLimit(scale);
	}
	float getSizeLimit(void) const {
		return Planet::getSizeLimit();
	}


	//! Set/Get if Moon display is scaled
	void setFlagMoonScale(bool b) {
		if (!b) getMoon()->set_sphere_scale(1);
		else getMoon()->set_sphere_scale(moonScale);
		flagMoonScale = b;
		ReferenceState state;
		state.moon_scaled = b;
		SharedData::Instance()->References( state );
	}
	bool getFlagMoonScale(void) const {
		return flagMoonScale;
	}

	//! Set/Get Moon display scaling factor
	void setMoonScale(float f) {
		moonScale = f;
		if (flagMoonScale) getMoon()->set_sphere_scale(moonScale);
	}
	float getMoonScale(void) const {
		return moonScale;
	}

	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b) {
		Planet::setFlagClouds(b);
	}
	//! Get flag for displaying Atmosphere
	bool getFlagClouds(void) const {
		return Planet::getFlagClouds();
	}

	//! Get list of all the translated planets name
	vector<string> getNamesI18(void);

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	vector<string> listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const;

	//! Set selected planet by english name or "" to select none
	void setSelected(const string& englishName) {
		setSelected(searchByEnglishName(englishName));
	}

	//! Set selected object from its pointer
	void setSelected(const Object &obj);

	//! Get selected object's pointer
	Object getSelected(void) const {
		return selected;
	}

	void setFlagShaders(bool b) {
		Planet::flagShaders = b;
	}

private:

	// load one object from a hash (returns error message if any)
	string addBody(stringHash_t & param, bool deletable);

	Planet* sun;
	Planet* moon;
	Planet* earth;

	//! The currently selected planet
	Object selected;

	// solar system related settings
	float object_scale;  // should be kept synchronized with star scale...

	bool flagMoonScale;
	float moonScale;	// Moon scale value

	s_font* planet_name_font;
	vector<Planet*> system_planets;		// Vector containing all the bodies of the system
	bool near_lunar_eclipse(const Navigator * nav, Projector * prj);

	// draw earth shadow on moon for lunar eclipses
	void draw_earth_shadow(const Navigator * nav, Projector * prj);

	// And sort them from the furthest to the closest to the observer
	struct bigger_distance : public binary_function<Planet*, Planet*, bool> {
		bool operator()(Planet* p1, Planet* p2) {
			return p1->get_distance() > p2->get_distance();
		}
	};

	s_texture *tex_earth_shadow;  // for lunar eclipses

	// Master settings
	bool flagOrbits;
	bool flag_light_travel_time;
	bool flagHints;
	bool flagTrails;

	planetHash_t planetHash;  // holds list of current planet english names
	// (with parent pointer) to enforce uniqueness

};


#endif // _SOLARSYSTEM_H_
