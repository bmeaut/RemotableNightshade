/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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

#include "solarsystem.h"
#include "navigator.h"
#include "nightshade.h"
#include "utility.h"
#include "object.h"


////////////////////////////////////////////////////////////////////////////////
Navigator::Navigator(Observer* obs) : view_offset_transition(0), flag_traking(0),
		flag_lock_equ_pos(0), flag_auto_move(0), time_speed(JD_SECOND), 
      JDay(0.), position(obs), view_offset(0), heading(0.), flag_change_heading(0), 
      start_heading(0), end_heading(0), move_to_mult(0), last_ra_aim(0.), 
      last_ra_start(0.)
{
	if (!position) {
		printf("ERROR : Can't create a Navigator without a valid Observer\n");
		exit(-1);
	}
	local_vision=Vec3d(1.,0.,0.);
	equ_vision=Vec3d(1.,0.,0.);
	prec_equ_vision=Vec3d(1.,0.,0.);  // not correct yet...
	viewing_mode = VIEW_HORIZON;  // default

}

Navigator::~Navigator() {}

////////////////////////////////////////////////////////////////////////////////
void Navigator::update_vision_vector(int delta_time,const Object &selected)
{
	if (flag_auto_move) {
		double ra_aim, de_aim, ra_start, de_start, ra_now, de_now;

		if ( zooming_mode != -1 && selected) {
			// if zooming in, object may be moving so be sure to zoom to latest position
			move.aim = selected.get_earth_equ_pos(this);
			move.aim.normalize();
		}

		// Use a smooth function
		float smooth = 4.f;
		double c;

		if (zooming_mode == 1) {
			if ( move.coef > .9 ) c = 1;
			else c = 1 - pow(1.-1.11*(move.coef),3);

		} else if (zooming_mode == -1) {

			c =  move.coef*move.coef*move.coef*move.coef;

			// keep in view at first as zoom out

			/* could track as moves too, but would need to know if start was actually
			   a zoomed in view on the object or an extraneous zoom out command
			   if(move.local_pos) {
			   move.start=earth_equ_to_local(selected.get_earth_equ_pos(this));
			   } else {
			   move.start=selected.get_earth_equ_pos(this);
			   }
			   move.start.normalize();
			*/

		} else c = atanf(smooth * 2.*move.coef-smooth)/atanf(smooth)/2+0.5;

// view offset
		if (zooming_mode != -1) {
			if (view_offset_transition < 1 ) view_offset_transition = c;
		} else {
			if (view_offset_transition > 0 ) view_offset_transition = 1 - c;
		}

		// Rewrote look transistion code, 201104
		Vec3d v1, v2, v3, v4;

		if (move.local_pos) {
			v1 = move.start;
			v2 = move.aim;
		} else {
			v1 = earth_equ_to_local(move.start);
			v2 = earth_equ_to_local(move.aim);
		}

		// just in case is not already normalized
		v1.normalize();
		v2.normalize();

		// prevent one situation where doesn't work
		if(v1 == v2 ) v1 = Mat4d::xrotation(0.0001) * v1;

		v4 = v1^v2;
		v3 = v4^v1;  // in plane of v1 and v2 and orthogonal to v1
		v3.normalize();
		
		double angle = atan2( v4.length(), v1.dot(v2) );

		local_vision = (v1*cos(angle*c) + v3*sin(angle*c)); 
		equ_vision = local_to_earth_equ(local_vision);

		
		if (move.coef>=1.) {
			flag_auto_move=0;
			if (move.local_pos) {
				local_vision=move.aim;
				equ_vision=local_to_earth_equ(local_vision);
			} else {
				equ_vision=move.aim;
				local_vision=earth_equ_to_local(equ_vision);
			}
		}

// Allow full transition to occur
		move.coef+=move.speed*delta_time;
		if (move.coef>1) move.coef = 1;
	} else {
		if (flag_traking && selected) { // Equatorial vision vector locked on selected object
			equ_vision= selected.get_earth_equ_pos(this);
			local_vision=earth_equ_to_local(equ_vision);
		} else {
			if (flag_lock_equ_pos) { // Equatorial vision vector locked
				// Recalc local vision vector
				local_vision=earth_equ_to_local(equ_vision);
			} else { // Local vision vector locked
				// Recalc equatorial vision vector
				equ_vision=local_to_earth_equ(local_vision);
			}
		}
	}

	prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;

}

////////////////////////////////////////////////////////////////////////////////
void Navigator::set_local_vision(const Vec3d& _pos)
{

// Transition vision vector by view offset as needed
	local_vision = Mat4d::yrotation(-view_offset * M_PI_2 * view_offset_transition) * _pos;

//	local_vision = _pos;
	equ_vision=local_to_earth_equ(local_vision);
	prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;
}

