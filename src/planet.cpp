/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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

#include <iostream>
#include <iomanip>

#include "night_shader.h"
#include "bump_shader.h"
#include "ring_shader.h"
#include "ringed_shader.h"
#include "planet.h"
#include "navigator.h"
#include "projector.h"
#include "s_font.h"
#include "s_gui.h"
#include "stellastro.h" // just for get_apparent_sidereal_time
#include <fastdb/fastdb.h>
#include <nshade_state.h>

s_font* Planet::planet_name_font = NULL;
float Planet::object_scale = 1.f;
float Planet::object_size_limit = 9;
Vec3f Planet::label_color = Vec3f(.4,.4,.8);
Vec3f Planet::planet_orbit_color = Vec3f(1,.6,1);
Vec3f Planet::satellite_orbit_color = Vec3f(.6,0,.6);
Vec3f Planet::trail_color = Vec3f(1,.7,.7);
LinearFader Planet::flagShow;
LinearFader Planet::flagClouds;
s_texture *Planet::defaultTexMap = NULL;
s_texture *Planet::tex_eclipse_map = NULL;
bool Planet::flagShaders = true;

Planet::Planet(Planet *parent,
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
			   ) :

		englishName(englishName), flagHalo(flagHalo),
		flag_lighting(flag_lighting),
		radius(radius), one_minus_oblateness(1.0-oblateness),
		interpolating(false), interpolationDirection(0),
		color(color), albedo(albedo), axis_rotation(0.),
		tex_map(NULL), tex_halo(NULL), tex_big_halo(NULL), 
		tex_norm(NULL), tex_night(NULL), tex_specular(NULL), tex_cloud(NULL),
		tex_shadow_cloud(NULL), tex_norm_cloud(NULL), eye_sun(0.0f, 0.0f, 0.0f),
		bumpEnable(tex_norm_name != ""), nightEnable(tex_night_name != ""), 
		ringedEnable(0), rings(NULL), sphere_scale(1.f),
		lastJD(J2000), lastLightJD(J2000), last_orbitJD(0), deltaJD(JD_SECOND), orbit_cached(0),
		orbit(orbit), 
		parent(parent), close_orbit(close_orbit), deleteable(_deleteable),
		flat_texture(flat_texture), is_satellite(0), orbit_bounding_radius(orbit_bounding_radius),
		boundingRadius(-1), ring_shadow(false), sun_half_angle(0.0), comet_abs_mag(comet_absolute_magnitude),
		comet_mag_slope(comet_magnitude_slope), hasValidityPeriod(false)
{

	if(hidden) visibility = HIDDEN;
	else visibility = NORMAL;

	if (parent) {
		parent->satellites.push_back(this);
		if (parent->getEnglishName() != "Sun") is_satellite = 1; // quicker lookup
	}
	ecliptic_pos = light_ecliptic_pos = Vec3d(0.,0.,0.);
	rot_local_to_parent = Mat4d::identity();
	rot_local_to_parent_unprecessed = Mat4d::identity();
	mat_local_to_parent = Mat4d::identity();

	if(tex_map_name == "") {
		// NOTE, default body texture initialization would ideally be done elsewhere
		if(defaultTexMap == NULL) 
			defaultTexMap = new s_texture("bodies/nomap.png", TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
		tex_map = defaultTexMap;
	} else {
		// planets are mipmapped	
		if ( tex_map_path == "" ) {
			tex_map = new s_texture(tex_map_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
			if (flagHalo) tex_halo = new s_texture(tex_halo_name);
		} else {
			// TODO: CAN NOT TELL if textures are loaded or not!!!
			tex_map = new s_texture(1, tex_map_path + tex_map_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
			if (flagHalo) tex_halo = new s_texture(1, tex_map_path + tex_halo_name, TEX_LOAD_TYPE_PNG_BLEND1);
		}
	}

	if(flagHalo) {
		if ( tex_map_path == "" ) tex_halo = new s_texture(tex_halo_name);
		else tex_halo = new s_texture(1, tex_map_path + tex_halo_name, TEX_LOAD_TYPE_PNG_BLEND1);
	}

	if(flagShaders) {
		if (nightEnable) {
			if ( tex_map_path == "" ) tex_night = new s_texture(tex_night_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
			else tex_night = new s_texture(1, tex_map_path + tex_night_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
			if(tex_map_path=="") tex_specular = new s_texture(tex_specular_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
			else tex_specular = new s_texture(1, tex_map_path + tex_specular_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
			if(!NightShader::instance()->isSupported()) {
				cout << "Unable to use night shader for " << englishName << endl;
				nightEnable = false;
			} else programObject.attachShader(NightShader::instance());
			bumpEnable = false;
		} else if (bumpEnable) {
			// Currently night and bump shader are exclusive
			if ( tex_map_path == "" )
				tex_norm = new s_texture(tex_norm_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
			else tex_norm = new s_texture(1, tex_map_path + tex_norm_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
			if(!tex_norm->getID() || !BumpShader::instance()->isSupported()) {
				cout << "Unable to use bump shader for " << englishName << endl;
				bumpEnable = false;
			} else programObject.attachShader(BumpShader::instance());
		} else if(englishName != "Sun") {
			// Default planet shader
			ringedEnable = true;
			if(!RingedShader::instance()->isSupported()) {
				cout << "Unable to use ringed shader for " << englishName << endl;
				ringedEnable = false;
			} else programObject.attachShader(RingedShader::instance());
			nightEnable = bumpEnable = false;
		} else {
			ringedEnable = nightEnable = bumpEnable = false;
		}
	}

	if(tex_cloud_name != "") {
		// Try to use cloud texture in any event, even if can not use shader
		if(tex_map_path == "") tex_cloud = new s_texture(tex_cloud_name, TEX_LOAD_TYPE_PNG_ALPHA, true);
		else tex_cloud = new s_texture(1, tex_map_path + tex_cloud_name, TEX_LOAD_TYPE_PNG_ALPHA, true);

		if(tex_norm_cloud_name=="") {
			cout << "No cloud normal texture defined for " << englishName << endl;
		} else {
			if( tex_map_path == "") tex_norm_cloud = new s_texture(tex_norm_cloud_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true);
			else tex_norm_cloud = new s_texture(tex_norm_cloud_name, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true);
		}
	}

	// Body trails
	DeltaTrail = 1;
	// small increment like 0.125 would allow observation of latitude related wobble of moon
	// if decide to show moon trail

#ifdef LSS
// 4 years trails
	MaxTrail = 1460;
#else
// 60 day trails
	MaxTrail = 60;
#endif

	last_trailJD = 0; // for now
	trail_on = 0;
	first_point = 1;

	nameI18 = englishName;

	local_orbit_color[3] = 0; // do not use local color override

	visibilityFader = true;

	if( SharedData::Instance()->DB() && visibility == NORMAL ) {
		ObjectRecord rec( englishName.c_str(), nameI18.c_str(), ObjectRecord::OBJECT_PLANET );
		insert(rec);
		SharedData::Instance()->DB()->commit();
	}
}

Planet::~Planet()
{
	if (tex_map && tex_map != defaultTexMap) delete tex_map;
	tex_map = NULL;
	if (tex_halo) delete tex_halo;
	tex_halo = NULL;
	if (rings) delete rings;
	rings = NULL;
	if (tex_big_halo) delete tex_big_halo;
	tex_big_halo = NULL;
	if (tex_norm) delete tex_norm;
	tex_norm = NULL;
	if (tex_night) delete tex_night;
	tex_night = NULL;
	if (tex_specular) delete tex_specular;
	tex_specular = NULL;
	if (tex_cloud) delete tex_cloud;
	tex_cloud = NULL;
	if (tex_norm_cloud) delete tex_norm_cloud;
	tex_norm_cloud = NULL;
	if(orbit) delete orbit;
	orbit = NULL;

	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_PLANET;
		if( cursor.select(q) )
			cursor.remove();
		SharedData::Instance()->DB()->commit();
	}
}

void Planet::set_rings(Ring* r) {
	rings = r;
}

void Planet::set_ring_shadow(bool shadow) {
	ring_shadow = shadow;
}

// Return the information string "ready to print" :)
string Planet::getInfoString(const Navigator * nav) const
{
	double tempDE, tempRA;
	ostringstream oss;

	oss << _(englishName);  // UI translation can differ from sky translation
	oss.setf(ios::fixed);
	oss.precision(1);
#ifndef LSS
	if (sphere_scale != 1.f) oss << " (x" << sphere_scale << ")";
#endif
	oss << endl;

	oss.precision(2);
	oss << _("Magnitude: ") << compute_magnitude(nav->get_observer_helio_pos()) << endl;

	Vec3d equPos = get_earth_equ_pos(nav);
	rect_to_sphe(&tempRA,&tempDE,equPos);

	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << " / " << Utility::printAngleDMS(tempDE) << endl;

	// calculate alt az position
	Vec3d localPos = nav->earth_equ_to_local(equPos);
	rect_to_sphe(&tempRA,&tempDE,localPos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;

	oss << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << " / " << Utility::printAngleDMS(tempRA) << endl;

	oss.precision(8);
	oss << _("Distance: ") << equPos.length() << " " << _("AU") << endl;

	return oss.str();
}

//! Get sky label (sky translation)
string Planet::getSkyLabel(const Navigator * nav) const
{
	string ws = nameI18;
	ostringstream oss;
	oss.setf(ios::fixed);
	oss.precision(1);
#ifndef LSS // the more info written the worse it is
	if (sphere_scale != 1.f) {
		oss << " (x" << sphere_scale << ")";
		ws += oss.str();
	}
#endif
	return ws;
}


// Return the information string "ready to print" :)
string Planet::getShortInfoString(const Navigator * nav) const
{
	ostringstream oss;

	oss << _(englishName);  // UI translation can differ from sky translation

	oss.setf(ios::fixed);
	oss.precision(1);
#ifndef LSS
	if (sphere_scale != 1.f) oss << " (x" << sphere_scale << ")";
#endif
	oss.precision(2);
	oss << "  " << _("Magnitude: ") << compute_magnitude(nav->get_observer_helio_pos());

	Vec3d equPos = get_earth_equ_pos(nav);
	oss.precision(5);
	oss << "  " <<  _("Distance: ") << equPos.length() << " " << _("AU");
#ifdef NAV
	double tempDE, tempRA;
	Vec3d localPos = nav->earth_equ_to_local(equPos);
	rect_to_sphe(&tempRA,&tempDE,localPos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;
	oss << "  " << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << "/" << Utility::printAngleDMS(tempRA);
#endif
	return oss.str();
}

double Planet::get_close_fov(const Navigator* nav) const
{
	return atanf(radius*sphere_scale*2.f/get_earth_equ_pos(nav).length())*180./M_PI * 4;
}

double Planet::get_satellites_fov(const Navigator * nav) const
{

	if ( !satellites.empty() && englishName != "Sun") {
		double rad = getBoundingRadius();
		if ( rad > 0 ) return atanf(rad/get_earth_equ_pos(nav).length()) *180./M_PI * 4;
	}

	return -1.;

}

double Planet::get_parent_satellites_fov(const Navigator *nav) const
{
	if (parent && parent->parent) return parent->get_satellites_fov(nav);
	return -1.0;
}

// Set the orbital elements
void Planet::set_rotation_elements(float _period, float _offset, double _epoch, float _obliquity, 
								   float _ascendingNode, float _precessionRate, double _sidereal_period, float _axial_tilt)
{
	re.period = _period;
	re.offset = _offset;
	re.epoch = _epoch;
	re.obliquity = _obliquity;
	re.ascendingNode = _ascendingNode;
	re.precessionRate = _precessionRate;
	re.sidereal_period = _sidereal_period;  // used for drawing orbit lines
	re.axialTilt = _axial_tilt; // Used for drawing tropic lines

	delta_orbitJD = re.sidereal_period/ORBIT_SEGMENTS;
}


// Return the Planet position in rectangular earth equatorial coordinate
Vec3d Planet::get_earth_equ_pos(const Navigator * nav) const
{
	Vec3d v = get_heliocentric_ecliptic_pos();
	return nav->helio_to_earth_pos_equ(v);		// this is earth equatorial but centered
	// on observer's position (latitude, longitude)
	//return navigation.helio_to_earth_equ(&v); this is the real equatorial centered on earth center
}

Vec3d Planet::getObsJ2000Pos(const Navigator *nav) const
{
	return mat_vsop87_to_j2000.multiplyWithoutTranslation(
	           get_heliocentric_ecliptic_pos()
	           - nav->get_observer_helio_pos());
}


// Compute the position in the parent Planet coordinate system
// Actually call the provided function to compute the ecliptical position
// NB: THIS IS ONLY USED FOR POSITION TO DETERMINE LIGHT TRAVEL TIME CORRECTION
void Planet::computePositionWithoutOrbits(const double date)
{
	if(visibility == NONEXISTANT) return;

	if (fabs(lastLightJD-date)>deltaJD) {
		if(orbit) orbit->positionAtTimevInVSOP87Coordinates(date, light_ecliptic_pos);
		lastLightJD = date;
	}
}

// Calculate the actual body position at given Julian day
// Also update orbit visualization points as needed
// Uses interpolation in between calculated points for better performance 
void Planet::compute_position(const double date)
{

	if(!orbit || visibility == NONEXISTANT) return;

	// Large performance advantage from avoiding object overhead
	OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction();

	// for performance only update orbit points if visible
	if (orbit_fader.getInterstate()*visibilityFader.getInterstate()>0.000001 
		&& delta_orbitJD > 0 
		&& (fabs(last_orbitJD-date)>delta_orbitJD || !orbit_cached)) {

		// calculate orbit first (for line drawing)
		double date_increment = re.sidereal_period/ORBIT_SEGMENTS;
		double calc_date;
		//	  int delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		int delta_points;

		if ( date > last_orbitJD ) {
			delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		} else {
			delta_points = (int)(-0.5 + (date - last_orbitJD)/date_increment);
		}
		double new_date = last_orbitJD + delta_points*date_increment;

		//	  printf( "Updating orbit coordinates for %s (delta %f) (%d points)\n", name.c_str(), delta_orbitJD, delta_points);
		//cout << englishName << ": " << delta_points << "  " << orbit_cached << endl;

		if ( delta_points > 0 && delta_points < ORBIT_SEGMENTS && orbit_cached) {

			for ( int d=0; d<ORBIT_SEGMENTS; d++ ) {
				if (d + delta_points >= ORBIT_SEGMENTS ) {
					// calculate new points
					calc_date = new_date + (d-ORBIT_SEGMENTS/2)*date_increment;
					// date increments between points will not be completely constant though

					if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
					else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
				} else {
					orbitPoint[d] = orbitPoint[d+delta_points];
				}
			}

			last_orbitJD = new_date;

		} else if ( delta_points < 0 && abs(delta_points) < ORBIT_SEGMENTS  && orbit_cached) {

			for ( int d=ORBIT_SEGMENTS-1; d>=0; d-- ) {
				if (d + delta_points < 0 ) {
					// calculate new points
					calc_date = new_date + (d-ORBIT_SEGMENTS/2)*date_increment;

					if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
					else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
				} else {
					orbitPoint[d] = orbitPoint[d+delta_points];
				}
			}

			last_orbitJD = new_date;

		} else if ( delta_points || !orbit_cached) {

			// update all points (less efficient)
			for ( int d=0; d<ORBIT_SEGMENTS; d++ ) {
				calc_date = date + (d-ORBIT_SEGMENTS/2)*date_increment;

				if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
				else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
			}

			last_orbitJD = date;

			// \todo remove this for efficiency?  Can cause rendering issues near body though
			// If orbit is largely constant through time cache it
			if (orbit->isStable(date)) orbit_cached = 1;
		}
	}


	// Done updating orbit visualization now calculate actual body position

	double delta = date-lastJD;
	int sign = 1;
	if(delta < 0 ) sign = -1;
	delta = fabs(delta);
	
	// Position interpolation for efficiency
	if(delta < deltaJD &&
	   !(interpolating && interpolationDirection != sign)) {
		if(!interpolating) {
			interpolating = true;
			interpolationDirection = sign;
			double nextDate = lastJD + sign * deltaJD;
			
			if(oscFunc) (*oscFunc)(nextDate,nextDate,next_pos);
			else orbit->positionAtTimevInVSOP87Coordinates(nextDate, nextDate, next_pos);
		}
		
		ecliptic_pos = (1-delta/deltaJD)*last_pos + (delta/deltaJD)*next_pos;
		
	} else {
		interpolating = false;
		
		// calculate actual Planet position
		if(oscFunc) (*oscFunc)(date,date,ecliptic_pos);
		else orbit->positionAtTimevInVSOP87Coordinates(date,date,ecliptic_pos);
		last_pos = ecliptic_pos;
		lastJD = date;
	}

}

// Compute the transformation matrix from the local Planet coordinate to the parent Planet coordinate
void Planet::compute_trans_matrix(double jd)
{
	axis_rotation = getSiderealTime(jd);

	// Special case - heliocentric coordinates are on ecliptic,
	// not solar equator...
	if (parent) {
		rot_local_to_parent = Mat4d::zrotation(re.ascendingNode
		                                       -re.precessionRate*(jd-re.epoch))
		                      * Mat4d::xrotation(re.obliquity);
		rot_local_to_parent_unprecessed = Mat4d::zrotation(re.ascendingNode)
		                      * Mat4d::xrotation(re.obliquity);

	}

	mat_local_to_parent = Mat4d::translation(ecliptic_pos)
	                      * rot_local_to_parent;

}

Mat4d Planet::getRotEquatorialToVsop87(void) const
{
	Mat4d rval = rot_local_to_parent;
	if (parent) for (const Planet *p=parent; p->parent; p=p->parent) {
			rval = p->rot_local_to_parent * rval;
		}
	return rval;
}

void Planet::setRotEquatorialToVsop87(const Mat4d &m) {
  Mat4d a = Mat4d::identity();
  if (parent) for (const Planet *p=parent;p->parent;p=p->parent) {
    a = p->rot_local_to_parent * a;
  }
  rot_local_to_parent = a.transpose() * m;
}


// Get a matrix which converts from heliocentric ecliptic coordinate to local geographic coordinate
//Mat4d Planet::get_helio_to_geo_matrix()
//{
//	Mat4d mat = mat_local_to_parent;
//	mat = mat * Mat4d::zrotation(axis_rotation*M_PI/180.);
//
//	// Iterate thru parents
//	Planet * p = parent;
//	while (p!=NULL && p->parent!=NULL)
//		{
//			mat = p->mat_local_to_parent * mat;
//			p=p->parent;
//		}
//	return mat;
//}

// Compute the z rotation to use from equatorial to geographic coordinates
double Planet::getSiderealTime(double jd) const
{
	if (englishName=="Earth") return get_apparent_sidereal_time(jd);

	double t = jd - re.epoch;
	double rotations = t / (double) re.period;
	double wholeRotations = floor(rotations);
	double remainder = rotations - wholeRotations;

	return remainder * 360. + re.offset;
}

// Get the Planet position in the parent Planet ecliptic coordinate
Vec3d Planet::get_ecliptic_pos() const
{
	return ecliptic_pos;
}

// Return the heliocentric ecliptical position
// used only for earth shadow, lunar eclipse -- DIGITALIS: this statement is not true!
Vec3d Planet::get_heliocentric_ecliptic_pos() const
{
	Vec3d pos = ecliptic_pos;
	const Planet *p = parent;

	while (p && p->parent) {
		pos += p->ecliptic_pos;
		p = p->parent;
	}

	return pos;
}


void Planet::set_heliocentric_ecliptic_pos(const Vec3d &pos)
{
	ecliptic_pos = pos;
	const Planet *p = parent;
	if (p) while (p->parent)
	{
		ecliptic_pos -= p->ecliptic_pos;
		p = p->parent;
	}
}


// Compute the distance to the given position in heliocentric coordinate (in AU)
double Planet::compute_distance(const Vec3d& obs_helio_pos)
{
	distance = (obs_helio_pos-get_heliocentric_ecliptic_pos()).length();

	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_PLANET;
		if( cursor.select(q) ) {
			cursor->distance = distance;
			cursor.update();
		}
		SharedData::Instance()->DB()->commit();
	}

	return distance;
}

// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (dist in AU)
double Planet::get_phase(Vec3d obs_pos) const
{
	const double sq = obs_pos.lengthSquared();
	const Vec3d heliopos = get_heliocentric_ecliptic_pos();
	const double Rq = heliopos.lengthSquared();
	const double pq = (obs_pos - heliopos).lengthSquared();
	const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
	return (1.0 - acos(cos_chi)/M_PI) * cos_chi
	       + sqrt(1.0 - cos_chi*cos_chi) / M_PI;
}

float Planet::compute_magnitude(Vec3d obs_pos) const
{
	float rval = 0;
	const double sq = obs_pos.lengthSquared();
	if (parent == 0) {
		// sun
		rval = -26.73f + 2.5f*log10f(sq);
	} else {

		const Vec3d heliopos = get_heliocentric_ecliptic_pos();

		if (comet_abs_mag < 99) {

			rval = comet_abs_mag + 5*log10f((obs_pos - heliopos).length()) 
				+ 2.5 * comet_mag_slope * log10f(heliopos.length());

		} else { // spherical body

			const double Rq = heliopos.lengthSquared();
			const double pq = (obs_pos - heliopos).lengthSquared();
			const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
			const double phase = (1.0 - acos(cos_chi)/M_PI) * cos_chi
				+ sqrt(1.0 - cos_chi*cos_chi) / M_PI;
			const float F = 2.0 * albedo * radius * radius * phase / (3.0*pq*Rq);
			rval = -26.73f - 2.5f * log10f(F);
		}
	}

	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_PLANET;
		if( cursor.select(q) ) {
			cursor->mag = rval;
			cursor.update();
		}
		SharedData::Instance()->DB()->commit();
	}

	return rval;
}

float Planet::compute_magnitude(const Navigator * nav) const
{
	return compute_magnitude(nav->get_observer_helio_pos());
}

void Planet::set_big_halo(const string& halotexfile, const string &path)
{
	tex_big_halo = new s_texture((path!=""), path + halotexfile, TEX_LOAD_TYPE_PNG_SOLID);
}

// Return the radius of a circle containing the object on screen
float Planet::get_on_screen_size(const Projector* prj, const Navigator * nav, bool orb_only)
{
	double rad;
	if (rings && !orb_only) rad = rings->get_size();
	else rad = radius;

	return atanf(rad*sphere_scale*2.f/get_earth_equ_pos(nav).length())*180./M_PI/prj->get_fov()*prj->getViewportHeight();
}

// Return the angle (degrees) of the planet orb
float Planet::get_angular_size(const Projector* prj, const Navigator * nav)
{

	return atanf(radius*sphere_scale*2.f/get_earth_equ_pos(nav).length())*180./M_PI;
}


// Return the radius of a circle containing the object and its satellites on screen
float Planet::get_on_screen_bounding_size(const Projector* prj, const Navigator * nav)
{
	double rad = getBoundingRadius();

	return atanf(rad*sphere_scale*2.f/get_earth_equ_pos(nav).length())*180./M_PI/prj->get_fov()*prj->getViewportHeight();
}


// Draw the Planet and all the related infos : name, circle etc..
double Planet::draw(Projector* prj, const Navigator * nav, const ToneReproductor* eye, int flag_point, bool stencil, bool depthTest, bool drawHomePlanet,bool selected)
{
	if (visibility != NORMAL) return 0;

	eye_sun = nav->get_helio_to_eye_mat() * Vec3f(0.0f, 0.0f, 0.0f);

	Mat4d mat = mat_local_to_parent;
	Mat4d parent_mat = Mat4d::identity();

	// \todo account for moon orbit precession (independent of parent)
	// also does not allow for multiple levels of precession

	const Planet *p = parent;

	bool myParent = true;
	while (p && p->parent) {

		// Some orbits are already precessed, namely elp82
		if(myParent && !useParentPrecession(lastJD)) {
			mat = Mat4d::translation(p->ecliptic_pos)
				* mat
				* p->rot_local_to_parent_unprecessed; 
		} else {
			mat = Mat4d::translation(p->ecliptic_pos)
				* mat
				* p->rot_local_to_parent; 
		}
		
		parent_mat = Mat4d::translation(p->ecliptic_pos)
			* parent_mat;

		p = p->parent;

		myParent = false;
	}


	// This removed totally the Planet shaking bug!!!
	mat = nav->get_helio_to_eye_mat() * mat;
	parent_mat = nav->get_helio_to_eye_mat() * parent_mat;

	// equivalent to below
	//	eye_planet = nav->get_helio_to_eye_mat() * get_heliocentric_ecliptic_pos();

	eye_planet = mat * Vec3f(0.0f, 0.0f, 0.0f);

	Vec3f lightDirection = eye_sun - eye_planet;
	sun_half_angle = atan(696000.0/AU/lightDirection.length());  // hard coded Sun radius!

	//	cout << sun_half_angle << " sun angle on " << englishName << endl;
	lightDirection.normalize();

	if (!drawHomePlanet && this == nav->getHomePlanet()) {
		if (rings) rings->draw(prj,mat,1000.0,lightDirection,eye_planet,radius);
		return 0;
	}
	

	// Compute the 2D position and check if in the screen
	float screen_sz = get_on_screen_size(prj, nav);
	//  float viewport_left = prj->getViewportPosX();
	//  float viewport_bottom = prj->getViewportPosY();

	// special case if big halo (Sun)
	float screen_size_with_halo = screen_sz;
	if (tex_big_halo && (big_halo_size > screen_sz))
		screen_size_with_halo = big_halo_size;

	// Check planet visible in viewport
	bool isVisible = prj->project_custom_check(Vec3f(0,0,0), screenPos, mat, (int)(screen_size_with_halo/2));

	bool visible = isVisible;
	if(selected) visible = true;  // always draw orbit if selected, even if not on screen

#ifdef LSS
	// always draw orbits
	visible = true;
#endif

	// fade orbits as enter/leave view
	if (is_satellite && prj->get_fov()>30) {

		// If not in the parent planet system
		// OR one of the other sister moons do not draw
		if (parent != nav->getHomePlanet() &&
			parent != nav->getHomePlanet()->get_parent() &&
			get_earth_equ_pos(nav).length() > 1) {  // If further than 1 AU of object if flying
			visibilityFader = false; // orbits will fade in and out now
		} else visibilityFader = visible;
	} else visibilityFader = visible;

	// Always draw orbits since they may be fading out
	// Also, can't draw orbit using depth buffer for planets...
	//		if( is_satellite &&
	//			parent != nav->getHomePlanet() &&
	//			parent != nav->getHomePlanet()->get_parent()) draw_orbit_3d(nav, prj, parent_mat);

	if ( is_satellite) draw_orbit_3d(nav, prj, parent_mat);
	else {
		draw_orbit_2d(nav, prj, parent_mat);
		//			cout << "2d orbit: " << englishName << endl;
	}

	// Do not draw anything else if was not visible
	// TODO hints could draw so they fade out
#ifdef LSS
	if(1) {
#else
	if(isVisible) {
#endif
		// Draw the name, and the circle if it's not too close from the body it's turning around
		// this prevents name overlaping (ie for jupiter satellites)
		float ang_dist = 300.f*atan(get_ecliptic_pos().length()/get_earth_equ_pos(nav).length())/prj->get_fov();
		if (ang_dist==0.f) ang_dist = 1.f; // if ang_dist == 0, the Planet is sun..

		draw_trail(nav, prj);

		if (ang_dist>0.25) {
			if (ang_dist>1.f) ang_dist = 1.f;
			//glColor4f(0.5f*ang_dist,0.5f*ang_dist,0.7f*ang_dist,1.f*ang_dist);
			draw_hints(nav, prj);
		}

		if (depthTest) glEnable(GL_DEPTH_TEST);

		//	if (screen_sz > 1 && screen_sz*4 >= screen_size_with_halo) {  // huge improvements in performance
		if (screen_sz > 1) {  // huge improvement in performance

			if (rings) {

				glEnable(GL_DEPTH_TEST);

				// flat texture is for images or simple markers (much faster than sphere)
				if (flat_texture) draw_image(prj, mat, screen_sz);
				else {
//			draw_axis(prj, mat, screen_sz);
					draw_sphere(prj, nav, mat, screen_sz);  // SL O O W
				}
				rings->draw(prj,mat,screen_sz,lightDirection,eye_planet,radius);
			} else {
				if (stencil) glEnable(GL_STENCIL_TEST);

				// flat texture is for images or simple markers (much faster than sphere)
				if (flat_texture) draw_image(prj, mat, screen_sz);
				else {
//			  draw_axis(prj, mat, screen_sz);
					draw_sphere(prj, nav, mat, screen_sz);  // SL O O W
					//					cout << "drew " << englishName << endl;
				}

				if (stencil) glDisable(GL_STENCIL_TEST);
			}
		}

		if (tex_halo) {
			// Workaround for depth buffer precision and near planets
			if(get_earth_equ_pos(nav).length() > 0.0001) {
				glDisable(GL_DEPTH_TEST);
				if (flag_point) draw_point_halo(nav, prj, eye);
				else draw_halo(nav, prj, eye);
			} else {
				if (flag_point) draw_point_halo(nav, prj, eye, mat);
				else draw_halo(nav, prj, eye, mat);
			}
		}

		glDisable(GL_DEPTH_TEST);

		if (tex_big_halo) draw_big_halo(nav, prj, eye);	
	}
//}
	double distanceSquared =
	    (screenPos[0] - previousScreenPos[0]) *
	    (screenPos[0] - previousScreenPos[0]) +
	    (screenPos[1] - previousScreenPos[1]) *
	    (screenPos[1] - previousScreenPos[1]);
	previousScreenPos = screenPos;
	return distanceSquared;
}

void Planet::draw_hints(const Navigator* nav, const Projector* prj)
{

	//	printf("Out level %f\n", hint_fader.getInterstate());

	if (!hint_fader.getInterstate()) return;

	prj->set_orthographic_projection();    // 2D coordinate

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	// Draw nameI18 + scaling if it's not == 1.
	float tmp = 10.f + get_on_screen_size(prj, nav)/sphere_scale/2.f; // Shift for nameI18 printing

	glColor4f(label_color[0], label_color[1], label_color[2],hint_fader.getInterstate());

	prj->getFlagGravityLabels() ?
	prj->print_gravity180(planet_name_font, screenPos[0],screenPos[1], getSkyLabel(nav), 1, tmp, tmp) :
		planet_name_font->print(screenPos[0]+tmp,screenPos[1]+tmp, getSkyLabel(nav), 1);

	// hint disapears smoothly on close view
	tmp -= 10.f;
	if (tmp<1) tmp=1;
	glColor4f(label_color[0], label_color[1], label_color[2],hint_fader.getInterstate()/tmp);

	// Draw the 2D small circle
	glCircle(screenPos, 8);
	prj->reset_perspective_projection();		// Restore the other coordinate
}

void Planet::draw_sphere(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	// Adapt the number of facets according with the size of the sphere for optimization
	int nb_facet = (int)(screen_sz/4);

	if (nb_facet % 2) nb_facet--;

	if (nb_facet<10) nb_facet = 10;
	if (nb_facet>80) nb_facet = 80;

//	cout << "Sphere for " << englishName << " with " << nb_facet << " facets.\n";

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	if (flag_lighting) glEnable(GL_LIGHTING);
	else {
		glDisable(GL_LIGHTING);
		glColor3fv(color);
	}
	glBindTexture(GL_TEXTURE_2D, tex_map->getID());


	// Rotate and add an extra quarter rotation so that the planet texture map
	// fits to the observers position. No idea why this is necessary,
	// perhaps some openGl strangeness, or confusing sin/cos.

	//prj->sSphere(radius*sphere_scale, one_minus_oblateness, nb_facet, nb_facet,
	//             mat * Mat4d::zrotation(M_PI/180*(axis_rotation + 90.)));


	// TODO Shaders need to be consolidated and rationalized in a better way

	bool useShader = Planet::flagShaders && (bumpEnable || nightEnable || ringedEnable);
	if(ringedEnable && !isSatellite() && !hasSatellite() && !rings) useShader = false;

	if(useShader) {

		programObject.enable();

		if (bumpEnable)
		{
			programObject.setParam("TextureMap", tex_map);
			programObject.setParam("NormalTexture", tex_norm);
		}


		if (nightEnable) {

			programObject.setParam("DayTexture", tex_map);
			programObject.setParam("NightTexture", tex_night);
			programObject.setParam("SpecularTexture", tex_specular);

			if(tex_cloud) {
				programObject.setParam("CloudTexture", tex_cloud);
				programObject.setParam("CloudNormalTexture", tex_norm_cloud);
			}
			programObject.setParam("Clouds", Planet::flagClouds && tex_norm_cloud);

		}

		if (ringedEnable) {

			// Handle generic or ringed planets
			programObject.setParam("Texture", tex_map);

			if(rings && ring_shadow) {
				programObject.setParam("RingTexture", rings->get_tex());
				programObject.setParam("RingInnerRadius", rings->get_inner_radius());
				programObject.setParam("RingOuterRadius", rings->get_size());
				programObject.setParam("Ambient", 0.04);
			} else {
				programObject.setParam("RingOuterRadius", 0.0);  // No ring shadow to render
				programObject.setParam("Ambient", 0.0);
			}

			if(!flag_lighting) programObject.setParam("Ambient", 1.0);

		}

		if( bumpEnable || nightEnable || ringedEnable ) {

			// Handle moon and parent shadowing
			int index=1;

			if(tex_eclipse_map->getID()) {
				programObject.setParam("LightPosition", eye_sun);
				programObject.setParam("ShadowTexture", tex_eclipse_map);
				programObject.setParam("SunHalfAngle", sun_half_angle);
				
				double length;
				double moonDotLight;
				Vec3d tmp(0,0,0);
				Vec3d tmp2(0.4, 0.12, 0.0);
				
				if(getEnglishName() == "Moon") programObject.setParam("UmbraColor", tmp2);
				else programObject.setParam("UmbraColor", tmp);
				
				Vec3d planet_helio = get_heliocentric_ecliptic_pos();
				Vec3d light = -planet_helio;
				light.normalize();
				
				list<Planet*>::iterator iter;
				for(iter=satellites.begin(); iter!=satellites.end() && index <= 4; iter++) {
					tmp2 = (*iter)->get_heliocentric_ecliptic_pos() - planet_helio;
					length = tmp2.length();
					tmp2.normalize();
					moonDotLight = tmp2.dot(light);
					if(moonDotLight > 0 && length*sin(acos(moonDotLight)) <= radius + 2*(*iter)->getRadius()) {
						//cout << "Adding satellite: " << (*iter)->getEnglishName() << endl;
						//cout << (*iter)->getEnglishName() << " had dot of " << tmp2.dot(light) << endl;
						tmp = nav->get_helio_to_eye_mat() * (*iter)->get_heliocentric_ecliptic_pos();
						
						ostringstream oss;		
						oss << index;
						
						programObject.setParam(string("MoonPosition") + oss.str(), tmp);
						programObject.setParam(string("MoonRadius") + oss.str(), (*iter)->getRadius());
						index++;
					}
				}
				
				if(isSatellite() && parent->getEnglishName() != "Sun" && index <= 4) {
					// Parent may shadow this satellite
					ostringstream oss;		
					oss << index;
					
					tmp = nav->get_helio_to_eye_mat() * parent->get_heliocentric_ecliptic_pos();
					programObject.setParam(string("MoonPosition") + oss.str(), tmp);
					programObject.setParam(string("MoonRadius") + oss.str(), parent->getRadius());
					index++;
				}
				
			}

			// clear any leftover values
			for(; index<=4; index++) {
				ostringstream oss;		
				oss << index;
				programObject.setParam(string("MoonRadius") + oss.str(), 0.0); // No moon data
			}
		}
	}

	prj->sSphere(radius, sphere_scale, one_minus_oblateness, nb_facet, nb_facet,
				 mat * Mat4d::zrotation(M_PI/180*(axis_rotation + 90.)), 0, 
				 useShader);

	if(useShader) programObject.disable();
	else if(Planet::flagClouds && tex_cloud && tex_cloud->getID()) {
		// Non shader cloud fallback
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, tex_cloud->getID());
		prj->sSphere((radius * 1.002)*sphere_scale, one_minus_oblateness, nb_facet, nb_facet,
					 mat * Mat4d::zrotation(M_PI/180*(axis_rotation + 90.)), 0, false);
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

}


// draw the texture as a flat image at the planet location
void Planet::draw_image(const Projector* prj, const Mat4d& mat, float screen_sz)
{
	prj->set_orthographic_projection();    	// 2D coordinate

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, tex_map->getID());
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);
	glTranslatef(screenPos[0], screenPos[1], 0.f);
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex3f(-screen_sz/2,-screen_sz/2,0.f);
	glTexCoord2i(1,0);
	glVertex3f( screen_sz/2,-screen_sz/2,0.f);
	glTexCoord2i(1,1);
	glVertex3f( screen_sz/2,screen_sz/2,0.f);
	glTexCoord2i(0,1);
	glVertex3f(-screen_sz/2,screen_sz/2,0.f);
	glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate

}


// Orthographic version
void Planet::draw_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	float cmag;

	// johannes 20070216 mag bug fix
	float fov_q = prj->get_fov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	float rmag = sqrtf(eye->adapt_luminance((expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * Planet::object_scale;

	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<1.2f) {
		if (rmag<0.3f) return;
		cmag=rmag*rmag/1.44f;
		rmag=1.2f;
	} else {

		float limit = Planet::object_size_limit/1.8;
		if (rmag>limit) {
			rmag = limit + sqrt(rmag-limit)/(limit + 1);

			if (rmag > Planet::object_size_limit) {
				rmag = Planet::object_size_limit;
			}
		}

		/*
		  if(rmag>5.f) {
		  rmag=5.f+sqrt(rmag-5)/6;
		if (rmag>9.f)
		{
		  rmag=9.f;
		}
		}
		*/

	}

	glBlendFunc(GL_ONE, GL_ONE);
	float screen_r = get_on_screen_size(prj, nav);
	cmag *= 0.5*rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {
		cmag*=rmag/screen_r;
		rmag = screen_r;
	}

	prj->set_orthographic_projection();    	// 2D coordinate

	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);
	glTranslatef(screenPos[0], screenPos[1], 0.f);
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex3f(-rmag, rmag,0.f);	// Bottom Left
	glTexCoord2i(1,0);
	glVertex3f( rmag, rmag,0.f);	// Bottom Right
	glTexCoord2i(1,1);
	glVertex3f( rmag,-rmag,0.f);	// Top Right
	glTexCoord2i(0,1);
	glVertex3f(-rmag,-rmag,0.f);	// Top Left
	glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate
}


// Depth buffer version
// NOTE: may flash if draw near center of planet while tracking
void Planet::draw_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, const Mat4d& mat)
{
	float cmag;

	// johannes 20070216 mag bug fix
	float fov_q = prj->get_fov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	float rmag = sqrtf(eye->adapt_luminance((expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * Planet::object_scale;

	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<1.2f) {
		if (rmag<0.3f) return;
		cmag=rmag*rmag/1.44f;
		rmag=1.2f;
	} else {

		float limit = Planet::object_size_limit/1.8;
		if (rmag>limit) {
			rmag = limit + sqrt(rmag-limit)/(limit + 1);

			if (rmag > Planet::object_size_limit) {
				rmag = Planet::object_size_limit;
			}
		}
	}

	float screen_r = get_on_screen_size(prj, nav);
	cmag *= 0.5*rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {

		if(rmag < 10*screen_r ) return;  // No need to draw if body is drawn fully (interferes with orbit lines)

		cmag*=rmag/screen_r;
		rmag = screen_r;
	}

	glPushMatrix();
	glLoadMatrixd(mat);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());
	glEnable(GL_TEXTURE_2D);
	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);

	glBegin(GL_QUADS);

	// -5*radius below to draw just in front of the body and not bisect 3d body drawing

	glTexCoord2i(0,0);  // Bottom Left
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]-rmag, screenPos[2]-5*radius, mat);
	glTexCoord2i(1,0);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]-rmag, screenPos[2]-5*radius, mat);
	glTexCoord2i(1,1);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]+rmag, screenPos[2]-5*radius, mat);
	glTexCoord2i(0,1);
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]+rmag, screenPos[2]-5*radius, mat);

	glEnd();

	glPopMatrix();

}


