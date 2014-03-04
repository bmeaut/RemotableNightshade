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

using namespace std;

#include <algorithm>
#include <iostream>
#include <string>
#include "solarsystem.h"
#include "s_texture.h"
#include "orbit.h"
#include "nightshade.h"
#include "draw.h"
#include "utility.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>


SolarSystem::SolarSystem()
	:sun(NULL),moon(NULL),earth(NULL),
	 moonScale(1.), planet_name_font(NULL), tex_earth_shadow(NULL),
	 flagOrbits(false),flag_light_travel_time(false),flagHints(false),flagTrails(false)
{
}

void SolarSystem::setFont(float font_size, const string& font_name)
{
	if (planet_name_font) delete planet_name_font;

	planet_name_font = new s_font(font_size, font_name);
	if (!planet_name_font) {
		printf("Can't create planet_name_font\n");
		exit(-1);
	}
	Planet::set_font(planet_name_font);
}

SolarSystem::~SolarSystem()
{
	// release selected:
	selected = Object();
	for (vector<Planet*>::iterator iter = system_planets.begin(); iter != system_planets.end(); ++iter) {
		if (*iter) delete *iter;
		*iter = NULL;
	}
	sun = NULL;
	moon = NULL;
	earth = NULL;

	if (planet_name_font) delete planet_name_font;
	if (tex_earth_shadow) delete tex_earth_shadow;
}


// Init and load the solar system data
void SolarSystem::load(const string& planetfile,  LoadingBar& lb)
{

	// Read ini file into a property tree
    using boost::property_tree::ptree;
    ptree solarSystemProperties;

	try {
		read_ini(planetfile, solarSystemProperties);
	}
	catch( std::exception &e ) {
		cerr << "FATAL ERROR : failed reading solar system definition! " << e.what() << endl;
		exit(1);
	}

	int bodies = 0;
	BOOST_FOREACH(ptree::value_type &v, solarSystemProperties)
		bodies++;

	int count = 0;
	BOOST_FOREACH(ptree::value_type &v, solarSystemProperties) {   
		count++;

		// Draw loading bar
		lb.SetMessage(_("Loading Solar System:") + Utility::intToString(count+1) +
			              "/" + Utility::intToString(bodies+1));
		lb.Draw((float)count/bodies);

		string name = v.first;
		//cout << "Body: " << name << endl;

		stringHash_t bodyParams;

		BOOST_FOREACH(ptree::value_type &b, v.second) {   
			bodyParams[b.first] = b.second.get<string>("");
			//cout << "\tpname: " << b.first << ", pvalue: " << bodyParams[b.first] << endl;
		}

		cout << addBody(bodyParams, false);  // config file bodies are not deletable
	}

	// special case: load earth shadow texture (DEPRECATED)
	tex_earth_shadow = new s_texture("earth-shadow.png", TEX_LOAD_TYPE_PNG_ALPHA);

	Planet::tex_eclipse_map = new s_texture("bodies/eclipse_map.png", TEX_LOAD_TYPE_PNG_SOLID);

	cout << "(solar system loaded)" << endl;

}

