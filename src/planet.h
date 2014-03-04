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

#ifndef _PLANET_H_
#define _PLANET_H_

#include "program_object.h"
#include "object_base.h"
#include "nightshade.h"
#include "utility.h"
#include "s_font.h"
#include "tone_reproductor.h"
#include "vecmath.h"
#include "callbacks.hpp"
#include "fader.h"
#include "translator.h"
#include "shared_data.h"
#include "orbit.h"

#include <list>
#include <string>


// epoch J2000: 12 UT on 1 Jan 2000
#define J2000 2451545.0

#define ORBIT_SEGMENTS 60

using namespace std;

struct TrailPoint {
	Vec3d point;
	double date;
};



// Class used to store orbital elements
class RotationElements
{
public:
	RotationElements(void)
	  : period(1.), offset(0.), epoch(J2000),
	    obliquity(0.), ascendingNode(0.), precessionRate(0.), 
		sidereal_period(0.), axialTilt(0.) {}
	float period;        // rotation period
	float offset;        // rotation at epoch
	double epoch;
	float obliquity;     // tilt of rotation axis w.r.t. ecliptic
	float ascendingNode; // long. of ascending node of equator on the ecliptic
	float precessionRate; // rate of precession of rotation axis in rads/day
	double sidereal_period; // sidereal period (Planet year in earth days)
	float axialTilt; // Only used for tropic lines on planets
};

// Class to manage rings for planets like saturn
class Ring
{
public:
	Ring(double radius_min,double radius_max,const string &texname, const string &path);
	~Ring(void);
	void draw(const Projector* prj,const Mat4d& mat,double screen_sz,Vec3f& lightDirection,Vec3f& planetPosition, float planetRadius);
	double get_size(void) const {
		return radius_max;
	}
	double get_inner_radius(void) const {
		return radius_min;
	}
	const s_texture *get_tex(void) const {
		return tex;
	}
private:
	const double radius_min;
	const double radius_max;
	const s_texture *tex;

	// Shaders
	ProgramObject programObject;
	bool shaderEnable;

};


class Planet : public ObjectBase
{
public:

	enum BODY_VISIBILITY_TYPE {
		NORMAL,
		HIDDEN,
		NONEXISTANT
	};

	double getSiderealDay(void) const {
		return re.period;
	}

	// TODO, shader texture flags are redundant
	Planet(Planet *parent,
	       const string& englishName,
	       int flagHalo,
	       int flag_lighting,
	       double radius,
	       double oblateness,
	       Vec3f color,
	       float albedo,
	       const string& tex_map_path,
	       const string& tex_map_name,
	       const string& tex_halo_name,
	       Orbit *orbit,
	       bool close_orbit,
	       bool hidden,
	       bool _deleteable,
	       bool flat_texture,
	       double orbit_bounding_radius,
		   const string& tex_norm_name,
		   const string& tex_night_name,
		   const string& tex_specular_name,
		   const string& tex_cloud_name,
		   const string& tex_norm_cloud_name,	
		   float comet_absolute_magnitude,
		   float comet_magnitude_slope
		   );

	~Planet();

	double getRadius(void) const {
		return radius;
	}

	double getOblateness(void) const {
		return 1-one_minus_oblateness;
	}

	// Return the information string "ready to print" :)
	string getSkyLabel(const Navigator * nav) const;
	string getInfoString(const Navigator * nav) const;
	string getShortInfoString(const Navigator * nav) const;
	double get_close_fov(const Navigator * nav) const;
	double get_satellites_fov(const Navigator * nav) const;
	double get_parent_satellites_fov(const Navigator * nav) const;
	float get_mag(const Navigator * nav) const {
		return compute_magnitude(nav);
	}
	const s_texture *getMapTexture(void) const {
		return tex_map;
	}

	Orbit *getOrbit() { return orbit; }

	/** Translate planet name using the passed translator */
	void translateName(Translator& trans);

	// Compute the z rotation to use from equatorial to geographic coordinates
	double getSiderealTime(double jd) const;
	Mat4d getRotEquatorialToVsop87(void) const;
	void setRotEquatorialToVsop87(const Mat4d &m);

	const RotationElements &getRotationElements(void) const {return re;}

	// Compute the position in the parent Planet coordinate system
	void computePositionWithoutOrbits(double date);
	void compute_position(double date);

