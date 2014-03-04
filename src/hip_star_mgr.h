/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#ifndef _STAR_MGR_H_
#define _STAR_MGR_H_

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "fader.h"
#include "core.h"
#include "object_type.h"

using namespace std;

class Object;
class ToneReproductor;
class Projector;
class Navigator;
class LoadingBar;
class SFont;

namespace BigStarCatalogExtension {
  class ZoneArray;
  class HipIndexStruct;
}

//! @class StarMgr 
//! Stores the star catalogue data.
//! Used to render the stars themselves, as well as determine the color table
//! and render the labels of those stars with names for a given SkyCulture.
//! 
//! The celestial sphere is split into zones, which correspond to the
//! triangular faces of a geodesic sphere. The number of zones (faces)
//! depends on the level of sub-division of this sphere. The lowest
//! level, 0, is an icosahedron (20 faces), subsequent levels, L,
//! of sub-division give the number of zones, n as:
//!
//! n=20 x 4^L
//!
//! Stellarium uses levels 0 to 7 in the existing star catalogues.
//! Star Data Records contain the position of a star as an offset from
//! the central position of the zone in which that star is located,
//! thus it is necessary to determine the vector from the observer
//! to the centre of a zone, and add the star's offsets to find the
//! absolute position of the star on the celestial sphere.
//!
//! This position for a star is expressed as a 3-dimensional vector
//! which points from the observer (at the centre of the geodesic sphere)
//! to the position of the star as observed on the celestial sphere.

class HipStarMgr
{
public:
	HipStarMgr(void);
	~HipStarMgr(void);
	
	///////////////////////////////////////////////////////////////////////////
	// Methods defined in the StelModule class
	//! Initialize the StarMgr.
	//! - Loads the star catalogue data into memory
	//! - Sets up the star color table
	//! - Loads the star texture
	//! - Loads the star font (for labels on named stars)
	//! - Loads the texture of the sar selection indicator
	//! - Lets various display flags from the ini parser object
	//!
	//! @param conf The ini parser object containing relevant settings.
	//! @param lb The LoadingBar object which shows progress and current operation.
	virtual void init(float font_size, const string& font_name, LoadingBar& lb, const InitParser &conf);
	
	
	//! Draw the stars and the star selection indicator if necessary.
	virtual double draw(Core* core, ToneReproductor* eye, Projector* prj);
	
	//! Update any time-dependent features.
	//! Includes fading in and out stars and labels when they are turned on and off.
	virtual void update(double deltaTime) {names_fader.update((int)(deltaTime*1000)); starsFader.update((int)(deltaTime*1000));}
	
	//! Translate text.
	virtual void updateI18n(Translator& trans);
	
	//! Sets the colour scheme (night / chart etc).
	virtual void setColorScheme(const InitParser& conf, const string& section);
	
	///////////////////////////////////////////////////////////////////////////
	// Methods defined in StelObjectManager class
	//! Return a stl vector containing the stars located inside the lim_fov circle around position v
	virtual vector<ObjectBaseP > searchAround(const Vec3d& v, double limitFov, const Core* core) const;

	//! Return the matching Stars object's pointer if exists or NULL
	//! @param nameI18n The case sensistive star common name or HP
	//! catalog name (format can be HP1234 or HP 1234) or sci name
	virtual ObjectBaseP searchByNameI18n(const string& nameI18n) const;

