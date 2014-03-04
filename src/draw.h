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

#ifndef __DRAW_H__
#define __DRAW_H__

#include <string>
#include <fstream>
#include "nightshade.h"
#include "s_font.h"
#include "projector.h"
#include "navigator.h"
#include "tone_reproductor.h"
#include "fader.h"
#include "translator.h"
#include "shared_data.h"

// Class which manages a grid to display in the sky
class SkyGrid
{
public:
	enum SKY_GRID_TYPE {
		EQUATORIAL,
		ALTAZIMUTAL,
		GALACTIC
	};
	// Create and precompute positions of a SkyGrid
	SkyGrid(SKY_GRID_TYPE grid_type = EQUATORIAL, unsigned int _nb_meridian = 24, unsigned int _nb_parallel = 17,
	        double _radius = 1., unsigned int _nb_alt_segment = 18, unsigned int _nb_azi_segment = 50);
	virtual ~SkyGrid();
	void draw(const Projector* prj) const;
	void set_font(float font_size, const string& font_name);
	void setColor(const Vec3f& c) {
		color = c;
		SettingsState state;
		if( gtype == EQUATORIAL ) {
			state.m_state.equator_grid[0] = c[0];
			state.m_state.equator_grid[1] = c[1];
			state.m_state.equator_grid[2] = c[2];
		}
		else if( gtype == ALTAZIMUTAL ) {
			state.m_state.azimuth_grid[0] = c[0];
			state.m_state.azimuth_grid[1] = c[1];
			state.m_state.azimuth_grid[2] = c[2];
		}
		else if( gtype == GALACTIC ) {
			state.m_state.galactic_grid[0] = c[0];
			state.m_state.galactic_grid[1] = c[1];
			state.m_state.galactic_grid[2] = c[2];
		}
		SharedData::Instance()->Settings(state);
	}
	const Vec3f& getColor() {
		return color;
	}
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void set_fade_duration(float duration) {
		fader.set_duration((int)(duration*1000.f));
	}
	void setFlagshow(bool b) {
		fader = b;
		ReferenceState state;
		if( gtype == ALTAZIMUTAL )
			state.azimuthal_grid = b;
		else if( gtype == EQUATORIAL )
			state.equatorial_grid = b;
		else if( gtype == GALACTIC )
			state.galactic_grid = b;
		SharedData::Instance()->References( state );
	}
	bool getFlagshow(void) const {
		return fader;
	}
	void set_top_transparancy(bool b) {
		transparent_top= b;
	}
private:

	unsigned int nb_meridian;
	unsigned int nb_parallel;
	double radius;
	unsigned int nb_alt_segment;
	unsigned int nb_azi_segment;
	bool transparent_top;
	Vec3f color;
	Vec3f** alt_points;
	Vec3f** azi_points;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	s_font* font;
	SKY_GRID_TYPE gtype;
	LinearFader fader;
};