////////////////////////////////////////////////////////////////////////////////
void Navigator::update_move(Projector *projector, double deltaAz, double deltaAlt, double fov)
{
	double azVision, altVision;

	if ( viewing_mode == VIEW_EQUATOR) rect_to_sphe(&azVision,&altVision,equ_vision);
	else rect_to_sphe(&azVision,&altVision,local_vision);

	// if we are moving in the Azimuthal angle (left/right)
	if (deltaAz) azVision-=deltaAz;
	if (deltaAlt) {
		if (altVision+deltaAlt <= M_PI_2 && altVision+deltaAlt >= -M_PI_2) altVision+=deltaAlt;
		if (altVision+deltaAlt > M_PI_2) altVision = M_PI_2 - 0.000001;		// Prevent bug
		if (altVision+deltaAlt < -M_PI_2) altVision = -M_PI_2 + 0.000001;	// Prevent bug
	}

	// recalc all the position variables
	if (deltaAz || deltaAlt) {
		if ( viewing_mode == VIEW_EQUATOR) {
			sphe_to_rect(azVision, altVision, equ_vision);
			local_vision=earth_equ_to_local(equ_vision);
		} else {
			sphe_to_rect(azVision, altVision, local_vision);
			// Calc the equatorial coordinate of the direction of vision wich was in Altazimuthal coordinate
			equ_vision=local_to_earth_equ(local_vision);
			prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;
		}
	}

	// Update the final modelview matrices
	update_model_view_mat(projector, fov);

}

////////////////////////////////////////////////////////////////////////////////
// Increment time
void Navigator::update_time(int delta_time)
{
	JDay+=time_speed*(double)delta_time/1000.;

	// Fix time limits to avoid ephemeris breakdowns
	if(JDay > NShadeDateTime::getMaxSimulationJD()) JDay = NShadeDateTime::getMaxSimulationJD();
	if(JDay < NShadeDateTime::getMinSimulationJD()) JDay = NShadeDateTime::getMinSimulationJD();

	update(delta_time);

	ObserverState state;
	state.heading = heading;
	NShadeDateTime::DateTimeFromJulianDay( JDay + NShadeDateTime::GMTShiftFromSystem(JDay)*0.041666666666,
										   &state.year, &state.month, &state.day,
			                               &state.hour, &state.minute, &state.seconds );
	SharedData::Instance()->Observer( state );
}

////////////////////////////////////////////////////////////////////////////////
// The non optimized (more clear version is available on the CVS : before date 25/07/2003)

// see vsop87.doc:

const Mat4d mat_j2000_to_vsop87(
    Mat4d::xrotation(-23.4392803055555555556*(M_PI/180)) *
    Mat4d::zrotation(0.0000275*(M_PI/180)));

const Mat4d mat_vsop87_to_j2000(mat_j2000_to_vsop87.transpose());

const Mat4d mat_j2000_to_galactic(
    Mat4d::zrotation(33*(M_PI/180)) *
    Mat4d::xrotation(-62.8717*(M_PI/180)) *
    Mat4d::zrotation(-282.85*(M_PI/180))
	);


const Mat4d mat_galactic_to_j2000(mat_j2000_to_galactic.transpose());

void Navigator::update_transform_matrices(void)
{
	mat_local_to_earth_equ = position->getRotLocalToEquatorial(JDay);
	mat_earth_equ_to_local = mat_local_to_earth_equ.transpose();

	mat_earth_equ_to_j2000 = mat_vsop87_to_j2000
	                         * position->getRotEquatorialToVsop87();
	mat_j2000_to_earth_equ = mat_earth_equ_to_j2000.transpose();

	mat_earth_equ_to_galactic = mat_j2000_to_galactic * mat_earth_equ_to_j2000;
	mat_galactic_to_earth_equ = mat_earth_equ_to_galactic.transpose();

	mat_helio_to_earth_equ =
	    mat_j2000_to_earth_equ *
	    mat_vsop87_to_j2000 *
	    Mat4d::translation(-position->getCenterVsop87Pos());


	// These two next have to take into account the position of the observer on the earth
	Mat4d tmp =
	    mat_j2000_to_vsop87 *
	    mat_earth_equ_to_j2000 *
	    mat_local_to_earth_equ;

	mat_local_to_helio =  Mat4d::translation(position->getCenterVsop87Pos()) *
	                      tmp *
	                      Mat4d::translation(Vec3d(0.,0., position->getDistanceFromCenter()));

	mat_helio_to_local =  Mat4d::translation(Vec3d(0.,0.,-position->getDistanceFromCenter())) *
	                      tmp.transpose() *
	                      Mat4d::translation(-position->getCenterVsop87Pos());


	mat_dome_fixed.set(0, -1, 0, 0,
	             -1, 0, 0, 0,
	             0, 0, -1, 0,
	             0, 0, 0, 1);

	mat_dome = Mat4d::zrotation(heading*M_PI/180.f) * mat_dome_fixed;
}