// Orthographic version
void Planet::draw_point_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	float cmag;

	//	rmag = eye->adapt_luminance(expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) +
	//                                            12.12331f)) * 108064.73f);
	// rmag = rmag/powf(prj->get_fov(),0.85f)*10.f;

// already in svn from johannes 20070216 mag bug fix
	float fov_q = prj->get_fov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	// float rmag = sqrtf(eye->adapt_luminance(term1 * fov_q)) * 30.f;
	float rmag = sqrtf(eye->adapt_luminance((expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) +
	                                        12.12331f)) * 108064.73f) * fov_q)) *
	             30.f * Planet::object_scale;


	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<0.3f) return;
	cmag=rmag*rmag/(1.4f*1.4f);
	rmag=1.4f;

	glBlendFunc(GL_ONE, GL_ONE);
	float screen_r = get_on_screen_size(prj, nav);
	cmag *= rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {
		cmag*=rmag/screen_r;
		rmag = screen_r;
	}
	prj->set_orthographic_projection();    	// 2D coordinate

	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);
	glTranslatef(screenPos[0], screenPos[1], 0.f);
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex3f(-rmag, rmag,0.f);	// Bottom Left
	glTexCoord2i(1,0);
	glVertex3f( rmag, rmag,0.f);	// Bottom Right
	glTexCoord2i(1,1);
	glVertex3f( rmag,-rmag,0.f);	// Top Right
	glTexCoord2i(0,1);
	glVertex3f(-rmag,-rmag,0.f);	// Top Left
	glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate
}