// Init and load one solar system object
// This is a the private method
string SolarSystem::addBody(stringHash_t & param, bool deletable)
{

	const string englishName = param["name"];
	const string str_parent = param["parent"];
	Planet *parent = NULL;

/*	cout << "Loading new Solar System object... " << englishName << endl;
	for ( stringHashIter_t iter = param.begin(); iter != param.end(); ++iter ) {
		cout << iter->first << " : " << iter->second << endl;
	}
*/

	// do not add if no name or parent
	if (englishName=="" || str_parent=="") {
		return("Can not add body with no name and/or no parent name");
	}

	// Do not add if body already exists - name must be unique
	if ( planetHash[englishName]!=NULL )
		return (string("Can not add body named \"") + englishName + string("\" because a body of that name already exists\n"));

	if (str_parent!="none") {
		parent = planetHash[str_parent];

		if (parent == NULL) {
			string error = string("WARNING : can't find parent for ") + englishName;
			return error;
		}
	}

	const string funcname = param["coord_func"];

	Orbit* orb = NULL;
	bool close_orbit = str_to_bool(param["close_orbit"], 1);

	// default value of -1 means unused
	double orbit_bounding_radius = str_to_double(param["orbit_bounding_radius"], -1);


	// when the parent is the sun use ecliptic rather than sun equator:
	const double parent_rot_obliquity = parent && parent->get_parent()
		? parent->getRotObliquity()
		: 0.0;
	const double parent_rot_asc_node = parent && parent->get_parent()
		? parent->getRotAscendingnode()
		: 0.0;
	double parent_rot_j2000_longitude = 0.0;
	if (parent && parent->get_parent()) {
		const double c_obl = cos(parent_rot_obliquity);
		const double s_obl = sin(parent_rot_obliquity);
		const double c_nod = cos(parent_rot_asc_node);
		const double s_nod = sin(parent_rot_asc_node);
		const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
		const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
		const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
		const Vec3d J2000Pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
		Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
		J2000NodeOrigin.normalize();
		parent_rot_j2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
	}

	// Comets have specific magnitude calculation and parameters
	double comet_absolute_magnitude = str_to_double(param["comet_absolute_magnitude"], 99);
	double comet_magnitude_slope = str_to_double(param["comet_magnitude_slope"], 4);


	if (funcname=="ell_orbit") {
		// Read the orbital elements
		double period = str_to_double(param["orbit_period"]);
		double epoch = str_to_double(param["orbit_epoch"],J2000);
		double semi_major_axis = str_to_double(param["orbit_semimajoraxis"])/AU;
		double eccentricity = str_to_double(param["orbit_eccentricity"]);
		double inclination = str_to_double(param["orbit_inclination"])*M_PI/180.;
		double ascending_node = str_to_double(param["orbit_ascendingnode"])*M_PI/180.;
		double long_of_pericenter = str_to_double(param["orbit_longofpericenter"])*M_PI/180.;
		double mean_longitude = str_to_double(param["orbit_meanlongitude"])*M_PI/180.;

		double arg_of_pericenter = long_of_pericenter - ascending_node;
		double anomaly_at_epoch = mean_longitude - (arg_of_pericenter + ascending_node);
		double pericenter_distance = semi_major_axis * (1.0 - eccentricity);

		// Create an elliptical orbit
		orb = new EllipticalOrbit(pericenter_distance,
		                          eccentricity,
		                          inclination,
		                          ascending_node,
		                          arg_of_pericenter,
		                          anomaly_at_epoch,
		                          period,
		                          epoch,
		                          parent_rot_obliquity,
		                          parent_rot_asc_node,
		                          parent_rot_j2000_longitude);

		orbit_bounding_radius = orb->getBoundingRadius();

	} else if (funcname=="comet_orbit") {

		// Read the orbital elements
		const double eccentricity = str_to_double(param["orbit_eccentricity"],0.0);
		if (eccentricity >= 1.0) close_orbit = false;
		double pericenter_distance = str_to_double(param["orbit_pericenterdistance"],-1e100);
		double semi_major_axis;
		if (pericenter_distance <= 0.0) {
			semi_major_axis = str_to_double(param["orbit_semimajoraxis"],-1e100);
			if (semi_major_axis <= -1e100) {
				return( string( "ERROR: " ) + englishName + string(": you must provide orbit_pericenterdistance or orbit_semimajoraxis"));
			} else {
				if (eccentricity == 1.0) return ("parabolic orbits have no semi_major_axis");
				pericenter_distance = semi_major_axis * (1.0-eccentricity);
			}
		} else {
			semi_major_axis = (eccentricity == 1.0)
			                  ? 0.0 // parabolic orbits have no semi_major_axis
			                  : pericenter_distance / (1.0-eccentricity);
		}
		double mean_motion = str_to_double(param["orbit_meanmotion"],-1e100);
		double period;
		if (mean_motion <= -1e100) {
			period = str_to_double(param["orbit_period"],-1e100);
			if (period <= -1e100) {
				if (parent->get_parent()) return ("ERROR: When the parent body is not the Sun\nyou must provide orbit_MeanMotion or orbit_Period");

				mean_motion = (eccentricity == 1.0)
				              ? 0.01720209895 * (1.5/pericenter_distance)
				              * sqrt(0.5/pericenter_distance)
				              : (semi_major_axis > 0.0)
				              ? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
				              : 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));

				if(eccentricity != 1) period = 2.0*M_PI/mean_motion;

			} else {
				mean_motion = 2.0*M_PI/period;
			}
		} else {
			mean_motion *= (M_PI/180.0);
		}
		double epoch = str_to_double(param["orbit_epoch"],-1e100);
		double time_at_pericenter = str_to_double(param["orbit_timeatpericenter"],-1e100);
		if (time_at_pericenter <= -1e100) {
			double mean_anomaly = str_to_double(param["orbit_meananomaly"],-1e100);
			if (epoch <= -1e100 || mean_anomaly <= -1e100) {
				return( string("ERROR: ") + englishName + string( ": when you do not provide orbit_TimeAtPericenter,\nyou must provide both orbit_Epoch and\norbit_MeanAnomaly"));
			} else {
				mean_anomaly *= (M_PI/180.0);
				time_at_pericenter = epoch - mean_anomaly / mean_motion;
			}
		} else {
			epoch = time_at_pericenter;
		}
		const double inclination = str_to_double(param["orbit_inclination"])*(M_PI/180.0);
		const double ascending_node = str_to_double(param["orbit_ascendingnode"])*(M_PI/180.0);
		const double arg_of_pericenter = str_to_double(param["orbit_argofpericenter"])*(M_PI/180.0);

		// If orbit eccentricity is not 1 (not parabolic orbit), use better elliptical orbit algorithms
		if(eccentricity != 1 ) {
			// Create an elliptical orbit
			orb = new EllipticalOrbit(pericenter_distance,
									  eccentricity,
									  inclination,
									  ascending_node,
									  arg_of_pericenter,
									  0,   // anomaly_at_epoch, which is perihelion
									  period,
									  epoch,
									  parent_rot_obliquity,
									  parent_rot_asc_node,
									  parent_rot_j2000_longitude);

			orbit_bounding_radius = orb->getBoundingRadius();

		} else {

			orb = new CometOrbit(pericenter_distance,
								 eccentricity,
								 inclination,
								 ascending_node,
								 arg_of_pericenter,
								 time_at_pericenter,
								 mean_motion,
								 parent_rot_obliquity,
								 parent_rot_asc_node,
								 parent_rot_j2000_longitude);
		}

	} else if (funcname=="earth_custom") {
		// Special case to take care of Earth-Moon Barycenter at a higher level than in ephemeris library

		cout << "Creating Earth orbit...\n" << endl;

		SpecialOrbit *sorb = new SpecialOrbit("emb_special");
		
		if (!sorb->isValid()) {
			string error = string("ERROR : can't find position function ") + funcname + string(" for ") + englishName + string("\n");
			cout << error << endl;
			return error;
		}

		// NB. moon has to be added later
		orb = new BinaryOrbit(sorb, 0.0121505677733761);

	} else if(funcname == "lunar_custom") {
		// This allows chaotic Moon ephemeris to be removed once start leaving acurate and sane range

		SpecialOrbit *sorb = new SpecialOrbit("lunar_special");
		
		if (!sorb->isValid()) {
			string error = string("ERROR : can't find position function ") + funcname + string(" for ") + englishName + string("\n");
			cout << error << endl;
			return error;
		}

		orb = new MixedOrbit(sorb,
							 str_to_double(param["orbit_period"]),
							 NShadeDateTime::JulianDayFromDateTime(-10000, 1, 1, 1, 1, 1),
							 NShadeDateTime::JulianDayFromDateTime(10000, 1, 1, 1, 1, 1),
							 EARTH_MASS + LUNAR_MASS,
							 0, 0, 0, 
							 false);
			
	} else {

		// *** Get special ephemeris 
		SpecialOrbit *sorb = new SpecialOrbit(funcname);

		if (!sorb->isValid()) {
			string error = string("ERROR : can't find position function ") + funcname + string(" for ") + englishName + string("\n");
			cout << error << endl;
			return error;
		}
		
		orb = sorb;
	}

	// Create the Planet and add it to the list
	Planet* p = new Planet(parent,
	                       englishName,
	                       str_to_bool(param["halo"]),
	                       str_to_bool(param["lighting"]),
	                       str_to_double(param["radius"])/AU,
	                       str_to_double(param["oblateness"], 0.0),
	                       Utility::str_to_vec3f(param["color"]),
	                       str_to_double(param["albedo"]),
	                       param["path"], // new
	                       param["tex_map"],
	                       param["tex_halo"],
						   orb,
	                       close_orbit,
	                       str_to_bool(param["hidden"], 0),
	                       deletable,
	                       str_to_bool(param["flat_texture"]),
	                       orbit_bounding_radius,
						   param["tex_normal"],
						   param["tex_night"],
						   param["tex_specular"],
						   param["tex_cloud"],
						   param["tex_cloud_normal"],
						   comet_absolute_magnitude,
						   comet_magnitude_slope
	                      );

	if(param["start_jday"] != "" || param["end_jday"] != "" )
		p->setValidityPeriod( str_to_double(param["start_jday"]), str_to_double(param["end_jday"]) );

	// These shortcuts are set first time through (should be defined in ssystem.ini)
	if(!sun && englishName=="Sun") sun = p;
	if(!earth && englishName=="Earth") earth = p;
	if(!moon && englishName=="Moon") {
		moon = p;
		
		// Update Earth binary orbit with the moon (if applicable)!
		if(earth) {
			BinaryOrbit *earthOrbit = dynamic_cast<BinaryOrbit *>(earth->getOrbit());
			if(earthOrbit) {
				cout << "Adding Moon to Earth binary orbit." << endl;
				earthOrbit->setSecondaryOrbit(orb);
			} else cout << "\nWARNING: " << englishName << " body could not be added to Earth orbit." << endl;
		} else cout << "\nWARNING: " << englishName << " body could not be added to Earth orbit calculation, position may be inacurate.\n" << endl;
	}
	
	// set local colors if need be
	Vec3f orbit_color = Utility::str_to_vec3f(param["orbit_color"]);
	if (orbit_color[0] != 0 || orbit_color[1] != 0 || orbit_color[2] != 0) {
		p->setLocalOrbitColor( Vec4f(orbit_color[0], orbit_color[1], orbit_color[2], 1));
	}

	// Use J2000 N pole data if available
	double rot_obliquity = str_to_double(param["rot_obliquity"],0.)*M_PI/180.;
	double rot_asc_node  = str_to_double(param["rot_equator_ascending_node"],0.)*M_PI/180.;

	// In J2000 coordinates
	double J2000_npole_ra = str_to_double(param["rot_pole_ra"],0.)*M_PI/180.;
	double J2000_npole_de = str_to_double(param["rot_pole_de"],0.)*M_PI/180.;

	// NB: north pole needs to be defined by right hand rotation rule
	if (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "") {
		// cout << "Using north pole data for " << englishName << endl;

		Vec3d J2000_npole;
		sphe_to_rect(J2000_npole_ra,J2000_npole_de,J2000_npole);

		Vec3d vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));

		double ra, de;
		rect_to_sphe(&ra, &de, vsop87_pole);

		rot_obliquity = (M_PI_2 - de);
		rot_asc_node = (ra + M_PI_2);

		//	  cout << "\tCalculated rotational obliquity: " << rot_obliquity*180./M_PI << endl;
		//cout << "\tCalculated rotational ascending node: " << rot_asc_node*180./M_PI << endl;
	}

	p->set_rotation_elements(
	    str_to_double(param["rot_periode"], str_to_double(param["orbit_period"], 24.))/24.,
	    str_to_double(param["rot_rotation_offset"],0.),
	    str_to_double(param["rot_epoch"], J2000),
	    rot_obliquity,
	    rot_asc_node,
	    str_to_double(param["rot_precession_rate"],0.)*M_PI/(180*36525),
	    str_to_double(param["orbit_visualization_period"],0.),
	    str_to_double(param["axial_tilt"],0.) );

	if (str_to_bool(param["rings"], 0)) {
		const double r_min = str_to_double(param["ring_inner_size"])/AU;
		const double r_max = str_to_double(param["ring_outer_size"])/AU;
		Ring *r = new Ring(r_min,r_max,param["tex_ring"],param["path"]);
		p->set_rings(r);
	}

	if (str_to_bool(param["ring_shadow"], 0)) {
		p->set_ring_shadow(true);
	}

	string bighalotexfile = param["tex_big_halo"];
	if (!bighalotexfile.empty()) {
		p->set_big_halo(bighalotexfile, param["path"]);
		p->set_halo_size(str_to_double(param["big_halo_size"], 50.f));
	}

	// Clone current flags to new body unless one is currently selected
	p->setFlagHints(getFlagHints());
	p->setFlagTrail(getFlagTrails());

	if (!selected || selected == Object(sun)) {
		p->setFlagOrbit(getFlagOrbits());
	}

	system_planets.push_back(p);

	planetHash[englishName] = p;

	p->updateBoundingRadii();

	return("");  // OK

}

