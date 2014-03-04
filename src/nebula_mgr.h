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

#ifndef _NEBULA_MGR_H_
#define _NEBULA_MGR_H_

#include <vector>
#include "object.h"
#include "fader.h"
#include "grid.h"
#include "shared_data.h"

using namespace std;

class Nebula;
class LoadingBar;
class Translator;
class ToneReproductor;

class NebulaMgr
{
public:
	NebulaMgr();
	virtual ~NebulaMgr();

	// Read the Nebulas data from files
	bool read(float font_size, const string& font_name, const string& catNGC, const string& catNGCnames, const string& catTextures, LoadingBar& lb);

	// Load an individual nebula from a script
	bool loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
					string name, string filename, string credit, double texture_luminance_adjust,
					double distance = -1);

	// remove user added nebula and optionally unhide the original of the same name
	string removeNebula(const string& name, bool showOriginal);

	// remove all user added nebula
	string removeSupplementalNebulae();

	// Draw all the Nebulas
	void draw(Projector *prj, const Navigator *nav, ToneReproductor *eye, double sky_brightness);
	void update(int delta_time) {
		hintsFader.update(delta_time);
		flagShow.update(delta_time);
	}

	Object search(const string& name);  // search by name M83, NGC 1123, IC 1234
	Nebula *searchNebula(const string& name, bool search_hidden);  // same but return a nebula object
	Object search(Vec3f Pos);    // Search the Nebulae by position


	void setNebulaCircleScale(float scale);
	float getNebulaCircleScale(void) const;

	void setHintsFadeDuration(float duration) {
		hintsFader.set_duration((int) (duration * 1000.f));
	}

	void setFlagHints(bool b) {
		hintsFader=b;
		ObjectsState state;
		state.m_state.dso_labels = b;
		SharedData::Instance()->Objects(state);
	}
	bool getFlagHints(void) const {
		return hintsFader;
	}

	void setFlagShow(bool b) {
		flagShow = b;
	}
	bool getFlagShow(void) const {
		return flagShow;
	}

	void setLabelColor(const Vec3f& c);
	const Vec3f &getLabelColor(void) const;

	void setCircleColor(const Vec3f& c);
	const Vec3f &getCircleColor(void) const;

	// Return a stl vector containing the nebulas located inside the lim_fov circle around position v
	vector<Object> search_around(Vec3d v, double lim_fov) const;

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	//! Set flag for displaying Nebulae as bright
	void setFlagBright(bool b);
	//! Get flag for displaying Nebulae as bright
	bool getFlagBright(void) const;

	//! Set flag for displaying Nebulae even without textures
	void setFlagDisplayNoTexture(bool b) {
		displayNoTexture = b;
	}
	//! Get flag for displaying Nebulae without textures
	bool getFlagDisplayNoTexture(void) const {
		return displayNoTexture;
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void setMaxMagHints(float f) {
		maxMagHints = f;
		SettingsState state;
		state.m_state.max_mag_nebula_name = f;
		SharedData::Instance()->Settings( state );
	}
	//! Get maximum magnitude at which nebulae hints are displayed
	float getMaxMagHints(void) const {
		return maxMagHints;
	}

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	vector<string> listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem=5) const;

	//! Return the matching Nebula object's pointer if exists or NULL
	//! @param nameI18n The case sensistive nebula name or NGC M catalog name : format can be M31, M 31, NGC31 NGC 31
	Object searchByNameI18n(const string& nameI18n) const;
private:
	Nebula *searchM(unsigned int M);
	Nebula *searchNGC(unsigned int NGC);
	Nebula *searchIC(unsigned int IC);
	/*Nebula *searchUGC(unsigned int UGC);*/
	bool loadNGC(const string& fileName, LoadingBar& lb);
	bool loadNGCNames(const string& fileName);
	bool loadTextures(const string& fileName, LoadingBar& lb);

	FILE *nebula_fic;
	vector<Nebula*> neb_array;		// The nebulas list
	LinearFader hintsFader;
	LinearFader flagShow;

	vector<Nebula*>* nebZones;		// array of nebula vector with the grid id as array rank
	Grid nebGrid;					// Grid for opimisation

	float maxMagHints;				// Define maximum magnitude at which nebulae hints are displayed
	bool displayNoTexture;			// Define if nebulas without textures are to be displayed
};

#endif // _NEBULA_MGR_H_