// Depth buffer version
void Planet::draw_point_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, const Mat4d& mat)
{
	float cmag;

	//	rmag = eye->adapt_luminance(expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) +
	//                                            12.12331f)) * 108064.73f);
	// rmag = rmag/powf(prj->get_fov(),0.85f)*10.f;

// already in svn from johannes 20070216 mag bug fix
	float fov_q = prj->get_fov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	// float rmag = sqrtf(eye->adapt_luminance(term1 * fov_q)) * 30.f;
	float rmag = sqrtf(eye->adapt_luminance((expf(-0.92103f*(compute_magnitude(nav->get_observer_helio_pos()) +
	                                        12.12331f)) * 108064.73f) * fov_q)) *
	             30.f * Planet::object_scale;


	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<0.3f) return;
	cmag=rmag*rmag/(1.4f*1.4f);
	rmag=1.4f;

	glBlendFunc(GL_ONE, GL_ONE);
	float screen_r = get_on_screen_size(prj, nav);
	cmag *= rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {
		cmag*=rmag/screen_r;
		rmag = screen_r;
	}


	glPushMatrix();
	glLoadMatrixd(mat);

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());
	glEnable(GL_TEXTURE_2D);
	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);

	glBegin(GL_QUADS);

	glTexCoord2i(0,0);  // Bottom Left
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]-rmag, screenPos[2], mat);
	glTexCoord2i(1,0);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]-rmag, screenPos[2], mat);
	glTexCoord2i(1,1);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]+rmag, screenPos[2], mat);
	glTexCoord2i(0,1);
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]+rmag, screenPos[2], mat);

	glEnd();

	glPopMatrix();

}