// Remove solar system object DIGITALIS
// If not from ini file
//string SolarSystem::removeBody(string name, string parent) {
string SolarSystem::removeBody(string name)
{

	string result = string("Could not find body to delete: " + name);

	Planet *planet = planetHash[name];
	if (planet == NULL) return result;

	//  cout << "Trying to delete " << name << endl;
	vector<Planet*>::iterator iter;

	if (!planet->isDeleteable() ) return ("Can not delete a body loaded from config file.");

	if(planet->hasSatellite()) return ("Can not delete body because it has satellites.\n");


	for (iter=system_planets.begin(); iter!=system_planets.end(); iter++) {

		if ((*iter)->getEnglishName()==name) {

			if ((*iter)->getParent()) {
				// remove from parent satellite list
				(*iter)->getParent()->removeSatellite((*iter));

				// update bounding radii without this planet
				(*iter)->getParent()->updateBoundingRadii();
			}

			// delete planet
			if (*iter) delete *iter;
			system_planets.erase(iter);
			planetHash[name] = NULL;

			result = "";  // deletion worked
			break;
		}
	}
	return result;
}


// Remove all solar system objects not from ini file DIGITALIS
// requires that HOME PLANET NOT BE A DELETABLE PLANET
// or will return error
string SolarSystem::removeSupplementalBodies(const Planet *home_planet)
{

	string name;

	if (home_planet->isDeleteable())
		return "Can not remove supplemental bodies because current home planet is also one.";

	bool deleting = true;

	// Loop removing deletable planets from the lowest children on up the tree
	while ( deleting ) {

		deleting = false;

		vector<Planet*>::iterator iter = system_planets.begin();
		while (iter != system_planets.end()) {

			if ((*iter)->isDeleteable() && !(*iter)->hasSatellite()) {

				deleting = true;

				name = (*iter)->getEnglishName();

				if ((*iter)->getParent()) (*iter)->getParent()->removeSatellite((*iter));

				// delete planet
				delete *iter;
				iter = system_planets.erase(iter);
				planetHash[name] = NULL;

			} else {
				iter++;
			}
		}
	}

	// Update bounding radii (not very efficient)
	for (vector<Planet*>::iterator iter = system_planets.begin(); iter != system_planets.end(); ++iter) {
		(*iter)->updateBoundingRadii();
	}


	/*
	// DEBUG
	for(vector<Planet*>::iterator iter = system_planets.begin(); iter != system_planets.end(); ++iter) {
	cout << "planet " << (*iter)->getEnglishName() << " remains\n";
	}
	// END DEBUG
	*/

	return "";
}

// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystem::computePositions(double date,const Planet *home_planet)
{
	if (flag_light_travel_time) {
		for (vector<Planet*>::const_iterator iter(system_planets.begin());
		        iter!=system_planets.end(); iter++) {
			(*iter)->computePositionWithoutOrbits(date);
		}
		const Vec3d home_pos(home_planet->get_heliocentric_ecliptic_pos());
		for (vector<Planet*>::const_iterator iter(system_planets.begin());
		        iter!=system_planets.end(); iter++) {
			const double light_speed_correction =
			    ((*iter)->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			//cout << "SolarSystem::computePositions: " << (*iter)->getEnglishName()
			//     << ": " << (86400*light_speed_correction) << endl;
			(*iter)->compute_position(date-light_speed_correction);
		}
	} else {
		for (vector<Planet*>::const_iterator iter(system_planets.begin());
		        iter!=system_planets.end(); iter++) {
			(*iter)->compute_position(date);
		}
	}

	computeTransMatrices(date, home_planet);

}

// Compute the transformation matrix for every elements of the solar system.
// The elements have to be ordered hierarchically, eg. it's important to compute earth before moon.
void SolarSystem::computeTransMatrices(double date,const Planet *home_planet)
{
	if (flag_light_travel_time) {
		const Vec3d home_pos(home_planet->get_heliocentric_ecliptic_pos());
		for (vector<Planet*>::const_iterator iter(system_planets.begin());
		        iter!=system_planets.end(); iter++) {
			const double light_speed_correction =
			    ((*iter)->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			(*iter)->compute_trans_matrix(date-light_speed_correction);
		}
	} else {
		for (vector<Planet*>::const_iterator iter(system_planets.begin());
		        iter!=system_planets.end(); iter++) {
			(*iter)->compute_trans_matrix(date);
		}
	}
}


struct depthBucket {
	double znear;
	double zfar;
};

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
double SolarSystem::draw(Projector * prj, const Navigator * nav, const ToneReproductor* eye, bool flag_point, bool drawHomePlanet)
{
	if (!getFlagPlanets()) return 0;

	// Set the light parameters taking sun as the light source
	const float zero[4] = {0,0,0,0};
	const float ambient[4] = {0.03,0.03,0.03,0.03};
	const float diffuse[4] = {1,1,1,1};

	const float mdiffuse[4] = {5,5,5,1};

	glLightfv(GL_LIGHT0,GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,zero);

	glMaterialfv(GL_FRONT,GL_AMBIENT,  ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,  diffuse);
	glMaterialfv(GL_FRONT,GL_EMISSION, zero);
	glMaterialfv(GL_FRONT,GL_SHININESS,zero);
	glMaterialfv(GL_FRONT,GL_SPECULAR, zero);

	// Light pos in zero (sun)
	nav->switch_to_heliocentric();
	glLightfv(GL_LIGHT0,GL_POSITION,Vec4f(0.f,0.f,0.f,1.f));
	glEnable(GL_LIGHT0);

	// Compute each Planet distance to the observer
	Vec3d obs_helio_pos = nav->get_observer_helio_pos();

//	cout << "obs: " << obs_helio_pos << endl;

	vector<Planet*>::iterator iter;
	for (iter = system_planets.begin(); iter != system_planets.end(); iter++) {
		(*iter)->compute_distance(obs_helio_pos);
		(*iter)->compute_magnitude(obs_helio_pos);
	}

	// And sort them from the furthest to the closest
	sort(system_planets.begin(),system_planets.end(),bigger_distance());

	// Determine optimal depth buffer buckets for drawing the scene
	// This is similar to Celestia, but instead of using ranges within one depth
	// buffer we just clear and reuse the entire depth buffer for each bucket.

	// TODO should have planets in a tree and or hash structure

	double znear, zfar;
	double lastNear = 0;
	double lastFar = 0;
	list<depthBucket>listBuckets;
	int nBuckets = 0;
	depthBucket db;

	for (iter = system_planets.begin(); iter != system_planets.end(); iter++) {
		if ( (*iter)->get_parent() == sun

			 // This will only work with natural planets
			 // and not some illustrative (huge) artificial planets for example
		     // && (!(*iter)->isSimplePlanet() || *iter == home_planet)
			 && (*iter)->get_on_screen_bounding_size(prj, nav) > 3 ) {

			// non-satellite, needs a depth buffer, potentially visible

			// if( (*iter)->get_parent() == sun && (*iter)->testVisibility(prj, nav) ) {

			double dist = (*iter)->get_earth_equ_pos(nav).length();  // AU

			double bounding = (*iter)->getBoundingRadius() * 1.01;

			if ( bounding >= 0 ) {
				// this is not a hidden object

				znear = dist - bounding;
				zfar  = dist + bounding;

				if (znear < 0.001) znear = 0.0000001;
				else if (znear < 0.05) znear *= 0.1;
				else if (znear < 0.5) znear *= 0.2;


				// see if overlaps previous bucket
				// TODO check that buffer isn't too deep
				if ( nBuckets > 0 && zfar > lastNear ) {
					// merge with last bucket

					//cout << "merged buckets " << (*iter)->getEnglishName() << " " << znear << " " << zfar << " with " << lastNear << " " << lastFar << endl;
					db = listBuckets.back();

					if(znear < lastNear ) {
						// Artificial planets may cover real planets, for example
						lastNear = db.znear = znear;
					}

					if ( zfar > lastFar ) {
						lastFar = db.zfar = zfar;
					}

					listBuckets.pop_back();
					listBuckets.push_back(db);

				} else {

					// create a new bucket
					//cout << "New bucket: " << (*iter)->getEnglishName() << znear << " zfar: " << zfar << endl;
					lastNear = db.znear = znear;
					lastFar  = db.zfar  = zfar;
					nBuckets++;
					listBuckets.push_back( db );

				}

			}

		}

	}

	list<depthBucket>::iterator dbiter;

	/*
	  cout << "***\n";
	  dbiter = listBuckets.begin();
	  while( dbiter != listBuckets.end() ) {
	  cout << (*dbiter).znear << " " << (*dbiter).zfar << endl;
	  dbiter++;
	  }
	  cout << "***\n";
	*/

	// Draw the elements
	double z_near, z_far;
	prj->get_clipping_planes(&z_near,&z_far); // Save clipping planes

	dbiter = listBuckets.begin();

	// clear depth buffer
	prj->set_clipping_planes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
	glClear(GL_DEPTH_BUFFER_BIT);

	//float depthRange = 1.0f/nBuckets;
	float currentBucket = nBuckets - 1;

	// economize performance by not clearing depth buffer for each bucket... good?
	// glDepthRange(currentBucket*depthRange, (currentBucket+1)*depthRange);

	//	cout << "\n\nNew depth rendering loop\n";

	bool depthTest = true;  // small objects don't use depth test for efficiency
	double dist;
	double maxSquaredDistance = 0;
	for (iter = system_planets.begin(); iter != system_planets.end(); iter++) {


		//cout << "drawing " << (*iter)->getEnglishName() << " distance: " << (*iter)->get_earth_equ_pos(nav).length() << "  znear: " << (*dbiter).znear << " zfar: " << (*dbiter).zfar << endl;

		dist = (*iter)->get_earth_equ_pos(nav).length();
		if (dist < (*dbiter).znear ) {

			// potentially use the next depth bucket
			dbiter++;

			if (dbiter == listBuckets.end() ) {
				dbiter--;
				// now closer than the first depth buffer
			} else {
				currentBucket--;

				// TODO: evaluate performance tradeoff???
				// glDepthRange(currentBucket*depthRange, (currentBucket+1)*depthRange);
				glClear(GL_DEPTH_BUFFER_BIT);

				// get ready to start using
				prj->set_clipping_planes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
			}
		}

		if (dist > (*dbiter).zfar || dist < (*dbiter).znear) {
			// don't use depth test (outside buckets)

			if ( depthTest ) prj->set_clipping_planes(z_near, z_far);
			depthTest = false;

		} else {
			if (!depthTest) prj->set_clipping_planes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
			depthTest = true;
		}


		// - make moon more realistic
		if (*iter==moon)	glLightfv(GL_LIGHT0,GL_DIFFUSE, mdiffuse);
		else glLightfv(GL_LIGHT0,GL_DIFFUSE, diffuse);

		double squaredDistance = 0;
		if (*iter==moon && near_lunar_eclipse(nav, prj)) {

			// TODO: moon magnitude label during eclipse isn't accurate...

			// special case to update stencil buffer for drawing lunar eclipses
			glClear(GL_STENCIL_BUFFER_BIT);
			glClearStencil(0x0);

			glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);

			squaredDistance = (*iter)->draw(prj, nav, eye, flag_point, 1, depthTest, drawHomePlanet, selected == (*iter));

		} else {
			squaredDistance = (*iter)->draw(prj, nav, eye, flag_point, 0, depthTest, drawHomePlanet, selected == (*iter));
		}
		if (squaredDistance > maxSquaredDistance)
			maxSquaredDistance = squaredDistance;


	}

	prj->set_clipping_planes(z_near,z_far);  // Restore old clipping planes
	//  glDepthRange(0,1);

	glDisable(GL_LIGHT0);

	// special case: draw earth shadow over moon if appropriate
	// stencil buffer is set up in moon drawing above
	// This effect curently only looks right from earth viewpoint

//	if (nav->getHomePlanet() == getEarth())
//		draw_earth_shadow(nav, prj);

	return maxSquaredDistance;
}

Planet* SolarSystem::searchByEnglishName(string planetEnglishName) const
{
	//printf("SolarSystem::searchByEnglishName(\"%s\"): start\n",
	//       planetEnglishName.c_str());
	// side effect - bad?
	//	transform(planetEnglishName.begin(), planetEnglishName.end(), planetEnglishName.begin(), ::tolower);

	vector<Planet*>::const_iterator iter = system_planets.begin();
	while (iter != system_planets.end()) {
		//printf("SolarSystem::searchByEnglishName(\"%s\"): %s\n",
		//       planetEnglishName.c_str(),
		//       (*iter)->getEnglishName().c_str());
		if ( (*iter)->getEnglishName() == planetEnglishName ) return (*iter); // also check standard ini file names
		++iter;
	}
	//printf("SolarSystem::searchByEnglishName(\"%s\"): not found\n",
	//       planetEnglishName.c_str());
	return NULL;
}

Object SolarSystem::searchByNamesI18(string planetNameI18) const
{

	// side effect - bad?
	//	transform(planetNameI18.begin(), planetNameI18.end(), planetNameI18.begin(), ::tolower);

	vector<Planet*>::const_iterator iter = system_planets.begin();
	while (iter != system_planets.end()) {

		if ( (*iter)->getNameI18n() == planetNameI18 ) return (*iter); // also check standard ini file names
		++iter;
	}
	return NULL;
}

// Search if any Planet is close to position given in earth equatorial position and return the distance
Object SolarSystem::search(Vec3d pos, const Navigator * nav,
                           const Projector * prj) const
{
	pos.normalize();
	Planet * closest = NULL;
	double cos_angle_closest = 0.;
	static Vec3d equPos;

	vector<Planet*>::const_iterator iter = system_planets.begin();
	while (iter != system_planets.end()) {
		equPos = (*iter)->get_earth_equ_pos(nav);
		equPos.normalize();
		double cos_ang_dist = equPos[0]*pos[0] + equPos[1]*pos[1] + equPos[2]*pos[2];
		if (cos_ang_dist>cos_angle_closest) {
			closest = *iter;
			cos_angle_closest = cos_ang_dist;
		}

		iter++;
	}

	if (cos_angle_closest>0.999) {
		return closest;
	} else return NULL;
}

// Return a stl vector containing the planets located inside the lim_fov circle around position v
vector<Object> SolarSystem::search_around(Vec3d v,
        double lim_fov,
        const Navigator * nav,
        const Projector * prj,
        bool *default_last_item,
        bool aboveHomePlanet ) const
{
	vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * M_PI/180.);
	static Vec3d equPos;
	const Planet *home_planet = nav->getHomePlanet();

	*default_last_item = false;

	// Should still be sorted by distance from farthest to closest
	// So work backwards to go closest to furthest

	vector<Planet*>::const_iterator iter = system_planets.end();
	while (iter != system_planets.begin()) {
		iter--;

		equPos = (*iter)->get_earth_equ_pos(nav);
		equPos.normalize();

		// First see if within a planet disk
		if ((*iter) != home_planet || aboveHomePlanet) {
			// Don't want home planet too easy to select unless can see it

			double angle = acos(v*equPos) * 180.f / M_PI;

			/*
			  cout << "Big testing " << (*iter)->getEnglishName()
			  << " angle: " << angle << " screen_angle: "
			  << (*iter)->get_angular_size(prj, nav)/2.f
			  << endl;
			*/

			if ( angle < (*iter)->get_angular_size(prj, nav)/2.f ) {

				// If near planet, may be huge but hard to select, so check size

				result.push_back(*iter);

				*default_last_item = true;

				break;  // do not want any planets behind this one!

			}
		}

		// See if within area of interest
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {
			result.push_back(*iter);

		}

	}
	return result;
}

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void SolarSystem::translateNames(Translator& trans)
{
	vector<Planet*>::iterator iter;
	for ( iter = system_planets.begin(); iter < system_planets.end(); iter++ ) {
		(*iter)->translateName(trans);
	}

	if(planet_name_font) planet_name_font->clearCache();
}

