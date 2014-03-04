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

#include "object.h"
#include "object_base.h"

class ObjectUninitialized : public ObjectBase
{
public:
	ObjectUninitialized(void) {}
private:
	string getInfoString(const Navigator *nav) const {
		return "";
	}
	string getShortInfoString(const Navigator *nav) const {
		return "";
	}
	ObjectRecord::OBJECT_TYPE get_type(void) const {
		return ObjectRecord::OBJECT_UNINITIALIZED;
	}
	string getEnglishName(void) const {
		return "";
	}
	string getNameI18n(void) const {
		return "";
	}
	Vec3d get_earth_equ_pos(const Navigator*) const {
		return Vec3d(1,0,0);
	}
	Vec3d getObsJ2000Pos(const Navigator*) const {
		return Vec3d(1,0,0);
	}
	float get_mag(const Navigator * nav) const {
		return -10;
	}
	bool isDeleteable() const {
		return 0;
	}
};

static ObjectUninitialized uninitialized_object;

Object::~Object(void)
{
	rep->release();
}

Object::Object(void)
		:rep(&uninitialized_object)
{
	rep->retain();
}

Object::Object(ObjectBase *r)
		:rep(r?r:&uninitialized_object)
{
	rep->retain();
}

Object::Object(const Object &o)
		:rep(o.rep)
{
	rep->retain();
}

const Object &Object::operator=(const Object &o)
{
	if (this != &o) {
		rep = o.rep;
		rep->retain();
	}
	return *this;
}

const Object &Object::operator=(ObjectBase* const r)
{
	if(r) {
		rep = r;
		rep->retain();
	}
	else
		rep = &uninitialized_object;

	return *this;
}

Object::operator bool(void) const
{
	return (rep != &uninitialized_object);
}

bool Object::operator==(const Object &o) const
{
	return (rep == o.rep);
}

void Object::update(void)
{
	rep->update();
}

void Object::draw_pointer(int delta_time,
                          const Projector *prj,
                          const Navigator *nav)
{
	rep->draw_pointer(delta_time,prj,nav);
}

string Object::getInfoString(const Navigator *nav) const
{
	return rep->getInfoString(nav);
}

string Object::getShortInfoString(const Navigator *nav) const
{
	return rep->getShortInfoString(nav);
}

ObjectRecord::OBJECT_TYPE Object::get_type(void) const
{
	return rep->get_type();
}

string Object::getEnglishName(void) const
{
	return rep->getEnglishName();
}

bool Object::isDeleteable(void) const
{
	return rep->isDeleteable();
}


string Object::getNameI18n(void) const
{
	return rep->getNameI18n();
}

Vec3d Object::get_earth_equ_pos(const Navigator *nav) const
{
	return rep->get_earth_equ_pos(nav);
}

Vec3d Object::getObsJ2000Pos(const Navigator *nav) const
{
	return rep->getObsJ2000Pos(nav);
}

float Object::get_mag(const Navigator *nav) const
{
	return rep->get_mag(nav);
}

Vec3f Object::get_RGB(void) const
{
	return rep->get_RGB();
}

ObjectBaseP Object::getBrightestStarInConstellation(void) const
{
	return rep->getBrightestStarInConstellation();
}

double Object::get_close_fov(const Navigator *nav) const
{
	return rep->get_close_fov(nav);
}

double Object::get_satellites_fov(const Navigator *nav) const
{
	return rep->get_satellites_fov(nav);
}

double Object::get_parent_satellites_fov(const Navigator *nav) const
{
	return rep->get_parent_satellites_fov(nav);
}

void Object::init_textures(void)
{
	ObjectBase::init_textures();
}

void Object::delete_textures(void)
{
	ObjectBase::delete_textures();
}

float Object::get_on_screen_size(const Projector *prj, const Navigator *nav, bool orb_only)
{
	return rep->get_on_screen_size(prj, nav, orb_only);
}

float Object::getStarDistance( void ) {
	return rep->getStarDistance();
}