void Planet::draw_big_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	glBlendFunc(GL_ONE, GL_ONE);
	float screen_r = get_on_screen_size(prj, nav);

	float rmag = big_halo_size/2;

	float cmag = rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r*2) {
		cmag*=rmag/(screen_r*2);
		rmag = screen_r*2;
	}

	prj->set_orthographic_projection();    	// 2D coordinate

	glBindTexture(GL_TEXTURE_2D, tex_big_halo->getID());
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);
	glTranslatef(screenPos[0], screenPos[1], 0.f);
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex3f(-rmag, rmag,0.f);	// Bottom Left
	glTexCoord2i(1,0);
	glVertex3f( rmag, rmag,0.f);	// Bottom Right
	glTexCoord2i(1,1);
	glVertex3f( rmag,-rmag,0.f);	// Top Right
	glTexCoord2i(0,1);
	glVertex3f(-rmag,-rmag,0.f);	// Top Left
	glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate
}

/* Depth buffer version flashes when tracking...!

void Planet::draw_big_halo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, const Mat4d& mat)
{

	float screen_r = get_on_screen_size(prj, nav);

	float rmag = big_halo_size/2;

	float cmag = rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r*2) {
		cmag*=rmag/(screen_r*2);
		rmag = screen_r*2;
	}

	glBindTexture(GL_TEXTURE_2D, tex_big_halo->getID());

	glPushMatrix();
	glLoadMatrixd(mat);

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_TEXTURE_2D);

	glColor3f(color[0]*cmag, color[1]*cmag, color[2]*cmag);

	glBegin(GL_QUADS);

	glTexCoord2i(0,0);  // Bottom Left
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]-rmag, screenPos[2], mat);
	glTexCoord2i(1,0);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]-rmag, screenPos[2], mat);
	glTexCoord2i(1,1);
	prj->oVertex3(screenPos[0]+rmag, screenPos[1]+rmag, screenPos[2], mat);
	glTexCoord2i(0,1);
	prj->oVertex3(screenPos[0]-rmag, screenPos[1]+rmag, screenPos[2], mat);

	glEnd();

	glPopMatrix();

}

*/