vector<string> SolarSystem::getNamesI18(void)
{
	vector<string> names;
	vector < Planet * >::iterator iter;

	for (iter = system_planets.begin(); iter != system_planets.end(); ++iter)
		names.push_back((*iter)->getNameI18n());
	return names;
}


// returns a newline delimited hash of localized:standard planet names for tui
// Planet translated name is PARENT : NAME
string SolarSystem::getPlanetHashString(void)
{
	ostringstream oss;
	vector < Planet * >::iterator iter;

	for (iter = system_planets.begin(); iter != system_planets.end(); ++iter) {
		if (!(*iter)->isDeleteable() ) { // no supplemental bodies in list
			if ((*iter)->get_parent() != NULL && (*iter)->get_parent()->getEnglishName() != "Sun") {
				oss << Translator::globalTranslator.translateUTF8((*iter)->get_parent()->getEnglishName())
				<< " : ";
			}

			oss << Translator::globalTranslator.translateUTF8((*iter)->getEnglishName()) << "\n";
			oss << (*iter)->getEnglishName() << "\n";
		}
	}

	// wcout <<  oss.str();

	return oss.str();

}


void SolarSystem::updateTrails(const Navigator* nav)
{
	vector<Planet*>::iterator iter;
	for ( iter = system_planets.begin(); iter < system_planets.end(); iter++ ) {
		(*iter)->update_trail(nav);
	}
}

