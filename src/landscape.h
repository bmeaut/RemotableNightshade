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

#ifndef _LANDSCAPE_H_
#define _LANDSCAPE_H_

#include <string>
#include "s_texture.h"
#include "vecmath.h"
#include "fmath.h"
#include "tone_reproductor.h"
#include "projector.h"
#include "navigator.h"
#include "fader.h"
#include "utility.h"

// Class which manages the displaying of the Landscape
class Landscape
{
public:
	enum LANDSCAPE_TYPE {
		OLD_STYLE,
		FISHEYE,
		SPHERICAL
	};

	Landscape(float _radius = 2.);
	virtual ~Landscape();
	virtual void load(const string& file_name, const string& section_name) = 0;
	void set_parameters(const Vec3f& sun_pos);
	void set_sky_brightness(float b) {
		sky_brightness = b;
	}

	//! Set whether landscape is displayed (does not concern fog)
	void setFlagShow(bool b);

	//! Get whether landscape is displayed (does not concern fog)
	bool getFlagShow() const {
		return (bool)land_fader;
	}
	//! Set whether fog is displayed
	void setFlagShowFog(bool b) {
		fog_fader=b;
	}
	//! Get whether fog is displayed
	bool getFlagShowFog() const {
		return (bool)fog_fader;
	}
	//! Get landscape name
	string getName() const {
		return name;
	}
	//! Get landscape author name
	string getAuthorName() const {
		return author;
	}
	//! Get landscape description
	string getDescription() const {
		return description;
	}

	void update(int delta_time) {
		land_fader.update(delta_time);
		fog_fader.update(delta_time);
	}
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav) = 0;

	static Landscape* create_from_file(const string& landscape_file, const string& section_name);
	static Landscape* create_from_hash(stringHash_t & param);
	static string get_file_content(const string& landscape_file);
	static string getLandscapeNames(const string& landscape_file);
	static string nameToKey(const string& landscape_file, const string & name);
protected:
	//! Load attributes common to all landscapes
	void loadCommon(const string& landscape_file, const string& section_name);
	float radius;
	string name;
	float sky_brightness;
	bool valid_landscape;   // was a landscape loaded properly?
	LinearFader land_fader;
	LinearFader fog_fader;
	string author;
	string description;
};

typedef struct {
	s_texture* tex;
	float tex_coords[4];
} landscape_tex_coord;

class LandscapeOldStyle : public Landscape
{
public:
	LandscapeOldStyle(float _radius = 2.);
	virtual ~LandscapeOldStyle();
	virtual void load(const string& fileName, const string& section_name);
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);
	void create(bool _fullpath, stringHash_t param);
private:
	void draw_fog(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const;
	void draw_decor(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const;
	void draw_ground(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const;
	s_texture** side_texs;
	int nb_side_texs;
	int nb_side;
	landscape_tex_coord* sides;
	s_texture* fog_tex;
	landscape_tex_coord fog_tex_coord;
	s_texture* ground_tex;
	landscape_tex_coord ground_tex_coord;
	int nb_decor_repeat;
	float fog_alt_angle;
	float fog_angle_shift;
	float decor_alt_angle;
	float decor_angle_shift;
	float decor_angle_rotatez;
	float ground_angle_shift;
	float ground_angle_rotatez;
	int draw_ground_first;
};

class LandscapeFisheye : public Landscape
{
public:
	LandscapeFisheye(float _radius = 1.);
	virtual ~LandscapeFisheye();
	virtual void load(const string& fileName, const string& section_name);
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);
	void create(const string _name, bool _fullpath, const string _maptex, double _texturefov,
	            const float _rotate_z, const string _maptex_night, const bool _mipmap);
private:

	s_texture* map_tex;
	s_texture* map_tex_night;
	float tex_fov;
	float rotate_z; // rotation around the z axis
};


class LandscapeSpherical : public Landscape
{
public:
	LandscapeSpherical(float _radius = 1.);
	virtual ~LandscapeSpherical();
	virtual void load(const string& fileName, const string& section_name);
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);
	void create(const string _name, bool _fullpath, const string _maptex, const float _base_altitude,
	            const float _top_altitude, const float _rotate_z, const string _maptex_night, const bool _mipmap);
private:

	s_texture* map_tex;
	s_texture* map_tex_night;
	float base_altitude, top_altitude;  // for partial sphere coverage
	float rotate_z; // rotation around the z axis
};

#endif // _LANDSCAPE_H_
