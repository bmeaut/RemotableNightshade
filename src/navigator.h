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

#ifndef _NAVIGATOR_H_
#define _NAVIGATOR_H_

#include "nightshade.h"
#include "observer.h"
#include "vecmath.h"
#include "projector.h"
#include "shared_data.h"

// Conversion in standar Julian time format
#define JD_SECOND 0.000011574074074074074074
#define JD_MINUTE 0.00069444444444444444444
#define JD_HOUR   0.041666666666666666666
#define JD_DAY    1.

extern const Mat4d mat_j2000_to_vsop87;
extern const Mat4d mat_vsop87_to_j2000;

class Object;

// Class which manages a navigation context
// Manage date/time, viewing direction/fov, observer position, and coordinate changes
class Navigator
{
public:

	enum VIEWING_MODE_TYPE {
		VIEW_HORIZON,
		VIEW_EQUATOR
	};
	// Create and initialise to default a navigation context
	Navigator(Observer* obs);
	virtual ~Navigator();

	// Init the viewing matrix, setting the field of view, the clipping planes, and screen size
	void init_project_matrix(int w, int h, double near, double far);

	void update_time(int delta_time);
	void update_transform_matrices(void);
	void update_vision_vector(int delta_time,const Object &selected);

	// Update the modelview matrices
	void update_model_view_mat(Projector *projector, double fov);

	// Move to the given position in equatorial or local coordinate depending on _local_pos value
	void move_to(const Vec3d& _aim, float move_duration = 1., bool _local_pos = false, int zooming = 0);

	// Loads
	void load_position(const string&);		// Load the position info in the file name given
	void save_position(const string&);		// Save the position info in the file name given

	// Time controls
	void set_JDay(double JD) {
		JDay=JD;
	}
	double get_JDay(void) const {
		return JDay;
	}
	void set_time_speed(double ts) {
		time_speed=ts;
	}
	double get_time_speed(void) const {
		return time_speed;
	}

	// Flags controls
	void set_flag_traking(int v) {
		flag_traking=v;
	}
	int get_flag_traking(void) const {
		return flag_traking;
	}
	void set_flag_lock_equ_pos(int v) {
		flag_lock_equ_pos=v;
	}
	int get_flag_lock_equ_pos(void) const {
		return flag_lock_equ_pos;
	}

	// Get vision direction
	const Vec3d& get_equ_vision(void) const {
		return equ_vision;
	}
	const Vec3d& get_prec_equ_vision(void) const {
		return prec_equ_vision;
	}
	const Vec3d& get_local_vision(void) const {
		return local_vision;
	}

	void set_local_vision(const Vec3d& _pos);

	const Planet *getHomePlanet(void) const {
		return position->getHomePlanet();
	}

	const double get_latitude(void) const {
		return position->get_latitude();
	}

	// Return the observer heliocentric position
	Vec3d get_observer_helio_pos(void) const;

	// Place openGL in earth equatorial coordinates
	void switch_to_earth_equatorial(void) const {
		glLoadMatrixd(mat_earth_equ_to_eye);
	}

	// Place openGL in heliocentric ecliptical coordinates
	void switch_to_heliocentric(void) const {
		glLoadMatrixd(mat_helio_to_eye);
	}

	// Place openGL in local viewer coordinates (Usually somewhere on earth viewing in a specific direction)
	void switch_to_local(void) const {
		glLoadMatrixd(mat_local_to_eye);
	}


	// Transform vector from local coordinate to equatorial
	Vec3d local_to_earth_equ(const Vec3d& v) const {
		return mat_local_to_earth_equ*v;
	}

	// Transform vector from equatorial coordinate to local
	Vec3d earth_equ_to_local(const Vec3d& v) const {
		return mat_earth_equ_to_local*v;
	}

	Vec3d earth_equ_to_j2000(const Vec3d& v) const {
		return mat_earth_equ_to_j2000*v;
	}
	Vec3d j2000_to_earth_equ(const Vec3d& v) const {
		return mat_j2000_to_earth_equ*v;
	}

	// Transform vector from heliocentric coordinate to local
	Vec3d helio_to_local(const Vec3d& v) const {
		return mat_helio_to_local*v;
	}

	// Transform vector from heliocentric coordinate to earth equatorial,
	// only needed in meteor.cpp
	Vec3d helio_to_earth_equ(const Vec3d& v) const {
		return mat_helio_to_earth_equ*v;
	}

	// Transform vector from heliocentric coordinate to false equatorial : equatorial
	// coordinate but centered on the observer position (usefull for objects close to earth)
	Vec3d helio_to_earth_pos_equ(const Vec3d& v) const {
		return mat_local_to_earth_equ*mat_helio_to_local*v;
	}