Ring::Ring(double radius_min,double radius_max,const string &texname, const string &path)
		:radius_min(radius_min),radius_max(radius_max)
{

	tex = new s_texture((path!=""), path + texname,TEX_LOAD_TYPE_PNG_ALPHA,1);

	if(!Planet::flagShaders || !RingShader::instance()->isSupported()) {
		cout << "Unable to use ring shader.\n";
		shaderEnable = false;
	} else {
		shaderEnable = true;
		programObject.attachShader(RingShader::instance());
	}
}

Ring::~Ring(void)
{
	if (tex) delete tex;
	tex = NULL;
}

void Ring::draw(const Projector* prj,const Mat4d& mat,double screen_sz, Vec3f& lightDirection, Vec3f& planetPosition, float planetRadius)
{

	screen_sz -= 50;
	screen_sz /= 350.0;

	if (screen_sz < 0.0) screen_sz = 0.0;
	else if (screen_sz > 1.0) screen_sz = 1.0;
	// const int slices = 128+(int)((256-128)*screen_sz);
	//const int stacks = 8+(int)((32-8)*screen_sz);

// perf testing
	const int slices = 64+(int)((256-64)*screen_sz);
	const int stacks = 4+(int)((16)*screen_sz);

	// Normal transparency mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glRotatef(axis_rotation + 180.,0.,0.,1.);

	glColor3f(1.0f, 1.0f, 1.0f); // For saturn only..
	//glColor3f(1.0f, 0.88f, 0.82f); // For saturn only..

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glBindTexture (GL_TEXTURE_2D, tex->getID());

	// solve the ring wraparound by culling:
	// decide if we are above or below the ring plane
	const double h = mat.r[ 8]*mat.r[12]
	                 + mat.r[ 9]*mat.r[13]
	                 + mat.r[10]*mat.r[14];

	if (shaderEnable) {
		RingShader::instance()->setLightDirection(lightDirection);
		RingShader::instance()->setPlanetPosition(planetPosition);
		RingShader::instance()->setTexMap(tex->getID());
		RingShader::instance()->setPlanetRadius(planetRadius);
		RingShader::instance()->setSunnySideUp(h>0.0);
	}

	if(shaderEnable) programObject.enable();
	prj->sRing(radius_min,radius_max,(h<0.0)?slices:-slices,stacks, mat, 0, shaderEnable);
	if(shaderEnable) programObject.disable();

	glDisable(GL_CULL_FACE);
}