	// Compute the transformation matrix from the local Planet coordinate to the parent Planet coordinate
	void compute_trans_matrix(double date);

	// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	double get_phase(Vec3d obs_pos) const;

	// Get the magnitude for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	float compute_magnitude(const Vec3d obs_pos) const;
	float compute_magnitude(const Navigator * nav) const;

	// Draw the Planet, if hint_ON is != 0 draw a circle and the name as well
	// Return the squared distance in pixels between the current and the
	// previous position this planet was drawn at.
	double draw(Projector* prj, const Navigator* nav, const ToneReproductor* eye,
	            int flag_point, bool stencil, bool depthTest, bool drawHomePlanet,
				bool selected);

	// Set the orbital elements
	void set_rotation_elements(float _period, float _offset, double _epoch,
	                           float _obliquity, float _ascendingNode, float _precessionRate, double _sidereal_period, float _axial_tilt);
	double getRotAscendingnode(void) const {
		return re.ascendingNode;
	}
	double getRotObliquity(void) const {
		return re.obliquity;
	}
	double getAxialTilt(void) const {
		return re.axialTilt;
	}


	// Get the Planet position in the parent Planet ecliptic coordinate
	Vec3d get_ecliptic_pos() const;

	// Return the heliocentric ecliptical position
	Vec3d get_heliocentric_ecliptic_pos() const;
	void set_heliocentric_ecliptic_pos(const Vec3d &pos);

	// Compute the distance to the given position in heliocentric coordinate (in AU)
	double compute_distance(const Vec3d& obs_helio_pos);
	double get_distance(void) const {
		return distance;
	}

	// Get a matrix which converts from heliocentric ecliptic coordinate to local geographic coordinate
//	Mat4d get_helio_to_geo_matrix();

	ObjectRecord::OBJECT_TYPE get_type(void) const {
		return ObjectRecord::OBJECT_PLANET;
	}

	// Return the Planet position in rectangular earth equatorial coordinate
	Vec3d get_earth_equ_pos(const Navigator *nav) const;
	// observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const;

	string getEnglishName(void) const {
		return englishName;
	}
	string getNameI18n(void) const {
		return nameI18;
	}

	void set_rings(Ring* r);

	void set_ring_shadow(bool shadow);

	void set_sphere_scale(float s) {
		sphere_scale = s;
	}
	float get_sphere_scale(void) const {
		return sphere_scale;
	}

	const Planet *get_parent(void) const {
		return parent;
	}

	// modifiable
	Planet *getParent(void) {
		return parent;
	}

	void set_big_halo(const string& halotexfile, const string& path);
	void set_halo_size(float s) {
		big_halo_size = s;
	}

	static void set_font(s_font* f) {
		planet_name_font = f;
	}

	static void setScale(float s) {
		object_scale = s;
	}
	static float getScale(void) {
		return object_scale;
	}

	static void setSizeLimit(float s) {
		object_size_limit = s;
	}
	static float getSizeLimit(void) {
		return object_size_limit;
	}


	static void set_label_color(const Vec3f& lc) {
		label_color = lc;
	}
	static const Vec3f& getLabelColor(void) {
		return label_color;
	}

	static void set_planet_orbit_color(const Vec3f& oc) {
		planet_orbit_color = oc;
	}
	static const Vec3f& getPlanetOrbitColor() {
		return planet_orbit_color;
	}

	// [3] means use the color if true
	void setLocalOrbitColor(const Vec4f& oc) {
		local_orbit_color = oc;
	}
	const Vec4f& getLocalOrbitColor() {
		return local_orbit_color;
	}

	static void set_satellite_orbit_color(const Vec3f& oc) {
		satellite_orbit_color = oc;
	}
	static const Vec3f& getSatelliteOrbitColor() {
		return satellite_orbit_color;
	}

	// draw orbital path of body
	void draw_orbit_2d(const Navigator * nav, const Projector* prj, const Mat4d &mat);
	void draw_orbit_3d(const Navigator * nav, const Projector* prj, const Mat4d &mat);

	void update_trail(const Navigator* nav);
	void draw_trail(const Navigator * nav, const Projector* prj);
	static void set_trail_color(const Vec3f& c) {
		trail_color = c;
	}
	static const Vec3f& getTrailColor() {
		return trail_color;
	}

	//! Start/stop accumulating new trail data (clear old data)
	void startTrail(bool b);