void SolarSystem::startTrails(bool b)
{
	vector<Planet*>::iterator iter;
	for ( iter = system_planets.begin(); iter < system_planets.end(); iter++ ) {
		(*iter)->startTrail(b);
	}
}

void SolarSystem::setFlagTrails(bool b)
{
	flagTrails = b;
	ObjectsState state;
	state.m_state.planet_trails = b;
	SharedData::Instance()->Objects(state);

	if (!b || !selected || selected == Object(sun)) {
		vector<Planet*>::iterator iter;
		for( iter = system_planets.begin(); iter < system_planets.end(); iter++ )
			(*iter)->setFlagTrail(b);
	} else {
		// if a Planet is selected and trails are on,
		// fade out non-selected ones
		
		vector<Planet*>::iterator iter;
		for (iter = system_planets.begin();
			 iter != system_planets.end(); iter++ ) {
			if (selected == (*iter) || 
				((*iter)->get_parent() && (*iter)->get_parent()->getEnglishName() == selected.getEnglishName()) )
				(*iter)->setFlagTrail(b);
			else (*iter)->setFlagTrail(false);
		}
	}
}

bool SolarSystem::getFlagTrails(void) const
{

	return flagTrails;

	/*
	  for (vector<Planet*>::const_iterator iter = system_planets.begin();
	  iter != system_planets.end(); iter++ ) {
	  if ((*iter)->getFlagTrail()) return true;
	  }
	  return false;
	*/

}