// draw orbital path of Planet
void Planet::draw_orbit_3d(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{

	if (!orbit_fader.getInterstate()) return;
	if (!visibilityFader.getInterstate()) return;

	// only draw moon orbits as zoom in
	// TODO: could fade in

	Vec3d onscreen;

	if (!re.sidereal_period) return; // TODO change name to visualization_period

	glPushMatrix();
	glLoadMatrixd(mat);

	glEnable(GL_DEPTH_TEST);

	// Normal transparency mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	if (local_orbit_color[3] > 0) {
		glColor4f(local_orbit_color[0], local_orbit_color[1], local_orbit_color[2], 
				  orbit_fader.getInterstate()*visibilityFader.getInterstate());
	} else {
		if (is_satellite) glColor4f(satellite_orbit_color[0], satellite_orbit_color[1], 
									satellite_orbit_color[2], 
									orbit_fader.getInterstate()*visibilityFader.getInterstate());
		else glColor4f(planet_orbit_color[0], planet_orbit_color[1], planet_orbit_color[2], 
					   orbit_fader.getInterstate()*visibilityFader.getInterstate());
	}

	Vec3d p[4];
	Vec3d pos = get_ecliptic_pos();
	int d, offset;
	bool on = false;
	bool spline = true;

	for ( int n=0; n<=ORBIT_SEGMENTS; n++) {
		
		d = n;
		offset = abs(ORBIT_SEGMENTS/2 - d);

		if(!close_orbit && (n < 2 || ORBIT_SEGMENTS - n < 3)) {
			// Not enough points at ends to create a spline
			if(n == ORBIT_SEGMENTS) break;
			p[2] = orbitPoint[d];
			spline = false;			
		} else {
			spline = true;
			// Draw interpolated spline
			for(int i=0; i<4; i++) {
				if((d + i-1) == ORBIT_SEGMENTS/2) {
					// special case - use current Planet position as center vertex so that draws
					// on it's orbit all the time (since segmented rather than fully smooth curve)
					p[i] = pos;
				} else if((d + i-1) < 0)
					p[i] = orbitPoint[ORBIT_SEGMENTS + (d + i-1)];
				else if((d + i-1) >= ORBIT_SEGMENTS)
					p[i] = orbitPoint[(d + i-1) - ORBIT_SEGMENTS];
				else
					p[i] = orbitPoint[d + i-1];
			}
		}

		if (prj->project_custom(p[2], onscreen, mat)) {
			if (!on) glBegin(GL_LINE_STRIP);
			
			if(spline) drawSpline3d(prj, mat, (offset<2 ? 1 : 3), p[0], p[1], p[2], p[3]);
			else prj->sVertex3( orbitPoint[d][0], orbitPoint[d][1], orbitPoint[d][2], mat);
			
			on = true;
		} else if ( on ) {
			glEnd();
			on = false;
		}
	}
	
	if (on) glEnd();

	glPopMatrix();
	glDisable(GL_DEPTH_TEST);
}

// draw orbital path of Planet in 2d projection
// for when clipping volume is too small for 3d drawing
void Planet::draw_orbit_2d(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{

	if (!orbit_fader.getInterstate()) return;
	if (!visibilityFader.getInterstate()) return;

	Vec3d onscreen;

	if (!re.sidereal_period) return;

	prj->set_orthographic_projection();    // 2D coordinate

	// Normal transparency mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	if (local_orbit_color[3] > 0) {
		glColor4f(local_orbit_color[0], local_orbit_color[1], local_orbit_color[2], 
				  orbit_fader.getInterstate()*visibilityFader.getInterstate());
	} else {
		if (is_satellite) glColor4f(satellite_orbit_color[0], satellite_orbit_color[1], 
									satellite_orbit_color[2], 
									orbit_fader.getInterstate()*visibilityFader.getInterstate());
		else glColor4f(planet_orbit_color[0], planet_orbit_color[1], planet_orbit_color[2], 
					   orbit_fader.getInterstate()*visibilityFader.getInterstate());
	}

	Vec3d p[4];
	Vec3d pos = get_ecliptic_pos();
	int d, offset;
	bool on = false;
	bool spline = true;

	for ( int n=0; n<=ORBIT_SEGMENTS; n++) {
		
		d = n;
		offset = abs(ORBIT_SEGMENTS/2 - d);

		if(!close_orbit && (n < 2 || ORBIT_SEGMENTS - n < 3)) {
			// Not enough points at ends to create a spline
			if(n == ORBIT_SEGMENTS) break;
			p[2] = orbitPoint[d];
			spline = false;			
		} else {
			spline = true;
			// Draw interpolated spline
			for(int i=0; i<4; i++) {
				if((d + i-1) == ORBIT_SEGMENTS/2) {
					// special case - use current Planet position as center vertex so that draws
					// on it's orbit all the time (since segmented rather than fully smooth curve)
					p[i] = pos;
				} else if((d + i-1) < 0)
					p[i] = orbitPoint[ORBIT_SEGMENTS + (d + i-1)];
				else if((d + i-1) >= ORBIT_SEGMENTS)
					p[i] = orbitPoint[(d + i-1) - ORBIT_SEGMENTS];
				else
					p[i] = orbitPoint[d + i-1];
			}
		}

		if (prj->project_custom(p[2], onscreen, mat)) {
			if (!on) glBegin(GL_LINE_STRIP);
			
			if(spline) drawSpline2d(prj, mat, (offset < 2 ? 1 : 15), p[0], p[1], p[2], p[3]);
			else glVertex3d(onscreen[0], onscreen[1], 0);
			
			on = true;
		} else if ( on ) {
			glEnd();
			on = false;
		}
	}

	if (on) glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}