	void update(int delta_time, double JD);

	void setFlagHints(bool b) {
		hint_fader = b;
	}
	bool getFlagHints(void) const {
		return hint_fader;
	}

	void setFlagOrbit(bool b) {
		orbit_fader = b;
	}
	bool getFlagOrbit(void) const {
		return orbit_fader;
	}

	void setFlagTrail(bool b) {
		if (b == trail_fader) return;
		trail_fader = b;
		startTrail(b);
	}
	bool getFlagTrail(void) const {
		return trail_fader;
	}

	static void setFlagShow(bool b) {
		Planet::flagShow = b;
		SettingsState state;
		state.m_state.planets = b;
		SharedData::Instance()->Settings(state);
	}

	static bool getFlagShow(void) {
		return Planet::flagShow;
	}

	static void setFlagClouds(bool b) {
		ReferenceState state;
		state.clouds = b;
		SharedData::Instance()->References( state );
		Planet::flagClouds = b;
	}

	static bool getFlagClouds(void) {
		return Planet::flagClouds;
	}


	void setVisibility(bool b) {
		visibilityFader = b;
	}

	bool getVisibility(void) {
		return visibilityFader;
	}


	bool isDeleteable() const {
		return deleteable;    // If allowed to delete from script
	}
	bool isSatellite() const {
		return is_satellite;
	}

// remove from parent satellite list
	void removeSatellite(Planet *planet);

// for depth buffering of orbits
	void updateBoundingRadii();
	double calculateBoundingRadius();
	double getBoundingRadius() const;

	// Return the radius of a circle containing the object on screen
	float get_on_screen_size(const Projector* prj, const Navigator * nav, bool orb_only = false);

	// Return the radius of a circle containing the object and satellites on screen
	float get_on_screen_bounding_size(const Projector* prj, const Navigator * nav);


	// Return the angle of the planet orb
	float get_angular_size(const Projector* prj, const Navigator * nav);



	// See if planet might be visible
	//	bool testVisibility(const Projector* prj, const Navigator * nav);

	// See if simple planet not needing depth buffer
	bool isSimplePlanet();
	bool hasSatellite() {
		return !satellites.empty();
	}

	double getLastJD() const { return lastJD; }

	// TODO: this should be protected or moved to solarsystem.cpp
	static s_texture *tex_eclipse_map;  // for moon shadow lookups

	static bool flagShaders;  // whether to use shaders when rendering

