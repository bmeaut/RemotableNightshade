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

#ifndef _CONSTELLATION_H_
#define _CONSTELLATION_H_

#include "nightshade.h"
#include "object_base.h"
#include "object_type.h"
#include "object.h"
#include "utility.h"
#include "s_font.h"
#include "fader.h"
#include <vector>

class HipStarMgr;

class Constellation : public ObjectBase
{
	friend class ConstellationMgr;
private:
	Constellation();
	~Constellation();

	/**
	 * for WebApi
	 */
	s_texture getTexture() { return *art_tex; }

	// Object method to override
	//! Write I18n information about the object.
	string getInfoString(const Navigator * nav) const {
		return getNameI18n() + "(" + getShortName() + ")";
	}
	//! The returned string can typically be used for object labeling in the sky
	string getShortInfoString(const Navigator * nav) const {
		return getNameI18n();
	}
	//! Return object's type
	ObjectRecord::OBJECT_TYPE get_type(void) const {
		return ObjectRecord::OBJECT_CONSTELLATION;
	}
	//! Get position in earth equatorial frame
	Vec3d get_earth_equ_pos(const Navigator *nav) const {
		return XYZname;
	}
	//! observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const {
		return XYZname;
	}
	//! Return object's magnitude
	float get_mag(const Navigator * nav) const {
		return 0.;
	}

	bool read(const string& record, HipStarMgr * _VouteCeleste);
	void draw_name(s_font * constfont, Projector* prj) const;
	void draw_art(Projector* prj, Navigator* nav) const;
	void draw_boundary_optim(Projector* prj) const;
	const Constellation* is_star_in(const Object&) const;
	ObjectBaseP getBrightestStarInConstellation(void) const;

	//! Return translated name in UTF8 string
	string getNameI18n(void) const {
		return nameI18;
	}
	string getEnglishName(void) const {
		return abbreviation;
	}
	string getShortName(void) const {
		return abbreviation;
	}

	void draw_optim(Projector* prj) const;
	void draw_art_optim(Projector* prj, Navigator* nav) const;
	void update(int delta_time);

	void setFlagLines(bool b) {
		line_fader=b;
	}
	void setFlagBoundaries(bool b) {
		boundary_fader=b;
	}
	void setFlagName(bool b) {
		name_fader=b;
	}
	void setFlagArt(bool b) {
		art_fader=b;
	}
	bool getFlagLines(void) const {
		return line_fader;
	}
	bool getFlagBoundaries(void) const {
		return boundary_fader;
	}
	bool getFlagName(void) const {
		return name_fader;
	}
	bool getFlagArt(void) const {
		return art_fader;
	}

	//! Translated name in UTF8 format (translated using gettext)
	string nameI18;

	/** Name in english */
	string englishName;

	/** Abbreviation (of the latin name for western constellations) */
	string abbreviation;

	/** Direction vector pointing on constellation name drawing position */
	Vec3f XYZname;
	Vec3d XYname;

	/** Number of segments in the lines */
	unsigned int nb_segments;

	/** List of stars forming the segments */
	ObjectBaseP* asterism;

	s_texture* art_tex;
	Vec3d art_vertex[9];

	/** Define whether art, lines, names and boundary must be drawn */
	LinearFader art_fader, line_fader, name_fader, boundary_fader;

	vector<vector<Vec3f> *> isolatedBoundarySegments;
	vector<vector<Vec3f> *> sharedBoundarySegments;

	// Currently we only need one color for all constellations, this may change at some point
	static Vec3f lineColor;
	static Vec3f labelColor;
	static Vec3f boundaryColor;
	static Vec3f artColor;

	/** Whether labels are to be printed with gravity */
	static bool gravityLabel;
	static bool singleSelected;
};

#endif // _CONSTELLATION_H_