// draw trail of Planet as seen from earth
void Planet::draw_trail(const Navigator * nav, const Projector* prj)
{
	float fade = trail_fader.getInterstate();

	if (!fade) return;

	if (trail.empty()) return;

	//  if(!re.sidereal_period) return;   // limits to planets

	Vec3d onscreen1;
	Vec3d onscreen2;

	prj->set_orthographic_projection();    // 2D coordinate

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	list<TrailPoint>::iterator iter;
	list<TrailPoint>::iterator begin = trail.begin();
	begin++;

	float segment = 0;

	//	glColor3fv(trail_color*fade);

	// 	cout << trail.size() << " " << MaxTrail << endl;

	bool line_on = 0;

	// TODO: take out viewport checks so doesn't flicker on/off when zoomed in?

	// draw final segment to finish at current Planet position
	if ( !first_point && prj->project_earth_equ_line_check( (*trail.begin()).point, onscreen2, get_earth_equ_pos(nav), onscreen1) ) {

		if (!line_on) {
			glBegin(GL_LINE_STRIP);
			line_on = 1;
		}

		glColor3fv(trail_color*fade);
		glVertex3d(onscreen1[0], onscreen1[1], 0);

		//    glColor3fv(trail_color*fade* (1 - .9*segment/MaxTrail) );
		glColor3fv(trail_color*fade* (1 - .9*segment/trail.size()) );
		glVertex3d(onscreen2[0], onscreen2[1], 0);
	}

	if (trail.size() > 1) {

		for (iter=begin; iter != trail.end(); iter++) {

			segment++;

			if ( prj->project_earth_equ_check( (*iter).point, onscreen1 )) {
				if (!line_on) {
					glBegin(GL_LINE_STRIP);
					line_on = 1;
				}

				//	glColor3fv(trail_color*fade* (1 - .9*segment/MaxTrail));
				glColor3fv(trail_color*fade* (1 - .9*segment/trail.size()));

				glVertex3d(onscreen1[0], onscreen1[1], 0);

			} else {
				line_on = 0;
				glEnd();
			}

		}
	}

	if (line_on) glEnd();

	prj->reset_perspective_projection();		// Restore the other coordinate

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

}

/*
// draw trail of Planet as seen from earth
void Planet::draw_trail(const Navigator * nav, const Projector* prj)
{
float fade = trail_fader.getInterstate();

if(!fade) return;

Vec3d onscreen1;
Vec3d onscreen2;

//  if(!re.sidereal_period) return;   // limits to planets

prj->set_orthographic_projection();    // 2D coordinate

glEnable(GL_BLEND);
glDisable(GL_LIGHTING);
glDisable(GL_TEXTURE_2D);

list<TrailPoint>::iterator iter;
list<TrailPoint>::iterator nextiter;
list<TrailPoint>::iterator begin = trail.begin();
//  begin++;

float segment = trail.size();

glColor3fv(trail_color*fade* (1 - .9*segment/MaxTrail) );
//	glColor3fv(trail_color*fade);

//	cout << trail.size() << " " << MaxTrail << endl;

bool line_on = 0;

if(trail.begin() != trail.end())
{

  nextiter = trail.end();
  nextiter--;

  for( iter=nextiter; iter != begin; iter--)
    {

      nextiter--;
      if( prj->project_earth_equ_line_check( (*iter).point, onscreen1, (*(nextiter)).point, onscreen2) )
	{
	  if(!line_on) {
	    glBegin(GL_LINE_STRIP);
	    line_on = 1;

	    glVertex3d(onscreen1[0], onscreen1[1], 0);
	  }

	  glColor3fv(trail_color*fade* (1 - .9*segment/MaxTrail));

	  glVertex3d(onscreen2[0], onscreen2[1], 0);
	  //glEnd();
	} else {
	  line_on = 0;
	  glEnd();
	}

      segment--;

    }
}

// draw final segment to finish at current Planet position
 if( !first_point && prj->project_earth_equ_line_check( (*trail.begin()).point, onscreen1, get_earth_equ_pos(nav), onscreen2) )
   {

     if(!line_on) {
       glBegin(GL_LINE_STRIP);
       line_on = 1;
     }

     glVertex3d(onscreen1[0], onscreen1[1], 0);

     glColor3fv(trail_color*fade);

     glVertex3d(onscreen2[0], onscreen2[1], 0);
   }

 if(line_on) glEnd();

 prj->reset_perspective_projection();		// Restore the other coordinate

 glEnable(GL_BLEND);
 glDisable(GL_LIGHTING);
 glEnable(GL_TEXTURE_2D);

}
*/

// update trail points as needed
void Planet::update_trail(const Navigator* nav)
{
	if (!trail_on) return;

	double date = nav->get_JDay();

	int dt=0;
	if (first_point || (dt=abs(int((date-last_trailJD)/DeltaTrail))) > MaxTrail) {
		dt=1;
		// clear old trail
		trail.clear();
		first_point = 0;
	}

	// Note that when jump by a week or day at a time, loose detail on trails
	// particularly for moon (if decide to show moon trail)

	// add only one point at a time, using current position only
	if (dt) {
		last_trailJD = date;
		TrailPoint tp;
		Vec3d v = get_heliocentric_ecliptic_pos();
		//      trail.push_front( nav->helio_to_earth_equ(v) );  // centered on earth
		tp.point = nav->helio_to_earth_pos_equ(v);
		tp.date = date;
		trail.push_front( tp );

		//      if( trail.size() > (unsigned int)MaxTrail ) {
		if ( trail.size() > (unsigned int)MaxTrail ) {
			trail.pop_back();
		}
	}

	// because sampling depends on speed and frame rate, need to clear out
	// points if trail gets longer than desired

	list<TrailPoint>::iterator iter;
	list<TrailPoint>::iterator end = trail.end();

	for ( iter=trail.begin(); iter != end; iter++) {
		if ( fabs((*iter).date - date)/DeltaTrail > MaxTrail ) {
			trail.erase(iter, end);
			break;
		}
	}
}

// Start/stop accumulating new trail data (clear old data)
void Planet::startTrail(bool b)
{
	if (b) {
		first_point = 1;
		//  printf("trail for %s: %f\n", name.c_str(), re.sidereal_period);
		// only interested in trails for planets
		// if(parent && parent->getEnglishName() == "Sun") trail_on = 1;
		if (!is_satellite && parent) trail_on = 1; // No trail for Sun or moons
		//	if(re.sidereal_period > 0) trail_on = 1;
	} else {
		trail_on = 0;
	}
}

void Planet::translateName(Translator& trans) {
	nameI18 = trans.translateUTF8(englishName);

	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_PLANET;
		if( cursor.select(q) ) {
			cursor->nameI18 = nameI18.c_str();
			cursor.update();
		}
		SharedData::Instance()->DB()->commit();
	}
}

void Planet::update(int delta_time, double JD)
{
	// Check orbit validity range
	if(hasValidityPeriod) {
		if(JD < startJD || JD > endJD) visibility = NONEXISTANT;
		else visibility = NORMAL;  // Note, always hidden bodies should not have validity periods or will become visible!
	}
	
	visibilityFader.update(delta_time);
	hint_fader.update(delta_time);
	orbit_fader.update(delta_time);
	trail_fader.update(delta_time);
}

void Planet::removeSatellite(Planet *planet)
{
	list<Planet *>::iterator iter;
	for (iter=satellites.begin(); iter != satellites.end(); iter++) {
		//      if( (*iter)->getEnglishName() == planet->getEnglishName() ) {
		if ( (*iter) == planet ) {
			satellites.erase(iter);
			break;
		}
	}
	/*
	// DEBUG
	cout << "Satellites for " << englishName << endl;
	for(iter=satellites.begin(); iter != satellites.end(); iter++)
	  cout << "\t" << (*iter)->getEnglishName() << endl;
	*/
}

// Update bounding radii from child up to parent(s)
void Planet::updateBoundingRadii()
{
	calculateBoundingRadius();
	Planet *p;
	p = parent;
	while (p) {
		p->calculateBoundingRadius();
		p = p->parent;
	}
}

// Calculate a bounding radius in AU
// For a planet with satellites, this bounds the most
// distant satellite orbit
// for planets with no elliptical satellites, this is the ring (if one)
// or planet radius

// Caches result until next call, can retrieve with getBoundingRadius