	// Return the modelview matrix for some coordinate systems
	const Mat4d& get_helio_to_eye_mat(void) const {
		return mat_helio_to_eye;
	}
	const Mat4d& get_earth_equ_to_eye_mat(void) const {
		return mat_earth_equ_to_eye;
	}
	const Mat4d& get_local_to_eye_mat(void) const {
		return mat_local_to_eye;
	}
	const Mat4d& get_j2000_to_eye_mat(void) const {
		return mat_j2000_to_eye;
	}

	const Mat4d& get_galactic_to_eye_mat(void) const {
		return mat_galactic_to_eye;
	}

	// Return fixed dome matrix (no heading adjustment)
	const Mat4d& get_dome_fixed_mat(void) const {
		return mat_dome_fixed;
	}
	// Return dome matrix adjusted for current heading
	const Mat4d& get_dome_mat(void) const {
		return mat_dome;
	}

	void update_move(Projector *projector, double deltaAz, double deltaAlt, double fov);

	void set_viewing_mode(VIEWING_MODE_TYPE view_mode);
	VIEWING_MODE_TYPE get_viewing_mode(void) const {
		return viewing_mode;
	}
	void switch_viewing_mode(void);

	// TODO - add error checking here?
	void set_heading(double _heading) {
		heading = _heading;
	}
	void set_defaultHeading(double _heading) {
		m_defaultHeading = _heading;
	}
	double get_defaultHeading() const {
		return m_defaultHeading;
	}
	double get_heading() const {
		double h = heading;

		// keep within -180 to 180 for TUI compatibility
		while (h > 180) h -= 360;
		while (h < -180) h += 360;

		return h;
	}

	// NB: always call stel_core method, not this one directly to set
	// so that init view vector gets updated correctly
	void set_view_offset(double _offset) {
		view_offset = _offset;
		SettingsState state;
		state.m_state.zoom_offset = _offset;
		SharedData::Instance()->Settings( state );
	}
	double get_view_offset() const {
		return view_offset;
	}

	// move gradually to a new heading
	void change_heading(double _heading, int duration);

	// for moving heading position gradually
	void update(int delta_time);


private:

	// Struct used to store data for auto move
	typedef struct {
		Vec3d start;
		Vec3d aim;
		float speed;
		float coef;
		bool local_pos;				// Define if the position are in equatorial or altazimutal
	} auto_move;

	float view_offset_transition;   // for transitioning to and from using a view offset

	// Matrices used for every coordinate transfo
	Mat4d mat_helio_to_local;		// Transform from Heliocentric to Observer local coordinate
	Mat4d mat_local_to_helio;		// Transform from Observer local coordinate to Heliocentric
	Mat4d mat_local_to_earth_equ;	// Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_earth_equ_to_local;	// Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_helio_to_earth_equ;	// Transform from Heliocentric to earth equatorial coordinate
	Mat4d mat_earth_equ_to_j2000;
	Mat4d mat_earth_equ_to_galactic;
	Mat4d mat_j2000_to_earth_equ;
	Mat4d mat_galactic_to_earth_equ;

	Mat4d mat_local_to_eye;			// Modelview matrix for observer local drawing
	Mat4d mat_earth_equ_to_eye;		// Modelview matrix for geocentric equatorial drawing
	Mat4d mat_j2000_to_eye;	// precessed version
	Mat4d mat_galactic_to_eye;	// precessed version
	Mat4d mat_helio_to_eye;			// Modelview matrix for heliocentric equatorial drawing

	Mat4d mat_dome_fixed; // Dome (fixed alt/az) modeliew matrix
	Mat4d mat_dome; // Dome (fixed alt/az) modeliew matrix with heading adjustment

	// Vision variables
	Vec3d local_vision, equ_vision, prec_equ_vision;	// Viewing direction in local and equatorial coordinates
	int flag_traking;				// Define if the selected object is followed
	int flag_lock_equ_pos;			// Define if the equatorial position is locked

	// Automove
	auto_move move;					// Current auto movement
	int flag_auto_move;				// Define if automove is on or off
	int zooming_mode;				// 0 : undefined, 1 zooming, -1 unzooming

	// Time variable
	double time_speed;				// Positive : forward, Negative : Backward, 1 = 1sec/sec
	double JDay;        			// Curent time in Julian day

	// Position variables
	Observer* position;

	VIEWING_MODE_TYPE viewing_mode;   // defines if view corrects for horizon, or uses equatorial coordinates

	double view_offset;              // To center/zoom away from the center of the viewport
	double heading;                  // Rotate the environment around the observer
	double m_defaultHeading;

	// for changing heading
	bool flag_change_heading;
	double start_heading, end_heading;
	float move_to_coef, move_to_mult;


	float last_ra_aim, last_ra_start;  // for better track above planet
};

#endif //_NAVIGATOR_H_