	void setValidityPeriod(double _startJD, double _endJD);  // Set valid range to observe body

protected:
	// Draw the 3D sphere
	void draw_sphere(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

// draw planet axis
	void draw_axis(const Projector* prj, const Mat4d& mat, float screen_sz);

// draw the map as a flat image
	void draw_image(const Projector* prj, const Mat4d& mat, float screen_sz);

	// Draw the small star-like 2D halo
	void draw_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);
	void draw_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, const Mat4d& mat);

	// Draw the small star-like point
	void draw_point_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);
	void draw_point_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, const Mat4d& mat);

	// Draw the circle and name of the Planet
	void draw_hints(const Navigator* nav, const Projector* prj);

	// Draw the big halo (for sun or moon)
	void draw_big_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	bool useParentPrecession(double jd) { return getOrbit()->useParentPrecession(jd); }

	// Interpolate a curve for orbits
	static Vec3d calculateSplinePoint(double, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3);

	static void drawSpline2d(const Projector *prj, const Mat4d &mat, int segments, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3);
	static void drawSpline3d(const Projector *prj, const Mat4d &mat, int segments, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3);

	string englishName; // english planet name
	string nameI18;				// International translated name
	int flagHalo;					// Set wether a little "star like" halo will be drawn
	int flag_lighting;				// Set wether light computation has to be proceed
	RotationElements re;			// Rotation param
	double radius;					// Planet radius in UA
	double one_minus_oblateness;    // (polar radius)/(equatorial radius)
	Vec3d orbitPoint[ORBIT_SEGMENTS];    // store heliocentric coordinates for drawing the orbit
	Vec3d ecliptic_pos; 			// Position in UA in the rectangular ecliptic coordinate system
	Vec3d light_ecliptic_pos; 			// position before light travel time correction (when correction enabled)
	// centered on the parent Planet
	Vec3d last_pos;                 // Used when interpolating
	Vec3d next_pos;                 // Used when interpolating
	bool interpolating;             // Whether to do a linear interpolation when delta_time < deltaJD
	int interpolationDirection;     // is time going forward (+) or backward (-)?
	Vec3d screenPos;				// Used to store temporarily the 2D position on screen
	Vec3d previousScreenPos;			// The position of this planet in the previous frame.
	Vec3f color;
	float albedo;					// Planet albedo
	Mat4d rot_local_to_parent;
	Mat4d rot_local_to_parent_unprecessed;  // currently used for moons (Moon elliptical orbit required this)
	Mat4d mat_local_to_parent;		// Transfo matrix from local ecliptique to parent ecliptic
	float axis_rotation;			// Rotation angle of the Planet on it's axis
	s_texture * tex_map;			// Planet map texture
	s_texture * tex_halo;			// Little halo texture
	s_texture * tex_big_halo;		// Big halo texture
	s_texture * tex_norm;
	s_texture * tex_night;
	s_texture * tex_specular;
	s_texture * tex_cloud;
	s_texture * tex_shadow_cloud;
	s_texture * tex_norm_cloud;

	Vec3f eye_sun;
	Vec3f eye_planet;
	ProgramObject programObject;    // Shaders
	ProgramObject cloudProgramObject;
	bool bumpEnable;
	bool nightEnable;
	bool ringedEnable;

	float big_halo_size;				// Halo size on screen

	Ring* rings;					// Planet rings

	double distance;				// Temporary variable used to store the distance to a given point
	// it is used for sorting while drawing

	float sphere_scale;				// Artificial scaling for better viewing

	double lastJD;  // Last Julian day ecliptic_position was calculated
	double lastLightJD;  // Last Julian day light_ecliptic_position was calculated
	double last_orbitJD;
	double deltaJD;
	double delta_orbitJD;
	bool orbit_cached;       // whether orbit calculations are cached for drawing orbit yet

	Orbit *orbit;            // orbit object for this body

// made non-const so can actually use this for things!
	Planet *parent;				// Planet parent i.e. sun for earth

	list<Planet *> satellites;		// satellites of the Planet

	static s_font* planet_name_font;// Font for names
	static float object_scale;
	static float object_size_limit;  // in pixels
	static Vec3f label_color;
	static Vec3f planet_orbit_color;
	static Vec3f satellite_orbit_color;
	static Vec3f trail_color;

// to override global colors
	// use if [3] is >0
	Vec4f local_orbit_color;

	list<TrailPoint>trail;
	bool trail_on;  // accumulate trail data if true
	double DeltaTrail;
	int MaxTrail;
	double last_trailJD;
	bool first_point;  // if need to take first point of trail still

	static LinearFader flagShow;
	static LinearFader flagClouds;

	LinearFader hint_fader;
	LinearFader orbit_fader;
	LinearFader trail_fader;
	LinearFader visibilityFader;  // allows related lines and labels to fade in/out

	bool close_orbit; // whether to close orbit loop
	BODY_VISIBILITY_TYPE visibility;  // useful for fake planets or caput comets: HIDDEN are not drawn or labeled, NONEXISTANT are not position calculated either
	bool deleteable;  // whether allowed to delete from scripts
	bool flat_texture; // whether to draw tex_map flat or on a sphere
	bool is_satellite;  // whether has a planet as a parent

	double orbit_bounding_radius; // AU calculated at load time for elliptical orbits at least DIGITALIS

	double boundingRadius;  // Cached AU value for use with depth buffer

	static s_texture *defaultTexMap;  // Default texture map for bodies if none supplied

	bool ring_shadow; 

	double sun_half_angle; // for moon shadow calcs

	float comet_abs_mag;  // if < 99 use comet mag calculation
	float comet_mag_slope;

	bool hasValidityPeriod; // if body is only valid for a given time period
	double startJD;
	double endJD;
};


// Class for flying to new planets
class ArtificialPlanet : public Planet 
{
public:
	ArtificialPlanet(Planet &orig);
	void setDest(Planet &dest);
	void computeAverage(float t);
private:
	void setRot(const Vec3d &r);
	static Vec3d getRot(Planet *p);
	Planet *dest;
	const string orig_name;
	const string orig_name_i18n;
	const double orig_radius;
	Vec3d orig_rotation;
	Vec3d startPos;
};


#endif // _PLANET_H_
