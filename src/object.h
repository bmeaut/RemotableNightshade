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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "vecmath.h"
#include "../nscontrol/src/nshade_state.h"
#include <iostream>
#include "object_type.h"

using namespace std;

class Navigator;
class Projector;
class s_texture;
class ObjectBase;

class Object
{
public:

	/**
	 * for WebApi
	 */
	ObjectBase* getObject() { return rep; }

	Object(void);
	~Object(void);
	Object(ObjectBase *r);
	Object(const Object &o);
	const Object &operator=(const Object &o);
	const Object &operator=(ObjectBase* const r );
	operator bool(void) const;
	bool operator==(const Object &o) const;

	void update(void);
	void draw_pointer(int delta_time,
	                  const Projector *prj,
	                  const Navigator *nav);

	//! Write I18n information about the object in string.
	string getInfoString(const Navigator *nav) const;

	//! The returned string can typically be used for object labeling in the sky
	string getShortInfoString(const Navigator *nav) const;

	//! Return object's type
	ObjectRecord::OBJECT_TYPE get_type(void) const;

	//! Return object's name
	string getEnglishName(void) const;
	string getNameI18n(void) const;

	float getStarDistance( void );

	// Stupid hack for figuring out if need to unselect before can delete planets safely
	bool isDeleteable(void) const;

	//! Get position in earth equatorial frame
	Vec3d get_earth_equ_pos(const Navigator *nav) const;

	//! observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const;

	//! Return object's magnitude
	float get_mag(const Navigator *nav) const;

	//! Get object main color, used to display infos
	Vec3f get_RGB(void) const;

	ObjectBaseP getBrightestStarInConstellation(void) const;

	// only needed for AutoZoomIn/Out, whatever this is:
	//! Return the best FOV in degree to use for a close view of the object
	double get_close_fov(const Navigator *nav) const;
	//! Return the best FOV in degree to use for a global view
	//! of the object satellite system (if there are satellites)
	double get_satellites_fov(const Navigator *nav) const;
	double get_parent_satellites_fov(const Navigator *nav) const;

	float get_on_screen_size(const Projector *prj, const Navigator *nav,  bool orb_only = false);

	static void init_textures(void);
	static void delete_textures(void);
private:
	ObjectBase *rep;
};

#endif