double Planet::calculateBoundingRadius()
{
	// TODO: only have boundary function for elliptical orbits right now

	if ( visibility != NORMAL && satellites.empty() ) {
		boundingRadius = -1;
		return -1;  // special case, not visible
	}

	double d = radius;

	if (rings) d = rings->get_size();

	list<Planet *>::const_iterator iter;
	list<Planet *>::const_iterator end = satellites.end();

	//	cout << englishName << endl;

	double r;
	for ( iter=satellites.begin(); iter != end; iter++) {

		r = (*iter)->getBoundingRadius();

		//		cout << "\t" << (*iter)->getEnglishName() << " : " << r << endl;


		if ( r > d ) d = r;
	}

	// if we are a planet, we want the boundary radius including all satellites
	// if we are a satellite, we want the full orbit radius as well
	if ( is_satellite ) {
		//		cout << "Sat: " << d << " bnd rad: " << orbit_bounding_radius << endl;
		if ( d > orbit_bounding_radius + radius ) boundingRadius = d;
		else boundingRadius = orbit_bounding_radius + radius;
	} else boundingRadius = d;

	//  cout << "Bounding radius for " << englishName << " = " << boundingRadius << endl;

	return boundingRadius;
}

// retrieve cached value

double Planet::getBoundingRadius() const
{
	return boundingRadius;
}


// See if simple planet not needing depth buffer
// (no rings or satellites)
// 20080805 or it has satellites but orbit lines are not on

bool Planet::isSimplePlanet()
{

	if (rings) return false;

	if ( satellites.empty() ) return true;

	// satellites, but orbits not drawing
	//  if( !orbit_fader ) return true;

	return false;

}

// TEST draw planet axis
void Planet::draw_axis(const Projector* prj, const Mat4d& mat, float screen_sz)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glColor3f(1,0,0);

	glPushMatrix();
	glLoadMatrixd(mat);

	glBegin(GL_LINE_STRIP);

	prj->sVertex3(0, 0,  1.5 * radius * sphere_scale, mat);
	prj->sVertex3(0, 0, -1.5 * radius * sphere_scale, mat);

	glEnd();

	glPopMatrix();
}


// Catmull-Rom Spline
// 0 >= t <= 1, returns point on curve between points v1 and v2

Vec3d Planet::calculateSplinePoint(double t, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3)
{
	Vec3d c2, c3, c4;

	c2 = -.5*v0 + .5*v2;
	c3 = v0 -2.5*v1 + 2.*v2 -0.5*v3;
	c4 = -0.5*v0 + 1.5*v1 -1.5*v2 + 0.5*v3;

	return(((c4*t + c3)*t +c2)*t + v1);
}

// Draw line segments approximating an interpolated spline
void Planet::drawSpline3d(const Projector *prj, const Mat4d &mat, int segments, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3)
{
	Vec3d point;

	for(int n=1; n<=segments; n++) {
		point = calculateSplinePoint(double(n)/segments, v0, v1, v2, v3);
		prj->sVertex3( point[0], point[1], point[2], mat);
	}
}

// Draw line segments approximating an interpolated spline
void Planet::drawSpline2d(const Projector *prj, const Mat4d &mat, int segments, Vec3d &v0, Vec3d &v1, Vec3d &v2, Vec3d &v3)
{
	Vec3d point, onscreen;

	for(int n=1; n<=segments; n++) {
		point = calculateSplinePoint((double)n/segments, v0, v1, v2, v3);
		if(prj->project_custom(point, onscreen, mat))
			glVertex3d( onscreen[0], onscreen[1], 0);
	}
}

// Set valid range to observe body
void Planet::setValidityPeriod(double _startJD, double _endJD)
{
	hasValidityPeriod = true;
	startJD = _startJD;
	endJD = _endJD;
}


//
// ARTIFICIAL PLANET CLASS - for flying to new planets
//

ArtificialPlanet::ArtificialPlanet(Planet &orig) : 
	Planet(0, "", 0, 0, orig.getRadius(), orig.getOblateness(), 
		   Vec3f(0,0,0), 0, "", "", "",
		   NULL, false, true, false, false, 0,
		   "", "", "", "", "", 99, 0), dest(0),
	orig_name(orig.getEnglishName()), orig_name_i18n(orig.getNameI18n()),
	orig_radius(orig.getRadius())
{

//  radius = 0;
    // set parent = sun:
	if (orig.get_parent()) {
		parent = orig.getParent();
		while (parent->getParent()) parent = parent->getParent();
	} else {
		parent = &orig; // sun
	}

	re = orig.getRotationElements();
	setRotEquatorialToVsop87(orig.getRotEquatorialToVsop87());
	set_heliocentric_ecliptic_pos(orig.get_heliocentric_ecliptic_pos());
}

void ArtificialPlanet::setDest(Planet &dest) {
	ArtificialPlanet::dest = &dest;

	englishName = orig_name + "->" + dest.getEnglishName();
	nameI18 = orig_name_i18n + "->" + dest.getNameI18n();
	
	// rotation:
	const RotationElements &r(dest.getRotationElements());
	//  lastJD = StelApp::getInstance().getCore()->getNavigation()->getJDay();
	lastJD = dest.getLastJD();
	
	
//  re.offset = fmod(re.offset + 360.0*24* (lastJD-re.epoch)/re.period,
//                              360.0);
//  re.epoch = lastJD;
//  re.period = r.period;
//  re.offset = fmod(re.offset + 360.0*24* (r.epoch-re.epoch)/re.period,
//                              360.0);
//  re.epoch = r.epoch;
  
	re.offset = r.offset + fmod(re.offset - r.offset
								+ 360.0*( (lastJD-re.epoch)/re.period
                                          - (lastJD-r.epoch)/r.period),
								360.0);
	
	re.epoch = r.epoch;
	re.period = r.period;
	if (re.offset - r.offset < -180.f) re.offset += 360.f; else
		if (re.offset - r.offset >  180.f) re.offset -= 360.f;

	// Assume not much planet movement while travelling
	startPos = get_heliocentric_ecliptic_pos();

	orig_rotation = getRot(this);
}

void ArtificialPlanet::setRot(const Vec3d &r) {
	const double ca = cos(r[0]);
	const double sa = sin(r[0]);
	const double cd = cos(r[1]);
	const double sd = sin(r[1]);
	const double cp = cos(r[2]);
	const double sp = sin(r[2]);
	Mat4d m;
	m.r[ 0] = cd*cp;
	m.r[ 4] = sd;
	m.r[ 8] = cd*sp;
	m.r[12] = 0;
	m.r[ 1] = -ca*sd*cp -sa*sp;
	m.r[ 5] =  ca*cd;
	m.r[ 9] = -ca*sd*sp +sa*cp;
	m.r[13] = 0;
	m.r[ 2] =  sa*sd*cp -ca*sp;
	m.r[ 6] = -sa*cd;
	m.r[10] =  sa*sd*sp +ca*cp;
	m.r[14] = 0;
	m.r[ 3] = 0;
	m.r[ 7] = 0;
	m.r[11] = 0;
	m.r[15] = 1.0;
	setRotEquatorialToVsop87(m);
}

Vec3d ArtificialPlanet::getRot(Planet *p) {
	const Mat4d m(p->getRotEquatorialToVsop87());
	const double cos_r1 = sqrt(m.r[0]*m.r[0]+m.r[8]*m.r[8]);
	Vec3d r;
	r[1] = atan2(m.r[4],cos_r1);
    // not well defined if cos(r[1])==0:
	if (cos_r1 <= 0.0) {
		// if (m.r[4]>0.0) sin,cos(a-p)=m.r[ 9],m.r[10]
		// else sin,cos(a+p)=m.r[ 9],m.r[10]
		// so lets say p=0:
		r[2] = 0.0;
		r[0] = atan2(m.r[9],m.r[10]);
	} else {
		r[0] = atan2(-m.r[6],m.r[5]);
		r[2] = atan2( m.r[8],m.r[0]);
	}
	return r;
}

void ArtificialPlanet::computeAverage(float t) {

	float f1, f2;

	// TODO a smoothing function would be better
	if(t < 0.5) f2 = 2*.95*t;
	else if(t < 0.75) f2 = .95 + 4*.04*(t-.5);
	else if(t < .875 ) f2 = .99 + 8*.009*(t-.75);
	else if(t < .9375 ) f2 = .999 + 16*.0009*(t-.875);
	else f2 = .9999 + 16*.0001*(t-.9375);

	f1 = 1 - f2;
	//	printf("%.4f\t%.4f\n", t, f1);

	set_heliocentric_ecliptic_pos(startPos*f1
								  + dest->get_heliocentric_ecliptic_pos()*f2);

	// finish rotation and radius adjustments before final travel
	if(t < 0.9) f2 = t / 0.9;
	else f2 = 1.0;
	f1 = 1 - f2;

	radius = orig_radius * f1 + dest->getRadius() * f2;

	// 3 Euler angles:
	// TODO: Seems like rotates excessively...
	Vec3d a1 = orig_rotation;
	const Vec3d a2(getRot(dest));
	if (a1[0]-a2[0] >  M_PI) a1[0] -= 2.0*M_PI; else
		if (a1[0]-a2[0] < -M_PI) a1[0] += 2.0*M_PI;

	if (a1[2]-a2[2] >  M_PI) a1[2] -= 2.0*M_PI; else
		if (a1[2]-a2[2] < -M_PI) a1[2] += 2.0*M_PI;

	setRot(a1*f1 + a2*f2);

    // rotation offset
	re.offset = f1*re.offset + f2*dest->getRotationElements().offset;

}
