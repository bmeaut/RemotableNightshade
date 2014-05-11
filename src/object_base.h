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

#ifndef _OBJECT_BASE_H_
#define _OBJECT_BASE_H_

#include "vecmath.h"
#include "../nscontrol/src/nshade_state.h"
#include <iostream>
#include "object_type.h"
#include "s_texture.h"

using namespace std;

class Navigator;
class Projector;

class ObjectBase;
void intrusive_ptr_add_ref(ObjectBase* p);
void intrusive_ptr_release(ObjectBase* p);

class ObjectBase
{
public:
	/**
	 * for WebApi
	 */
	virtual s_texture getTexture() { return s_texture("no_texture"); }

	virtual ~ObjectBase(void) {}
	virtual void retain(void) {}
	virtual void release(void) {}

	virtual void update(void) {}
	void draw_pointer(int delta_time,
	                  const Projector* prj,
	                  const Navigator *nav);

	//! Write I18n information about the object in string.
	virtual string getInfoString(const Navigator *nav) const = 0;

	//! The returned string can typically be used for object labeling in the sky
	virtual string getShortInfoString(const Navigator *nav) const = 0;

	virtual float getStarDistance( void ){ return 0; };

	//! Return object's type
	virtual ObjectRecord::OBJECT_TYPE get_type(void) const = 0;

	//! Return object's name
	virtual string getEnglishName(void) const = 0;
	virtual string getNameI18n(void) const = 0;

	virtual bool isDeleteable() const {
		return 0;
	}

	//! Get position in earth equatorial frame
	virtual Vec3d get_earth_equ_pos(const Navigator *nav) const = 0;

	//! observer centered J2000 coordinates
	virtual Vec3d getObsJ2000Pos(const Navigator *nav) const = 0;

	//! Return object's magnitude
	virtual float get_mag(const Navigator *nav) const = 0;

	//! Get object main color, used to display infos
	virtual Vec3f get_RGB(void) const {
		return Vec3f(1.,1.,1.);
	}

	virtual ObjectBaseP getBrightestStarInConstellation(void) const;

	//! Return the best FOV in degree to use for a close view of the object
	virtual double get_close_fov(const Navigator *nav) const {
		return 10.;
	}

	//! Return the best FOV in degree to use for a global view of the object satellite system (if there are satellites)
	virtual double get_satellites_fov(const Navigator *nav) const {
		return -1.;
	}
	virtual double get_parent_satellites_fov(const Navigator *nav) const {
		return -1.;
	}

	static void init_textures(void);
	static void delete_textures(void);
//protected:
	virtual float get_on_screen_size(const Projector *prj,
	                                 const Navigator *nav = NULL,
	                                 bool orb_only = false) {
		return 0;
	}
private:
	static int local_time;
	static s_texture * pointer_star;
	static s_texture * pointer_planet;
	static s_texture * pointer_nebula;
	static s_texture * pointer_telescope;
};

#endif