// Class which manages a line to display around the sky like the ecliptic line
class SkyLine
{
public:
	enum SKY_LINE_TYPE {
		EQUATOR,
		ECLIPTIC,
		PRECESSION,
		CIRCUMPOLAR,
		TROPIC,
		LOCAL,
		MERIDIAN
	};
	// Create and precompute positions of a SkyGrid
	SkyLine(SKY_LINE_TYPE _line_type = EQUATOR, double _radius = 1., unsigned int _nb_segment = 48);
	virtual ~SkyLine();
	//	void draw(const Projector* prj) const; 20060825 patch
	void draw(const Projector *prj,const Navigator *nav) const;
	void setColor(const Vec3f& c) {
		color = c;
		SettingsState state;
		if( line_type == EQUATOR ) {
			state.m_state.equator_line[0] = c[0];
			state.m_state.equator_line[1] = c[1];
			state.m_state.equator_line[2] = c[2];
		}
		else if( line_type == ECLIPTIC ) {
			state.m_state.ecliptic_line[0] = c[0];
			state.m_state.ecliptic_line[1] = c[1];
			state.m_state.ecliptic_line[2] = c[2];
		}
		else if( line_type == MERIDIAN ) {
			state.m_state.meridian_line[0] = c[0];
			state.m_state.meridian_line[1] = c[1];
			state.m_state.meridian_line[2] = c[2];
		}
		else if( line_type == PRECESSION ) {
			state.m_state.precession_circle[0] = c[0];
			state.m_state.precession_circle[1] = c[1];
			state.m_state.precession_circle[2] = c[2];
		}
		else if( line_type == CIRCUMPOLAR ) {
			state.m_state.circumpolar_circle[0] = c[0];
			state.m_state.circumpolar_circle[1] = c[1];
			state.m_state.circumpolar_circle[2] = c[2];
		}
		SharedData::Instance()->Settings(state);
	}
	const Vec3f& getColor() {
		return color;
	}
	void translateLabels(Translator& trans);  // for i18n
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void set_fade_duration(float duration) {
		fader.set_duration((int)(duration*1000.f));
	}
	void setFlagshow(bool b) {
		fader = b;
		ReferenceState state;
		if( line_type == MERIDIAN )
			state.meridian_line = b;
		else if( line_type == TROPIC )
			state.tropic_lines = b;
		else if( line_type == ECLIPTIC )
			state.ecliptic_line = b;
		else if( line_type == EQUATOR )
			state.equator_line = b;
		else if( line_type == PRECESSION )
			state.precession_circle = b;
		else if( line_type == CIRCUMPOLAR )
			state.circumpolar_circle = b;
		SharedData::Instance()->References( state );
	}
	bool getFlagshow(void) const {
		return fader;
	}
	void set_font(float font_size, const string& font_name);

private:

	void drawTick(Vec3d &pt1, Vec3d &pt2) const;

	double radius;
	unsigned int nb_segment;
	SKY_LINE_TYPE line_type;
	Vec3f color;
	Vec3f* points;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	LinearFader fader;
	s_font * font;
	string month[13]; // labels for translating on ecliptic
};

// Class which manages the cardinal points displaying
class Cardinals
{
public:
	Cardinals(float _radius = 1.);
	virtual ~Cardinals();
	void draw(const Projector* prj, double latitude, bool gravityON = false) const;
	void setColor(const Vec3f& c) {
		color = c;
		SettingsState state;
		state.m_state.cardinal_points[0] = c[0];
		state.m_state.cardinal_points[1] = c[1];
		state.m_state.cardinal_points[2] = c[2];
		SharedData::Instance()->Settings(state);
	}
	Vec3f get_color() {
		return color;
	}
	void set_font(float font_size, const string& font_name);
	void translateLabels(Translator& trans);  // for i18n
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void set_fade_duration(float duration) {
		fader.set_duration((int)(duration*1000.f));
	}
	void setFlagShow(bool b) {
		fader = b;
		ReferenceState state;
		state.cardinal_points = b;
		SharedData::Instance()->References( state );
	}
	bool getFlagShow(void) const {
		return fader;
	}

private:
	float radius;
	s_font* font;
	Vec3f color;
	string sNorth, sSouth, sEast, sWest;
	LinearFader fader;
};

// Class which manages the displaying of the Milky Way
class MilkyWay
{
public:
	MilkyWay(float _radius = 1.);
	virtual ~MilkyWay();
	void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const;
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void set_intensity(float _intensity);
	float get_intensity() {
		return intensity;
	};
	void set_texture(const string& tex_file, bool blend = false, bool make_default = false);
	void setColor(const Vec3f& c) {
		color=c;
	}
	void setFlagShow(bool b) {
		fader = b;
	}
	bool getFlagShow(void) const {
		return fader;
	}

private:
	float radius;
	s_texture* tex;
	s_texture* default_tex;  // Default milky way texture can always fall back to
	Vec3f color;
	float intensity;
	LinearFader fader;
};

// Class which manages the displaying of the Milky Way
class Draw
{
public:
	// Draw a point... (used for tests)
	static void drawPoint(float X,float Y,float Z);
};

#endif // __DRAW_H__