void SolarSystem::setFlagHints(bool b)
{
	flagHints = b;
	ObjectsState state;
	state.m_state.planet_labels = b;
	SharedData::Instance()->Objects(state);

	// TODO just use planet static variable

	vector<Planet*>::iterator iter;
	for ( iter = system_planets.begin(); iter < system_planets.end(); iter++ ) {
		(*iter)->setFlagHints(b);
	}
}


bool SolarSystem::getFlagHints(void) const
{
	return flagHints;

	/*
	  for (vector<Planet*>::const_iterator iter = system_planets.begin();
	  iter != system_planets.end(); iter++ ) {
	  if ((*iter)->getFlagHints()) return true;
	  }
	  return false;
	*/

}


void SolarSystem::setFlagOrbits(bool b)
{
	flagOrbits = b;
	ObjectsState state;
	state.m_state.planet_orbits = b;
	SharedData::Instance()->Objects(state);

	if (!b || !selected || selected == Object(sun)) {
		vector<Planet*>::iterator iter;
		for ( iter = system_planets.begin(); iter < system_planets.end(); iter++ ) {
			(*iter)->setFlagOrbit(b);
		}
	} else {
		// if a Planet is selected and orbits are on,
		// fade out non-selected ones
		// unless they are orbiting the selected planet 20080612 DIGITALIS
		vector<Planet*>::iterator iter;
		for (iter = system_planets.begin();
		        iter != system_planets.end(); iter++ ) {
			//            if (selected == (*iter)) (*iter)->setFlagOrbit(b);
			if (selected == (*iter) ||
			        ((*iter)->get_parent() && (*iter)->get_parent()->getEnglishName() == selected.getEnglishName()) )
				(*iter)->setFlagOrbit(b);

			else (*iter)->setFlagOrbit(false);
		}
	}
}


