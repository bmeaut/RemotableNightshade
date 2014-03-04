/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Ch�eau
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

#ifndef _CONSTELLATION_MGR_H_
#define _CONSTELLATION_MGR_H_

#include <vector>

#include "object.h"
#include "fader.h"
#include "loadingbar.h"
#include "translator.h"

class HipStarMgr;
class Constellation;
class Projector;
class Navigator;

class ConstellationMgr
{
public:
	ConstellationMgr(HipStarMgr *_hip_stars);
	~ConstellationMgr();

	//! Draw constellation lines, art, names and boundaries if activated
	void draw(Projector* prj, Navigator* nav) const;

	//! Update faders
	void update(int delta_time);

	//! @brief Read constellation names from the given file
	//! @param namesFile Name of the file containing the constellation names in english
	void loadNames(const string& names_file);

	//! @brief Update i18 names from english names according to current locale
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	//! @brief Load constellation line shapes, art textures and boundaries shapes from data files
	int loadLinesAndArt(const string& skyCultureDir, LoadingBar& lb);

	//! Set constellation art fade duration
	void setArtFadeDuration(float duration);
	//! Get constellation art fade duration
	float getArtFadeDuration() const {
		return artFadeDuration;
	}

	//! Set constellation maximum art intensity
	void setArtIntensity(float f);
	//! Set constellation maximum art intensity
	float getArtIntensity() const {
		return artMaxIntensity;
	}

	//! Set whether constellation art will be displayed
	void setFlagArt(bool b);
	//! Get whether constellation art is displayed
	bool getFlagArt(void) const {
		return flagArt;
	}

	//! Set whether constellation path lines will be displayed
	void setFlagLines(bool b);
	//! Get whether constellation path lines are displayed
	bool getFlagLines(void) const {
		return flagLines;
	}

	//! Set whether constellation boundaries lines will be displayed
	void setFlagBoundaries(bool b);
	//! Get whether constellation boundaries lines are displayed
	bool getFlagBoundaries(void) const {
		return flagBoundaries;
	}

	//! Set whether constellation names will be displayed
	void setFlagNames(bool b);
	//! Set whether constellation names are displayed
	bool getFlagNames(void) const {
		return flagNames;
	}

	//! Set whether selected constellation must be displayed alone
	void setFlagIsolateSelected(bool s) {
		isolateSelected = s;
	}
	//! Get whether selected constellation is displayed alone
	bool getFlagIsolateSelected(void) const {
		return isolateSelected;
	}

	//! Define wehther lable are print with gravity effect
	void setFlagGravityLabel(bool g);

	//! Define line color
	void setLineColor(const Vec3f& c);
	//! Get line color
	Vec3f getLineColor() const;

	//! Define boundary color
	void setBoundaryColor(const Vec3f& c);
	//! Get current boundary color
	Vec3f getBoundaryColor() const;

	//! Set label color for names
	void setLabelColor(const Vec3f& c);
	//! Get label color for names
	Vec3f getLabelColor() const;

	//! Define art color
	void setArtColor(const Vec3f& c);
	//! Get art color
	Vec3f getArtColor() const;


	//! Define font file name and size to use for constellation names display
	void setFont(float font_size, const string& font_name);

	//! Define which constellation is selected from its abbreviation
	void setSelected(const string& abbreviation);

	//! Define which constellation to unselect by its abbreviation
	void unsetSelected(const string& abbreviation);

	//! Define which constellation is selected and return brightest star
	ObjectBaseP setSelectedStar(const string& abbreviation);

	//! Define which constellation is selected from a star number
	void setSelected(const Object &s) {
		if (!s) setSelectedConst(NULL);
		else setSelectedConst(is_star_in(s));
	}

	//! Remove constellation from selected list
	void unsetSelectedConst(Constellation * c);

	//! Remove all selected constellations
	void deselect() {
		setSelected(Object(NULL));
	}

	Object getSelected(void) const;

	//! Return the matching constellation object's pointer if exists or NULL
	//! @param nameI18n The case sensistive constellation name
	Object searchByNameI18n(const string& nameI18n) const;

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	vector<string> listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem=5) const;
private:
	bool loadBoundaries(const string& conCatFile);
	void draw_lines(Projector * prj) const;
	void draw_art(Projector * prj, Navigator * nav) const;
	void draw_names(Projector * prj) const;
	void drawBoundaries(Projector* prj) const;
	void setSelectedConst(Constellation* c);

	Constellation* is_star_in(const Object &s) const;
	Constellation* findFromAbbreviation(const string& abbreviation) const;
	vector<Constellation*> asterisms;
	s_font *asterFont;
	HipStarMgr *hipStarMgr;
	vector<Constellation*> selected;
	bool isolateSelected;
	vector<vector<Vec3f> *> allBoundarySegments;

	// These are THE master settings - individual constellation settings can vary based on selection status
	bool flagNames;
	bool flagLines;
	bool flagArt;
	bool flagBoundaries;
	float artFadeDuration;
	float artMaxIntensity;
};

#endif // _CONSTELLATION_MGR_H_