	//! Return the matching star if exists or NULL
	//! @param name The case sensistive standard program planet name
	virtual ObjectBaseP searchByName(const string& name) const;

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name.
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	virtual vector<string> listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem=5) const;
	
	///////////////////////////////////////////////////////////////////////////
	// Properties setters and getters
	//! Get the maximum level of the geodesic sphere used.
	//! See the class description for a short introduction to the meaning of this value.
	int getMaxGridLevel(void) const {return max_geodesic_grid_level;}
	
	//! Initializes each triangular face of the geodesic grid.
	// TODO: But why?
	void setGrid(class GeodesicGrid* grid);
	
	//! Gets the maximum search level.
	// TODO: add a non-lame description - what is the purpose of the max search level?
	int getMaxSearchLevel(const ToneReproductor *eye, const Projector *prj) const;
	
	//! Sets the time it takes for star names to fade and off.
	//! @param duration the time in seconds.
	void set_names_fade_duration(float duration) {names_fader.set_duration((int) (duration * 1000.f));}
	
	//! Loads common names for stars from a file.
	//! Called when the SkyCulture is updated.
	//! @param the path to a file containing the common names for bright stars.
	int load_common_names(const string& commonNameFile);
	
	//! Loads scientific names for stars from a file.
	//! Called when the SkyCulture is updated.
	//! @param the path to a file containing the scientific names for bright stars.
	void load_sci_names(const string& sciNameFile);
	
	//! Search for the nearest star to some position.
	//! @param Pos the 3d vector representing the direction to search.
	//! @return the nearest star from the specified position, or an 
	//! empty StelObjectP if none were found close by.
	ObjectBaseP search(Vec3d Pos) const;
	
	//! Search for a star by catalogue number (including catalogue prefix).
	//! @param id the catalogue identifier for the required star.
	//! @return the requested StelObjectP or an empty objecy if the requested
	//! one was not found.
	ObjectBaseP search(const string& id) const;
	
	//! Search bu Hipparcos catalogue number.
	//! @param num the Hipparcos catalogue number of the star which is required.
	//! @return the requested StelObjectP or an empty objecy if the requested
	//! one was not found.
	ObjectBaseP searchHP(int num) const;
	
	//! Set the color used to label bright stars.
	void setLabelColor(const Vec3f& c) {label_color = c;}
	
	//! Get the current color used to label bright stars.
	Vec3f getLabelColor(void) const {return label_color;}
	
	// TODO: *Doxygen* what is this? Is it obsolete?
	void setCircleColor(const Vec3f& c) {circle_color = c;}
	Vec3f getCircleColor(void) const {return circle_color;}
	
	//! Set display flag for Stars.
	void setFlagStars(bool b) {
		starsFader=b;
		SettingsState state;
		state.m_state.stars = b;
		SharedData::Instance()->Settings(state);
	}
	
	//! Get display flag for Stars 
	bool getFlagStars(void) const {return starsFader==true;}
	
	//! Set display flag for Star names (labels).
	void setFlagNames(bool b) {
		names_fader=b;
		ObjectsState state;
		state.m_state.star_labels = b;
		SharedData::Instance()->Objects(state);
	}
	//! Get display flag for Star names (labels).
	bool getFlagNames(void) const {return names_fader==true;}
	
	//! Set display flag for Star Scientific names.
	void setFlagStarsSciNames(bool b) {flagStarSciName=b;}
	//! Get display flag for Star Scientific names.
	bool getFlagStarsSciNames(void) const {return flagStarSciName;}
	
	//! Set flag for Star twinkling.
	void setFlagTwinkle(bool b) {flagStarTwinkle=b;}
	//! Get flag for Star twinkling.
	bool getFlagTwinkle(void) const {return flagStarTwinkle;}
	
	//! Set flag for displaying Star as GLpoints (faster on some hardware but not so nice).
	void setFlagPointStar(bool b) {
		flagPointStar=b;
		SettingsState state;
		state.m_state.point_star = b;
		SharedData::Instance()->Settings(state);
	}
	//! Get flag for displaying Star as GLpoints (faster on some hardware but not so nice).
	bool getFlagPointStar(void) const {return flagPointStar;}
	
	//! Set maximum magnitude at which stars names are displayed.
	void setMaxMagName(float b) {
		maxMagStarName=b;
		SettingsState state;
		state.m_state.max_mag_star_name = b;
		SharedData::Instance()->Settings( state );
	}
	//! Get maximum magnitude at which stars names are displayed.
	float getMaxMagName(void) const {return maxMagStarName;} 
	
	//! Set base stars display scaling factor.
	void setScale(float b) {
		starScale=b;
		SettingsState state;
		state.m_state.star_scale = b;
		SharedData::Instance()->Settings(state);
	}
	//! Get base stars display scaling factor.
	float getScale(void) const {return starScale;}
	
	float getStarSizeLimit(void) const {return starSizeLimit;}
	void setStarSizeLimit(float f) {
		starSizeLimit = f;
		SettingsState state;
		state.m_state.star_size_limit = f;
		SharedData::Instance()->Settings(state);
	}

	//! This is the marginal planet size limit set here
	//! for use rendering stars.  Master location is in ssystem. 
	void setObjectSizeLimit(float f) {
		objectSizeLimit = f;
	}

	//! Set stars display scaling factor wrt magnitude.
	void setMagScale(float b) {
		starMagScale=b;
		SettingsState state;
		state.m_state.star_mag_scale = b;
		SharedData::Instance()->Settings(state);
	}
	//! Get base stars display scaling factor wrt magnitude.
	float getMagScale(void) const {return starMagScale;}
	
	//! Set stars twinkle amount.
	void setTwinkleAmount(float b) {
		SettingsState state;
		state.m_state.star_twinkle_amount = b;
		SharedData::Instance()->Settings( state );
		twinkleAmount=b;
	}
	//! Get stars twinkle amount.
	float getTwinkleAmount(void) const {return twinkleAmount;}
	
	
	//! Set MagConverter maximum FOV.
	//! Usually stars/planet halos are drawn fainter when FOV gets larger, 
	//! but when FOV gets larger than this value, the stars do not become
	//! fainter any more. Must be >= 60.0.
	void setMagConverterMaxFov(float x) {mag_converter->setMaxFov(x);}
	
	//! Set MagConverter minimum FOV.
	//! Usually stars/planet halos are drawn brighter when FOV gets smaller.
	//! But when FOV gets smaller than this value, the stars do not become
	//! brighter any more. Must be <= 60.0.
	void setMagConverterMinFov(float x) {mag_converter->setMinFov(x);}
	
	//! Set MagConverter magnitude shift.
	//! draw the stars/planet halos as if they were brighter of fainter
	//! by this amount of magnitude
	void setMagConverterMagShift(float x) {mag_converter->setMagShift(x);}
	
	//! Set MagConverter maximum magnitude.
	//! stars/planet halos, whose original (unshifted) magnitude is greater
	//! than this value will not be drawn.
	void setMagConverterMaxMag(float mag) {mag_converter->setMaxMag(mag);}
	
	//! Set MagConverter maximum scaled magnitude wrt 60 degree FOV.
	//! Stars/planet halos, whose original (unshifted) magnitude is greater
	//! than this value will not be drawn at 60 degree FOV.
	void setMagConverterMaxScaled60DegMag(float mag) {
		SettingsState state;
		state.m_state.star_limiting_mag = mag;
		SharedData::Instance()->Settings( state );
		mag_converter->setMaxScaled60DegMag(mag);
	}
	
	//! Get MagConverter maximum FOV.
	float getMagConverterMaxFov(void) const {return mag_converter->getMaxFov();}
	//! Get MagConverter minimum FOV.
	float getMagConverterMinFov(void) const {return mag_converter->getMinFov();}
	//! Get MagConverter magnitude shift.
	float getMagConverterMagShift(void) const {return mag_converter->getMagShift();}
	//! Get MagConverter maximum magnitude.
	float getMagConverterMaxMag(void) const {return mag_converter->getMaxMag();}
	//! Get MagConverter maximum scaled magnitude wrt 60 degree FOV.
	float getMagConverterMaxScaled60DegMag(void) const {return mag_converter->getMaxScaled60DegMag();}
	
	//! Compute RMag and CMag from magnitude.
	//! Useful for conststent drawing of Planet halos.
	int computeRCMag(float mag, bool point_star, float fov, 
			 const ToneReproductor *eye, float rc_mag[2]) const
	{
		mag_converter->setFov(fov);
		mag_converter->setEye(eye);
		return mag_converter->computeRCMag(mag,point_star,eye,rc_mag);
	}
	
	//! Define font size to use for star names display.
	void setFont(float font_size, const string& font_name);
	
	//! Show scientific or catalog names on stars without common names.
	static void setFlagSciNames(bool f) {flagSciNames = f;}
	static bool getFlagSciNames(void) {return flagSciNames;}
	
	//! Draw a star of specified position, magnitude and color.
	int drawStar(const Projector *prj, const Vec3d &XY,
			const float rc_mag[2], const Vec3f &color) const;
	
	//! Get the (translated) common name for a star with a specified 
	//! Hipparcos catalogue number.
	static string getCommonName(int hip);
	
	//! Get the (translated) scientifc name for a star with a specified 
	//! Hipparcos catalogue number.
	static string getSciName(int hip);

	static Vec3f color_table[128];
	static double getCurrentJDay(void) {return current_JDay;}
	static string convertToSpectralType(int index);
	static string convertToComponentIds(int index);