////////////////////////////////////////////////////////////////////////////////
// Update the modelview matrices
void Navigator::update_model_view_mat(Projector *projector, double fov)
{

	Vec3d f;

	if ( viewing_mode == VIEW_EQUATOR) {
		// view will use equatorial coordinates, so that north is always up
		f = equ_vision;
	} else {
		// view will correct for horizon (always down)
		f = local_vision;
	}


	f.normalize();
	Vec3d s(f[1],-f[0],0.);


	if ( viewing_mode == VIEW_EQUATOR) {
		// convert everything back to local coord
		f = local_vision;
		f.normalize();
		s = earth_equ_to_local( s );
	}

	Vec3d u(s^f);
	s.normalize();
	u.normalize();

	mat_local_to_eye.set(s[0],u[0],-f[0],0.,
	                     s[1],u[1],-f[1],0.,
	                     s[2],u[2],-f[2],0.,
	                     0.,0.,0.,1.);
	/*
	printf("local to eye\n%f %f %f 0\n%f %f %f 0\n%f %f %f 0\n0 0 0 1\n",
	       s[0], u[0], -f[0],
	       s[1], u[1], -f[1],
	       s[2], u[2], -f[2]);
	*/

//johannes
//    mat_local_to_eye =  Mat4d::zrotation(0.5*M_PI) * mat_local_to_eye;

	// Bug fix on amd
	//	mat_local_to_eye =  Mat4d::zrotation(0.0001) * mat_local_to_eye;

// redo view offset
	mat_local_to_eye =  Mat4d::xrotation(view_offset *fov/2.f*M_PI/180.f * view_offset_transition) * mat_local_to_eye;

// heading
	mat_local_to_eye =  Mat4d::zrotation(heading*M_PI/180.f + 0.0001) * mat_local_to_eye;

	mat_earth_equ_to_eye = mat_local_to_eye*mat_earth_equ_to_local;
	mat_helio_to_eye = mat_local_to_eye*mat_helio_to_local;
	mat_j2000_to_eye = mat_earth_equ_to_eye*mat_j2000_to_earth_equ;
	mat_galactic_to_eye = mat_earth_equ_to_eye*mat_galactic_to_earth_equ;
}

////////////////////////////////////////////////////////////////////////////////
// Return the observer heliocentric position
Vec3d Navigator::get_observer_helio_pos(void) const
{
	Vec3d v(0.,0.,0.);
	return mat_local_to_helio*v;
}

////////////////////////////////////////////////////////////////////////////////
// Move to the given equatorial position
void Navigator::move_to(const Vec3d& _aim, float move_duration, bool _local_pos, int zooming)
{

	Vec3d tmp = _aim;
	tmp.normalize();
	tmp *= 2.;

// already moving to correct position
	if ( flag_auto_move && fabs(move.aim[0] - tmp[0]) <= .00000001f
	        && fabs(move.aim[1] - tmp[1]) <= .00000001f
	        && fabs(move.aim[2] - tmp[2]) <= .00000001f ) {
		//    cout << "already moving here\n";
		return;
	} //else {
	//cout << fabs(move.aim[0] - tmp[0]) << " " << fabs(move.aim[1] - tmp[1])
	//<< " " << fabs(move.aim[2] - tmp[2]) << endl;
	//}

	zooming_mode = zooming;
	move.aim=_aim;
	move.aim.normalize();
	move.aim*=2.;
	if (_local_pos) {
		move.start=local_vision;
	} else {
		move.start=equ_vision;
	}
	move.start.normalize();
	move.speed=1.f/(move_duration*1000);
	move.coef=0.;
	move.local_pos = _local_pos;
	flag_auto_move = true;

}


////////////////////////////////////////////////////////////////////////////////
// Set type of viewing mode (align with horizon or equatorial coordinates)
void Navigator::set_viewing_mode(VIEWING_MODE_TYPE view_mode)
{
	viewing_mode = view_mode;

	// TODO: include some nice smoothing function trigger here to rotate between
	// the two modes

}

void Navigator::switch_viewing_mode(void)
{
	if (viewing_mode==VIEW_HORIZON) set_viewing_mode(VIEW_EQUATOR);
	else set_viewing_mode(VIEW_HORIZON);
}

// move gradually to a new heading
void Navigator::change_heading(double _heading, int duration)
{
	flag_change_heading = 1;

	start_heading = heading;
	end_heading = _heading;

	move_to_coef = 1.0f/duration;
	move_to_mult = 0;

}

// for moving heading position gradually
void Navigator::update(int delta_time)
{
	if (flag_change_heading) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_change_heading = 0;
		}

		heading = start_heading - move_to_mult*(start_heading-end_heading);
	}
}