void SolarSystem::setSelected(const Object &obj)
{
	if (obj.get_type() == ObjectRecord::OBJECT_PLANET) selected = obj;
	else selected = Object();
	// Undraw other objects hints, orbit, trails etc..
	//  setFlagHints(getFlagHints()); 20080612
	setFlagOrbits(getFlagOrbits());
	setFlagTrails(getFlagTrails());  // TODO should just hide trail display and not affect data collection
}

// draws earth shadow overlapping the moon using stencil buffer
// umbra and penumbra are sized separately for accuracy
void SolarSystem::draw_earth_shadow(const Navigator * nav, Projector * prj)
{

	Vec3d e = getEarth()->get_ecliptic_pos();
	Vec3d m = getMoon()->get_ecliptic_pos();  // relative to earth
	Vec3d mh = getMoon()->get_heliocentric_ecliptic_pos();  // relative to sun
	float mscale = getMoon()->get_sphere_scale();

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(1,1,1);

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;
	double r_umbra = 6378.1/AU - m.length()*(689621.9/AU/e.length());

	// find vector orthogonal to sun-earth vector using cross product with
	// a non-parallel vector
	Vec3d rpt = shadow^Vec3d(0,0,1);
	rpt.normalize();
	Vec3d upt = rpt*r_umbra*mscale*1.02;  // point on umbra edge
	rpt *= r_penumbra*mscale;  // point on penumbra edge

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if (mdist.length() > r_penumbra + 2000/AU) return;  // not visible so don't bother drawing

	shadow = mh + mdist*mscale;
	r_penumbra *= mscale;

	nav->switch_to_heliocentric();
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	Mat4d mat = nav->get_helio_to_eye_mat();

	// shadow radial texture
	glBindTexture(GL_TEXTURE_2D, tex_earth_shadow->getID());

	Vec3d r, s;

	// umbra first
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0,0);
	prj->sVertex3( shadow[0],shadow[1], shadow[2], mat);

	for (int i=0; i<=100; i++) {
		r = Mat4d::rotation(shadow, 2*M_PI*i/100.) * upt;
		s = shadow + r;

		glTexCoord2f(0.6,0);  // position in texture of umbra edge
		prj->sVertex3( s[0],s[1], s[2], mat);
	}
	glEnd();


	// now penumbra
	Vec3d u, sp;
	glBegin(GL_TRIANGLE_STRIP);
	for (int i=0; i<=100; i++) {
		r = Mat4d::rotation(shadow, 2*M_PI*i/100.) * rpt;
		u = Mat4d::rotation(shadow, 2*M_PI*i/100.) * upt;
		s = shadow + r;
		sp = shadow + u;

		glTexCoord2f(0.6,0);
		prj->sVertex3( sp[0],sp[1], sp[2], mat);

		glTexCoord2f(1.,0);  // position in texture of umbra edge
		prj->sVertex3( s[0],s[1], s[2], mat);
	}
	glEnd();

	glDisable(GL_STENCIL_TEST);

}


void SolarSystem::update(int delta_time, Navigator* nav)
{
	vector<Planet*>::iterator iter = system_planets.begin();

	double JD = nav->get_JDay();
	while (iter != system_planets.end()) {
		(*iter)->update_trail(nav);
		(*iter)->update(delta_time, JD);
		iter++;
	}

}


// is a lunar eclipse close at hand?
bool SolarSystem::near_lunar_eclipse(const Navigator * nav, Projector *prj)
{
	// TODO: could replace with simpler test

	Vec3d e = getEarth()->get_ecliptic_pos();
	Vec3d m = getMoon()->get_ecliptic_pos();  // relative to earth
	Vec3d mh = getMoon()->get_heliocentric_ecliptic_pos();  // relative to sun

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if (mdist.length() > r_penumbra + 2000/AU) return 0;  // not visible so don't bother drawing

	return 1;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
vector<string> SolarSystem::listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	vector < Planet * >::const_iterator iter;
	for (iter = system_planets.begin(); iter != system_planets.end(); ++iter) {
		string constw = (*iter)->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}