private:
	//! Load all the stars from the files.
	void load_data(const InitParser &conf, LoadingBar& lb);
	
	LinearFader names_fader;
	LinearFader starsFader;
	
	float starSizeLimit;
	float objectSizeLimit;
	float starScale;
	float starMagScale;
	bool flagStarName;
	bool flagStarSciName;
	float maxMagStarName;
	bool flagStarTwinkle;
	float twinkleAmount;
	bool flagPointStar;
	bool gravityLabel;
	
	s_texture* starTexture; // star texture
	
	int max_geodesic_grid_level;
	int last_max_search_level;
	typedef map<int,BigStarCatalogExtension::ZoneArray*> ZoneArrayMap;
	ZoneArrayMap zone_arrays; // index is the grid level
	static void initTriangleFunc(int lev, int index,
				const Vec3d &c0,
				const Vec3d &c1,
				const Vec3d &c2,
				void *context)
	{
		reinterpret_cast<HipStarMgr*>(context)->initTriangle(lev, index, c0, c1, c2);
	}
	
	void initTriangle(int lev, int index,
			const Vec3d &c0,
			const Vec3d &c1,
			const Vec3d &c2);
	
	BigStarCatalogExtension::HipIndexStruct *hip_index; // array of hiparcos stars
	
	class MagConverter
	{
	public:
		MagConverter(const HipStarMgr &mgr) : mgr(mgr)
		{
			setMaxFov(180.f);
			setMinFov(0.1f);
			setFov(180.f);
			setMagShift(0.f);
			setMaxMag(30.f);
			min_rmag = 0.01f;
		}
		void setMaxFov(float fov) {max_fov = (fov < 60.f) ? 60.f : fov;}
		void setMinFov(float fov) {min_fov = (fov > 60.f) ? 60.f : fov;}
		void setMagShift(float d) {mag_shift = d;}
		void setMaxMag(float mag) {max_mag = mag;}
		void setMaxScaled60DegMag(float mag) {max_scaled_60deg_mag = mag;}
		float getMaxFov(void) const {return max_fov;}
		float getMinFov(void) const {return min_fov;}
		float getMagShift(void) const {return mag_shift;}
		float getMaxMag(void) const {return max_mag;}
		float getMaxScaled60DegMag(void) const {return max_scaled_60deg_mag;}
		void setFov(float fov);
		void setEye(const ToneReproductor *eye);
		int computeRCMag(float mag, bool point_star,
	                 const ToneReproductor *eye, float rc_mag[2]) const;
	private:
		const HipStarMgr &mgr;
		float max_fov, min_fov, mag_shift, max_mag, max_scaled_60deg_mag,
		min_rmag, fov_factor;
	};

	MagConverter *mag_converter;
	
	static map<int, string> common_names_map;
	static map<int, string> common_names_map_i18n;
	static map<string, int> common_names_index;
	static map<string, int> common_names_index_i18n;

	static map<int, string> sci_names_map_i18n;
	static map<string, int> sci_names_index_i18n;
	
	static double current_JDay;
	
	double fontSize;
	s_font* starFont;
	static bool flagSciNames;
	Vec3f label_color, circle_color;
	float twinkle_amount;
	
	s_texture* texPointer;		// The selection pointer texture
};


#endif // _STAR_MGR_H_
