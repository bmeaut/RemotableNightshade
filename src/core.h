/*
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

#ifndef _CORE_H_
#define _CORE_H_

// default font sizes
#define FontSizeCardinalPoints 30.
#define FontSizeSolarSystem 14.
#define FontSizeGeneral 12.
#define FontSizeConstellations 16.

#include <string>

#include "solarsystem.h"
#include "navigator.h"
#include "observer.h"
#include "projector.h"
#include "object.h"
#include "constellation_mgr.h"
#include "nebula_mgr.h"
#include "atmosphere.h"
#include "tone_reproductor.h"
#include "utility.h"
#include "init_parser.h"
#include "draw.h"
#include "landscape.h"
#include "meteor_mgr.h"
#include "sky_localizer.h"
#include "loadingbar.h"
#include "image_mgr.h"
#include "callbacks.hpp"
#include "geodesic_grid.h"

//!  @brief Main class for application core processing.
//!
//! Manage all the objects to be used in the program.
//! This class is the main API of the program. It must be documented using doxygen.
class Core
{
public:

	NebulaMgr& getNebulaManager() { return *nebulas; }
	Object& getSelectedObject() { return selected_object; }


	//! Possible mount modes
	enum MOUNT_MODE { MOUNT_ALTAZIMUTAL, MOUNT_EQUATORIAL };

	//! Change Milkyway texture
	void milkyswap(string mdir);

	// Inputs are the locale directory and root directory and callback function for recording actions
	Core(const string& LDIR, const string& DATA_ROOT, const boost::callback <void, string> & recordCallback);
	virtual ~Core();

	//! Init and load all main core components from the passed config file.
	void init(const InitParser& conf, const int viewW, const int viewH);

	//! Update all the objects with respect to the time.
	//! @param delta_time the time increment in ms.
	void update(int delta_time);

	//! Execute all the drawing functions
	//! @param delta_time the time increment in ms.
	// Returns the max squared distance in pixels any single object has
	// moved since the previous update.
	double draw(int delta_time);

	//! Get the name of the directory containing the data
	const string getDataDir(void) const {
		return dataRoot + "/data/";
	}

	//! Get the name of the local script directory
	const string getScriptDir(void) const {
		return dataRoot + "/data/scripts/";
	}

	//! Get the name of the directory containing the data
	const string getLocaleDir(void) const {
		return localeDir;
	}

	//! Get the name of the root directory i.e the one containing the other main directories
	const string& getDataRoot() const {
		return dataRoot;
	}

	//! Set the sky culture from I18 name
	//! Returns false and doesn't change if skyculture is invalid
	bool setSkyCulture(const string& cultureName);

	//! Set the current sky culture from the passed directory
	bool setSkyCultureDir(const string& culturedir);

	string getSkyCultureDir() {
		return skyCultureDir;
	}

	//! Get the current sky culture I18 name
	string getSkyCulture() const {
		return skyloc->directoryToSkyCultureI18(skyCultureDir);
	}

	//! Get the I18 available sky culture names
	string getSkyCultureListI18() const {
		return skyloc->getSkyCultureListI18();
	}

	string getSkyCultureHash() const {
		return skyloc->getSkyCultureHash();
	}

	bool loadSkyCulture(const string& culturePath);

	//! Set the landscape
	bool setLandscape(const string& new_landscape_name);

	//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
	//! and make it the current landscape
	bool loadLandscape(stringHash_t& param);

	//! @brief Set the sky language and reload the sky objects names with the new translation
	//! This function has no permanent effect on the global locale
	//!@param newSkyLocaleName The name of the locale (e.g fr) to use for sky object labels
	void setSkyLanguage(const string& newSkyLocaleName);

	//! Get the current sky language used for sky object labels
	//! @return The name of the locale (e.g fr)
	string getSkyLanguage() {
		return skyTranslator.getLocaleName();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Navigation
	//! Set time speed in JDay/sec
	void setTimeSpeed(double ts) {
		navigation->set_time_speed(ts);
	}
	//! Get time speed in JDay/sec
	double getTimeSpeed(void) const {
		return navigation->get_time_speed();
	}

	//! Set simulation time to current real world time
	void setTimeNow();
	//! Get wether the current simulation time is the real world time
	bool getIsTimeNow(void) const;


	//! Set the current date in Julian Day
	void setJDay(double JD) {
		navigation->set_JDay(JD);
	}
	//! Get the current date in Julian Day
	double getJDay(void) const {
		return navigation->get_JDay();
	}

	//! Set object tracking
	void setFlagTracking(bool b);
	//! Get object tracking
	bool getFlagTracking(void) {
		return navigation->get_flag_traking();
	}

	//! Set whether sky position is to be locked
	void setFlagLockSkyPosition(bool b) {
		navigation->set_flag_lock_equ_pos(b);
	}
	//! Set whether sky position is locked
	bool getFlagLockSkyPosition(void) {
		return navigation->get_flag_lock_equ_pos();
	}

	//! Set current mount type
	void setMountMode(MOUNT_MODE m) {
		navigation->set_viewing_mode((m==MOUNT_ALTAZIMUTAL) ? Navigator::VIEW_HORIZON : Navigator::VIEW_EQUATOR);
	}
	//! Get current mount type
	MOUNT_MODE getMountMode(void) {
		return ((navigation->get_viewing_mode()==Navigator::VIEW_HORIZON) ? MOUNT_ALTAZIMUTAL : MOUNT_EQUATORIAL);
	}
	//! Toggle current mount mode between equatorial and altazimutal
	void toggleMountMode(void) {
		if (getMountMode()==MOUNT_ALTAZIMUTAL) setMountMode(MOUNT_EQUATORIAL);
		else setMountMode(MOUNT_ALTAZIMUTAL);
	}

	// TODO!
	void loadObservatory();

	//! Go to the selected object
	void gotoSelectedObject(void) {
		if (selected_object)
			navigation->move_to(
			    selected_object.get_earth_equ_pos(navigation),
			    auto_move_duration);
	}

	//! Move view in alt/az (or equatorial if in that mode) coordinates
	void panView(double delta_az, double delta_alt)	{
		setFlagTracking(0);
		navigation->update_move(projection, delta_az, delta_alt, projection->get_fov());
	}


	void setViewOffset(double offset);
	double getViewOffset();
	void setHeading(double heading, int duration);
	double getHeading();

	//! Set flight duration in seconds between bodies
	void setFlightDuration(float f) {
		flight_duration = f;
		SettingsState state;
		state.m_state.flyto_duration = f;
		SharedData::Instance()->Settings( state );
	}
	//! Get flight duration in seconds
	float getFlightDuration(void) const {
		return flight_duration;
	}


	//! Set automove duration in seconds
	void setAutomoveDuration(float f) {
		auto_move_duration = f;
		SettingsState state;
		state.m_state.auto_move_duration = f;
		SharedData::Instance()->Settings( state );
	}
	//! Get automove duration in seconds
	float getAutomoveDuration(void) const {
		return auto_move_duration;
	}

	//! Zoom to the given FOV (in degree)
	void zoomTo(double aim_fov, float move_duration = 1.) {
		projection->zoom_to(aim_fov, move_duration);
	}

	//! Get current FOV (in degree)
	float getFov(void) const {
		return projection->get_fov();
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov(void) const {
		return projection->getAimFov();
	}

	//! Set the current FOV (in degree)
	void setFov(double f) {
		projection->set_fov(f);
	}

	//! Set the maximum FOV (in degree)
	void setMaxFov(double f) {
		projection->setMaxFov(f);
	}

	//! Go and zoom temporarily to the selected object.
	void autoZoomIn(float move_duration = 1.f, bool allow_manual_zoom = 1);

	//! Unzoom to the previous position
	void autoZoomOut(float move_duration = 1.f, bool full = 0, bool allow_manual_zoom = 0);

	//! Set whether auto zoom can go further than normal
	void setFlagManualAutoZoom(bool b) {
		FlagManualZoom = b;
		SettingsState state;
		state.m_state.manual_zoom = b;
		SharedData::Instance()->Settings( state );

		ObjectsState ostate;
		ostate.m_state.manual_zoom = b;
		SharedData::Instance()->Objects( ostate );
	}
	//! Get whether auto zoom can go further than normal
	bool getFlagManualAutoZoom(void) {
		return FlagManualZoom;
	}

	// Viewing direction function : 1 move, 0 stop.
	void turn_right(int);
	void turn_left(int);
	void turn_up(int);
	void turn_down(int);
	void zoom_in(int);
	void zoom_out(int);

	//! Make the first screen position correspond to the second (useful for mouse dragging)
	void dragView(int x1, int y1, int x2, int y2);

	//! Find and select an object near given equatorial position
	//! @return true if a object was found at position (this does not necessarily means it is selected)
	bool findAndSelect(const Vec3d& pos);

	//! Find and select an object near given screen position
	//! @return true if a object was found at position (this does not necessarily means it is selected)
	bool findAndSelect(int x, int y);

	//! Find and select an object from its translated name
	//! @param nameI18n the case sensitive object translated name
	//! @return true if a object was found with the passed name
	bool findAndSelectI18n(const string &nameI18n);

	//! Find and select an object based on selection type and standard name or number
	//! @return true if an object was selected
	bool selectObject(const string &type, const string &id);


	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	vector<string> listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem=5) const;

	//! Return whether an object is currently selected
	bool getFlagHasSelected(void) {
		return selected_object;
	}

	//! Deselect selected object if any
	//! Does not deselect selected constellation
	void unSelect(void) {
		selected_object=NULL;
		//asterisms->setSelected(NULL);
		ssystem->setSelected(Object());
		// do not:  setFlagTracking(0);  // do not put in svn 200703
		// stay looking at same area of sky
	}

	void unsetSelectedConstellation(string constellation) {
		asterisms->unsetSelected(constellation);
	}

	void deselect(void);

	//! Set whether a pointer is to be drawn over selected object
	void setFlagSelectedObjectPointer(bool b) {
		object_pointer_visibility = b;
	}

	string getSelectedPlanetEnglishName() const;

	//! Get a multiline string describing the currently selected object
	string getSelectedObjectInfo(void) const {
		return selected_object.getInfoString(navigation);
	}

	//! Get a 1 line string briefly describing the currently selected object
	string getSelectedObjectShortInfo(void) const {
		return selected_object.getShortInfoString(navigation);
	}

	//! Get a color used to display info about the currently selected object
	Vec3f getSelectedObjectInfoColor(void) const;


	///////////////////////////////////////////////////////////////////////////////////////
	// Rendering settings

	//! Set rendering flag of antialiased lines
	void setFlagAntialiasLines(bool b) {
		FlagAntialiasLines = b;
		SettingsState state;
		state.m_state.antialias_lines = b;
		SharedData::Instance()->Settings( state );

		if(b) glEnable(GL_LINE_SMOOTH);
		else glDisable(GL_LINE_SMOOTH);
	}
	//! Get display flag of constellation lines
	bool getFlagAntialiasLines(void) {
		return FlagAntialiasLines;
	}

	void setLineWidth(float w) {
		m_lineWidth = w;
		SettingsState state;
		state.m_state.line_width = w;
		SharedData::Instance()->Settings( state );
	}
	float getLineWidth() {
		return m_lineWidth;
	}

	//! Enable shaders
	void setFlagShaders(bool b) {
		FlagShaders = b;
		ssystem->setFlagShaders(b);
	}
	//! Get flag of shaders
	bool getFlagShaders(void) {
		return FlagShaders;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Constellations methods
	//! Set display flag of constellation lines
	void setFlagConstellationLines(bool b) {
		asterisms->setFlagLines(b);
	}
	//! Get display flag of constellation lines
	bool getFlagConstellationLines(void) {
		return asterisms->getFlagLines();
	}

	//! Set display flag of constellation art
	void setFlagConstellationArt(bool b) {
		asterisms->setFlagArt(b);
	}
	//! Get display flag of constellation art
	bool getFlagConstellationArt(void) {
		return asterisms->getFlagArt();
	}

	//! Set display flag of constellation names
	void setFlagConstellationNames(bool b) {
		asterisms->setFlagNames(b);
	}
	//! Get display flag of constellation names
	bool getFlagConstellationNames(void) {
		return asterisms->getFlagNames();
	}

	//! Set display flag of constellation boundaries
	void setFlagConstellationBoundaries(bool b) {
		asterisms->setFlagBoundaries(b);
	}
	//! Get display flag of constellation boundaries
	bool getFlagConstellationBoundaries(void) {
		return asterisms->getFlagBoundaries();
	}
	Vec3f getColorConstellationBoundaries(void) const {
		return asterisms->getBoundaryColor();
	}

	//! Set constellation art intensity
	void setConstellationArtIntensity(float f) {
		asterisms->setArtIntensity(f);
	}
	//! Get constellation art intensity
	float getConstellationArtIntensity(void) const {
		return asterisms->getArtIntensity();
	}

	//! Set constellation art intensity
	void setConstellationArtFadeDuration(float f) {
		asterisms->setArtFadeDuration(f);
	}
	//! Get constellation art intensity
	float getConstellationArtFadeDuration(void) const {
		return asterisms->getArtFadeDuration();
	}

	//! Set whether selected constellation is drawn alone
	void setFlagConstellationIsolateSelected(bool b) {
		asterisms->setFlagIsolateSelected(b);
	}
	//! Get whether selected constellation is drawn alone
	bool getFlagConstellationIsolateSelected(void) {
		return asterisms->getFlagIsolateSelected();
	}

	//! Get constellation line color
	Vec3f getColorConstellationLine() const {
		return asterisms->getLineColor();
	}
	//! Set constellation line color
	void setColorConstellationLine(const Vec3f& v) {
		asterisms->setLineColor(v);
	}

	//! Get constellation names color
	Vec3f getColorConstellationNames() const {
		return asterisms->getLabelColor();
	}
	//! Set constellation names color
	void setColorConstellationNames(const Vec3f& v) {
		asterisms->setLabelColor(v);
	}

	//! Get constellation art color
	Vec3f getColorConstellationArt() const {
		return asterisms->getArtColor();
	}
	//! Set constellation line color
	void setColorConstellationArt(const Vec3f& v) {
		asterisms->setArtColor(v);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Stars methods
	//! Set display flag for Stars
	void setFlagStars(bool b);
	//! Get display flag for Stars
	bool getFlagStars(void) const;

	//! Set display flag for Star names. Also make sure that stars are on if want labels
	void setFlagStarName(bool b);
	//! Get display flag for Star names
	bool getFlagStarName(void) const;

	//! Set display flag for Star Scientific names
	void setFlagStarSciName(bool b);
	//! Get display flag for Star Scientific names
	bool getFlagStarSciName(void) const;

	//! Set flag for Star twinkling
	void setFlagStarTwinkle(bool b);
	//! Get flag for Star twinkling
	bool getFlagStarTwinkle(void) const;

	//! Set flag for displaying Star as GLpoints (faster but not so nice)
	void setFlagPointStar(bool b);
	//! Get flag for displaying Star as GLpoints (faster but not so nice)
	bool getFlagPointStar(void) const;

	//! Set maximum magnitude at which stars names are displayed
	void setMaxMagStarName(float f);
	//! Get maximum magnitude at which stars names are displayed
	float getMaxMagStarName(void) const;

	//! Set maximum magnitude at which stars scientific names are displayed
	void setMaxMagStarSciName(float f);
	//! Get maximum magnitude at which stars scientific names are displayed
	float getMaxMagStarSciName(void) const;

	//! Set base stars display scaling factor
	void setStarScale(float f);
	//! Get base stars display scaling factor
	float getStarScale(void) const;

	//! Set base planets display scaling factor
	//! This is additive to star size limit above
	//! since makes no sense to be less
	//! ONLY SET THROUGH THIS METHOD
	void setPlanetsSizeLimit(float f);

	//! Get base planets display scaling factor
	float getPlanetsSizeLimit(void) const {
		return (ssystem->getSizeLimit()-getStarSizeLimit());
	}

	float getStarSizeLimit(void) const;
	void setStarSizeLimit(float);

	float getStarLimitingMag(void) const;
	void setStarLimitingMag(float);

	//! Set stars display scaling factor wrt magnitude
	void setStarMagScale(float f);
	//! Get base stars display scaling factor wrt magnitude
	float getStarMagScale(void) const;

	//! Set stars twinkle amount
	void setStarTwinkleAmount(float f);
	//! Get stars twinkle amount
	float getStarTwinkleAmount(void) const;

	float getMagConverterMagShift(void) const;
	void setMagConverterMagShift(float f);
	float getMagConverterMaxScaled60DegMag(void) const;
	void setMagConverterMaxScaled60DegMag(float f);
	float getMagConverterMaxFov(void) const;
	void setMagConverterMaxFov(float f);
	float getMagConverterMinFov(void) const;
	void setMagConverterMinFov(float f);
	float getMagConverterMaxMag(void) const;
	void setMagConverterMaxMag(float f);

	Vec3f getColorStarNames(void) const;
	Vec3f getColorStarCircles(void) const;
	void setColorStarNames(const Vec3f &v);
	void setColorStarCircles(const Vec3f &v);

	///////////////////////////////////////////////////////////////////////////////////////
	// Planets flags
	//! Set flag for displaying Planets
	void setFlagPlanets(bool b) {
		ssystem->setFlagPlanets(b);
	}
	//! Get flag for displaying Planets
	bool getFlagPlanets(void) const {
		return ssystem->getFlagPlanets();
	}

	//! Set flag for displaying Planets Trails
	void setFlagPlanetsTrails(bool b) {
		ssystem->setFlagTrails(b);
	}
	//! Get flag for displaying Planets Trails
	bool getFlagPlanetsTrails(void) const {
		return ssystem->getFlagTrails();
	}

	//! Set flag for displaying Planets Hints
	void setFlagPlanetsHints(bool b) {
		ssystem->setFlagHints(b);
	}
	//! Get flag for displaying Planets Hints
	bool getFlagPlanetsHints(void) const {
		return ssystem->getFlagHints();
	}

	//! Set flag for displaying Planets Orbits
	void setFlagPlanetsOrbits(bool b) {
		ssystem->setFlagOrbits(b);
	}
	//! Get flag for displaying Planets Orbits
	bool getFlagPlanetsOrbits(void) const {
		return ssystem->getFlagOrbits();
	}

	void setFlagLightTravelTime(bool b) {
		ssystem->setFlagLightTravelTime(b);
	}
	bool getFlagLightTravelTime(void) const {
		return ssystem->getFlagLightTravelTime();
	}

	Vec3f getColorPlanetsOrbits(void) const {
		return ssystem->getPlanetOrbitColor();
	}
	Vec3f getColorSatelliteOrbits(void) const {
		return ssystem->getSatelliteOrbitColor();
	}
	Vec3f getColorPlanetsNames(void) const {
		return ssystem->getLabelColor();
	}

	//! Start/stop displaying planets Trails
	void startPlanetsTrails(bool b) {
		ssystem->startTrails(b);
	}
	Vec3f getColorPlanetsTrails(void) const {
		return ssystem->getTrailColor();
	}

	//! Set base planets display scaling factor
	void setPlanetsScale(float f) {
		ssystem->setScale(f);
	}
	//! Get base planets display scaling factor
	float getPlanetsScale(void) const {
		return ssystem->getScale();
	}



	//! Set selected planets by englishName
	//! @param englishName The planet name or "" to select no planet
	void setPlanetsSelected(const string& englishName) {
		ssystem->setSelected(englishName);
	}

	string getPlanetHashString(void);

	bool setHomePlanet(string planet, float transit_time=0.f);

	//! Set flag for displaying a scaled Moon
	void setFlagMoonScaled(bool b) {
		ssystem->setFlagMoonScale(b);
	}
	//! Get flag for displaying a scaled Moon
	bool getFlagMoonScaled(void) const {
		return ssystem->getFlagMoonScale();
	}

	//! Set Moon scale
	void setMoonScale(float f) {
		if (f<0) ssystem->setMoonScale(1.);
		else ssystem->setMoonScale(f);
	}
	//! Get Moon scale
	float getMoonScale(void) const {
		return ssystem->getMoonScale();
	}

	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b) {
		ssystem->setFlagClouds(b);
	}
	//! Get flag for displaying Atmosphere
	bool getFlagClouds(void) const {
		return ssystem->getFlagClouds();
	}


// for adding planets
	string addSolarSystemBody(stringHash_t& param);

	string removeSolarSystemBody(string name);

	string removeSupplementalSolarSystemBodies();

	///////////////////////////////////////////////////////////////////////////////////////
	// Grid and lines
	//! Set flag for displaying Azimutal Grid
	void setFlagAzimutalGrid(bool b) {
		azi_grid->setFlagshow(b);
	}
	//! Get flag for displaying Azimutal Grid
	bool getFlagAzimutalGrid(void) const {
		return azi_grid->getFlagshow();
	}
	Vec3f getColorAzimutalGrid(void) const {
		return azi_grid->getColor();
	}

	//! Set flag for displaying Equatorial Grid
	void setFlagEquatorGrid(bool b) {
		equ_grid->setFlagshow(b);
	}
	//! Get flag for displaying Equatorial Grid
	bool getFlagEquatorGrid(void) const {
		return equ_grid->getFlagshow();
	}
	Vec3f getColorEquatorGrid(void) const {
		return equ_grid->getColor();
	}

	void setFlagGalacticGrid(bool b) {
		gal_grid->setFlagshow(b);
	}
	//! Get flag for displaying Galactic Grid
	bool getFlagGalacticGrid(void) const {
		return gal_grid->getFlagshow();
	}
	Vec3f getColorGalacticGrid(void) const {
		return gal_grid->getColor();
	}


	//! Set flag for displaying Equatorial Line
	void setFlagEquatorLine(bool b) {
		equator_line->setFlagshow(b);
	}
	//! Get flag for displaying Equatorial Line
	bool getFlagEquatorLine(void) const {
		return equator_line->getFlagshow();
	}
	Vec3f getColorEquatorLine(void) const {
		return equator_line->getColor();
	}

	//! Set flag for displaying Ecliptic Line
	void setFlagEclipticLine(bool b) {
		ecliptic_line->setFlagshow(b);
	}
	//! Get flag for displaying Ecliptic Line
	bool getFlagEclipticLine(void) const {
		return ecliptic_line->getFlagshow();
	}
	Vec3f getColorEclipticLine(void) const {
		return ecliptic_line->getColor();
	}

	//! Set flag for displaying Precession Circle
	void setFlagPrecessionCircle(bool b) {
		precession_circle->setFlagshow(b);
	}
	//! Get flag for displaying Precession Circle
	bool getFlagPrecessionCircle(void) const {
		return precession_circle->getFlagshow();
	}
	Vec3f getColorPrecessionCircle(void) const {
		return precession_circle->getColor();
	}

	//! Set flag for displaying Circumpolar Circle
	void setFlagCircumpolarCircle(bool b) {
		circumpolar_circle->setFlagshow(b);
	}
	//! Get flag for displaying Circumpolar Circle
	bool getFlagCircumpolarCircle(void) const {
		return circumpolar_circle->getFlagshow();
	}
	Vec3f getColorCircumpolarCircle(void) const {
		return circumpolar_circle->getColor();
	}

	//! Set flag for displaying Tropic Lines
	void setFlagTropicLines(bool b) {
		tropic_line->setFlagshow(b);
	}
	//! Get flag for displaying Tropic Lines
	bool getFlagTropicLines(void) const {
		return tropic_line->getFlagshow();
	}
	Vec3f getColorTropicLine(void) const {
		return tropic_line->getColor();
	}

	//! Set flag for displaying Meridian Line
	void setFlagMeridianLine(bool b) {
		meridian_line->setFlagshow(b);
	}
	//! Get flag for displaying Meridian Line
	bool getFlagMeridianLine(void) const {
		return meridian_line->getFlagshow();
	}
	Vec3f getColorMeridianLine(void) const {
		return meridian_line->getColor();
	}

	//! Set flag for displaying Cardinals Points
	void setFlagCardinalsPoints(bool b) {
		cardinals_points->setFlagShow(b);
	}
	//! Get flag for displaying Cardinals Points
	bool getFlagCardinalsPoints(void) const {
		return cardinals_points->getFlagShow();
	}

	//! Set Cardinals Points color
	void setColorCardinalPoints(const Vec3f& v) {
		cardinals_points->setColor(v);
	}
	//! Get Cardinals Points color
	Vec3f getColorCardinalPoints(void) const {
		return cardinals_points->get_color();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Colors

	void setColorConstellationBoundaries(const Vec3f& v) {
		asterisms->setBoundaryColor(v);
	}
	void setColorPlanetsOrbits(const Vec3f& v) {
		ssystem->setPlanetOrbitColor(v);
	}
	void setColorSatelliteOrbits(const Vec3f& v) {
		ssystem->setSatelliteOrbitColor(v);
	}
	void setColorPlanetsNames(const Vec3f& v) {
		ssystem->setLabelColor(v);
	}
	void setColorPlanetsTrails(const Vec3f& v) {
		ssystem->setTrailColor(v);
	}
	void setColorAzimutalGrid(const Vec3f& v) {
		azi_grid->setColor(v);
	}
	void setColorGalacticGrid(const Vec3f& v) {
		gal_grid->setColor(v);
	}
	void setColorEquatorGrid(const Vec3f& v) {
		equ_grid->setColor(v);
	}
	void setColorEquatorLine(const Vec3f& v) {
		equator_line->setColor(v);
		tropic_line->setColor(v);
	}
	void setColorEclipticLine(const Vec3f& v) {
		ecliptic_line->setColor(v);
	}
	void setColorPrecessionCircle(const Vec3f& v) {
		precession_circle->setColor(v);
	}
	void setColorCircumpolarCircle(const Vec3f& v) {
		circumpolar_circle->setColor(v);
	}
	void setColorTropicLine(const Vec3f& v) {
		tropic_line->setColor(v);
	}
	void setColorMeridianLine(const Vec3f& v) {
		meridian_line->setColor(v);
	}
	void setColorNebulaLabels(const Vec3f& v) {
		nebulas->setLabelColor(v);
	}
	void setColorNebulaCircle(const Vec3f& v) {
		nebulas->setCircleColor(v);
	}

	bool loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
					string name, string filename, string credit, double texture_luminance_adjust,
					double distance);

	// remove one nebula added by user
	string removeNebula(const string& name);

	// remove all user added nebulae
	string removeSupplementalNebulae();

	///////////////////////////////////////////////////////////////////////////////////////
	// Projection
	//! Set the horizontal viewport offset in pixels
	void setViewportHorizontalOffset(int hoff) const {
		projection->setViewportPosX(hoff);
	}
	//! Get the horizontal viewport offset in pixels
	int getViewportHorizontalOffset(void) const {
		return projection->getViewportPosX();
	}

	//! Set the vertical viewport offset in pixels
	void setViewportVerticalOffset(int voff) const {
		projection->setViewportPosY(voff);
	}
	//! Get the vertical viewport offset in pixels
	int getViewportVerticalOffset(void) const {
		return projection->getViewportPosY();
	}

	//! Maximize viewport according to passed screen values
	void setMaximizedViewport(int screenW, int screenH) {
		projection->setViewport(0, 0, screenW, screenH);
	}

	//! Set a centered squared viewport with passed vertical and horizontal offset
	void setSquareViewport(int screenW, int screenH, int hoffset, int voffset) {
		int m = MY_MIN(screenW, screenH);
		projection->setViewport((screenW-m)/2+hoffset, (screenH-m)/2+voffset, m, m);
	}

	//! Set whether a disk mask must be drawn over the viewport
	void setViewportMaskDisk(void) {
		projection->setMaskType(Projector::DISK);
	}

	//! also set disk viewport values at same time
	void setViewportMaskDisk(int cx, int cy, int screenW, int screenH, int radius) {
		projection->setMaskType(Projector::DISK);
		projection->setDiskViewport(cx, cy, screenW, screenH, radius);
	}

	void setCenterHorizontalOffset(int h) {
		projection->setCenterHorizontalOffset(h);
	}
	int getCenterHorizontalOffset() {
		return projection->getCenterHorizontalOffset();
	}

	//! Get whether a disk mask must be drawn over the viewport
	bool getViewportMaskDisk(void) const {
		return projection->getMaskType()==Projector::DISK;
	}

	//! Set whether no mask must be drawn over the viewport
	void setViewportMaskNone(void) {
		projection->setMaskType(Projector::NONE);
	}

// for gravity ui
	Vec3d getViewportCenter(void) {
		return projection->getViewportCenter();
	}

	//! Set the projection type
	void setProjectionType(const string& ptype);
	//! Get the projection type
	string getProjectionType(void) const {
		return Projector::typeToString(projection->getType());
	}

	void setProjectorConfiguration( int configuration ) {
		projection->setProjectorConfiguration(configuration);
	}

	// can the projector be configured?
	bool projectorConfigurationSupported(void) {
		return projection->projectorConfigurationSupported();
	}

	int getProjectorConfiguration() {
		return projection->getProjectorConfiguration();
	}


	// What lens is being used?
	int getLens() {
		return projection->getLens();
	}

	void setProjectorShearHorz( double shear ) {
		projection->setShearHorz(shear);
	}

	double getProjectorShearHorz() {
		return projection->getShearHorz();
	}

	void setProjectorOffset( double x, double y ) {
		projection->setProjectionOffset(x, y);
	}

	double getProjectorOffsetX() {
		return projection->getOffsetX();
	}

	double getProjectorOffsetY() {
		return projection->getOffsetY();
	}


	//! get/set horizontal/vertical image flip
	bool getFlipHorz(void) const {
		return projection->getFlipHorz();
	}
	bool getFlipVert(void) const {
		return projection->getFlipVert();
	}
	void setFlipHorz(bool flip);
	void setFlipVert(bool flip);

	//! Set flag for enabling gravity labels
	void setFlagGravityLabels(bool b) {
		projection->setFlagGravityLabels(b);
	}
	//! Get flag for enabling gravity labels
	bool getFlagGravityLabels(void) const {
		return projection->getFlagGravityLabels();
	}

	//! Get display width
	int getDisplayWidth(void) const {
		return projection->getDisplayWidth();
	}

	//! Get display height
	int getDisplayHeight(void) const {
		return projection->getDisplayHeight();
	}


	//! Get viewport width
	int getViewportWidth(void) const {
		return projection->getViewportWidth();
	}

	//! Get viewport height
	int getViewportHeight(void) const {
		return projection->getViewportHeight();
	}

	//! Get viewport X position
	int getViewportPosX(void) const {
		return projection->getViewportPosX();
	}

	//! Get viewport Y position
	int getViewportPosY(void) const {
		return projection->getViewportPosY();
	}

	//! Set the viewport width and height
	void setViewportSize(int w, int h);

	//! Print the passed string so that it is oriented in the drection of the gravity
	void printGravity(s_font* font, float altitude, float azimuth, const string& str, bool outline = 1,
					  int justify = 0, bool cache = 0) const {
		font->print_gravity(projection, altitude, azimuth, str, justify, cache, outline, 0, 0);

	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Landscape
	//! Set flag for displaying Landscape
	void setFlagLandscape(bool b) {
		landscape->setFlagShow(b);
	}
	//! Get flag for displaying Landscape
	bool getFlagLandscape(void) const {
		return landscape->getFlagShow();
	}

	//! Set flag for displaying Fog
	void setFlagFog(bool b) {
		landscape->setFlagShowFog(b);
	}
	//! Get flag for displaying Fog
	bool getFlagFog(void) const {
		return landscape->getFlagShowFog();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Atmosphere
	//! Set flag for displaying Atmosphere
	void setFlagAtmosphere(bool b) {
		atmosphere->setFlagShow(b);
	}
	//! Get flag for displaying Atmosphere
	bool getFlagAtmosphere(void) const {
		return atmosphere->getFlagShow();
	}

	//! Set atmosphere fade duration in s
	void setAtmosphereFadeDuration(float f) {
		atmosphere->setFadeDuration(f);
	}
	//! Get atmosphere fade duration in s
	float getAtmosphereFadeDuration(void) const {
		return atmosphere->getFadeDuration();
	}

	//! Set light pollution limiting magnitude (naked eye)
	void setLightPollutionLimitingMagnitude(float mag) {
		lightPollutionLimitingMagnitude = mag;
		SettingsState state;
		state.m_state.light_pollution_luminance = mag;
		SharedData::Instance()->Settings( state );
		float ln = log(mag);
		float lum = 30.0842967491175 -19.9408790405749*ln +2.12969160094949*ln*ln - .2206;
		atmosphere->setLightPollutionLuminance(lum);
	}
	//! Get light pollution limiting magnitude
	float getLightPollutionLimitingMagnitude(void) const {
		return lightPollutionLimitingMagnitude;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Milky Way
	//! Set flag for displaying Milky Way
	void setFlagMilkyWay(bool b) {
		milky_way->setFlagShow(b);
	}
	//! Get flag for displaying Milky Way
	bool getFlagMilkyWay(void) const {
		return milky_way->getFlagShow();
	}

	//! Set Milky Way intensity
	void setMilkyWayIntensity(float f) {
		milky_way->set_intensity(f);
	}
	//! Get Milky Way intensity
	float getMilkyWayIntensity(void) const {
		return milky_way->get_intensity();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Nebulae
	//! Set flag for displaying Nebulae
	void setFlagNebula(bool b) {
		nebulas->setFlagShow(b);
	}
	//! Get flag for displaying Nebulae
	bool getFlagNebula(void) const {
		return nebulas->getFlagShow();
	}

	//! Set flag for displaying Nebulae Hints
	void setFlagNebulaHints(bool b) {
		nebulas->setFlagHints(b);
	}
	//! Get flag for displaying Nebulae Hints
	bool getFlagNebulaHints(void) const {
		return nebulas->getFlagHints();
	}

	//! Set Nebulae Hints circle scale
	void setNebulaCircleScale(float f) {
		nebulas->setNebulaCircleScale(f);
	}
	//! Get Nebulae Hints circle scale
	float getNebulaCircleScale(void) const {
		return nebulas->getNebulaCircleScale();
	}

	//! Set flag for displaying Nebulae as bright
	void setFlagBrightNebulae(bool b) {
		nebulas->setFlagBright(b);
	}
	//! Get flag for displaying Nebulae as brigth
	bool getFlagBrightNebulae(void) const {
		return nebulas->getFlagBright();
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void setNebulaMaxMagHints(float f) {
		nebulas->setMaxMagHints(f);
	}
	//! Get maximum magnitude at which nebulae hints are displayed
	float getNebulaMaxMagHints(void) const {
		return nebulas->getMaxMagHints();
	}

	//! Set flag for displaying Nebulae even without textures
	void setFlagNebulaDisplayNoTexture(bool b) {
		nebulas->setFlagDisplayNoTexture(b);
	}
	//! Get flag for displaying Nebulae without textures
	bool getFlagNebulaDisplayNoTexture(void) const {
		return nebulas->getFlagDisplayNoTexture();
	}

	Vec3f getColorNebulaLabels(void) const {
		return nebulas->getLabelColor();
	}
	Vec3f getColorNebulaCircle(void) const {
		return nebulas->getCircleColor();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Observer
	//! Return the current observatory (as a const object)
// made non const so can track when save data!  Hmmm. 20070215
	// TODO resolve issue
	Observer* getObservatory(void) {
		return observatory;
	}

	//! Move to a new latitude and longitude on home planet
	void moveObserver(double lat, double lon, double alt, int delay, const string& name) {
		observatory->move_to(lat, lon, alt, delay, name);
	}

	//! Set Meteor Rate in number per hour
	void setMeteorsRate(int f) {
		meteors->set_ZHR(f);
	}
	//! Get Meteor Rate in number per hour
	int getMeteorsRate(void) const {
		return meteors->get_ZHR();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Others
	//! Load color scheme from the given ini file and section name
	void setColorScheme(const string& skinFile, const string& section);

	double getZoomSpeed() {
		return zoom_speed;
	}
	float getAutoMoveDuration() {
		return auto_move_duration;
	}

	void getFontForLocale(const string &_locale, string &_fontFile, float &_fontScale,
	                      string &_fixedFontFile, float &_fixedFontScale);

	string getLandscapeName(void) {
		return landscape->getName();
	}
	string getLandscapeAuthorName(void) {
		return landscape->getAuthorName();
	}
	string getLandscapeDescription(void) {
		return landscape->getDescription();
	}

	//! Get the current navigation (manages frame transformation) used in the core
	Navigator* getNavigation() {
		return navigation;
	}

	bool observerAboveHomePlanet() {
		return aboveHomePlanet;
	}

	//! Get the shared instance of GeodesicGrid
	GeodesicGrid* getGeodesicGrid() {return geodesic_grid;}
	//! Get the shared instance of GeodesicGrid
	const GeodesicGrid* getGeodesicGrid() const {return geodesic_grid;}

private:

	//! Callback to record actions
	boost::callback<void, string> recordActionCallback;

	//! Select passed object
	//! @return true if the object was selected (false if the same was already selected)
	bool selectObject(const Object &obj);

	//! Find any kind of object by the name
	Object searchByNameI18n(const string &name) const;

	//! Find in a "clever" way an object from its equatorial position
	Object clever_find(const Vec3d& pos) const;

	//! Find in a "clever" way an object from its screen position
	Object clever_find(int x, int y) const;

	string baseFontFile;				// The font file used by default during initialization

	string dataRoot;					// The root directory where the data is
	string localeDir;					// The directory containing the translation .mo file
	string skyCultureDir;				// The directory containing data for the culture used for constellations, etc..
	Translator skyTranslator;			// The translator used for astronomical object naming

	// Main elements of the program
	Navigator * navigation;				// Manage all navigation parameters, coordinate transformations etc..
	Observer * observatory;			// Manage observer position
	Projector * projection;				// Manage the projection mode and matrix
	Object selected_object;			// The selected object
	class HipStarMgr * hip_stars;		// Manage the hipparcos stars
	ConstellationMgr * asterisms;		// Manage constellations (boundaries, names etc..)
	NebulaMgr * nebulas;				// Manage the nebulas
	SolarSystem* ssystem;				// Manage the solar system
	Atmosphere * atmosphere;			// Atmosphere
	SkyGrid * equ_grid;					// Equatorial grid
	SkyGrid * azi_grid;					// Azimutal grid
	SkyGrid * gal_grid;					// Galactic grid
	SkyLine * equator_line;				// Celestial Equator line
	SkyLine * ecliptic_line;			// Ecliptic line
	SkyLine * precession_circle;			// Precession circle
	SkyLine * circumpolar_circle;			// Circumpolar circle
	SkyLine * tropic_line;				// Tropic line
	SkyLine * meridian_line;			// Meridian line
	Cardinals * cardinals_points;		// Cardinals points
	MilkyWay * milky_way;				// Our galaxy
	MeteorMgr * meteors;				// Manage meteor showers
	Landscape * landscape;				// The landscape ie the fog, the ground and "decor"
	ToneReproductor * tone_converter;	// Tones conversion between simulation world and display device
	SkyLocalizer *skyloc;				// for sky cultures and locales
	class TelescopeMgr *telescope_mgr;

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	void drawChartBackground(void);
	string get_cursor_pos(int x, int y);

	// Increment/decrement smoothly the vision field and position
	void updateMove(int delta_time);
	int FlagEnableZoomKeys;
	int FlagEnableMoveKeys;

	double deltaFov,deltaAlt,deltaAz;	// View movement
	double move_speed, zoom_speed;		// Speed of movement and zooming

	float InitFov;						// Default viewing FOV
	Vec3d InitViewPos;					// Default viewing direction
	int FlagManualZoom;					// Define whether auto zoom can go further
	Vec3f chartColor;					// ?
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	float flight_duration;			    // Duration of flight between bodies in seconds

	bool firstTime;                     // For init to track if reload or first time setup
	float constellationFontSize;        // size for constellation labels
	bool aboveHomePlanet;               // true if looking down at home planet (high altitude)

	bool FlagAntialiasLines;            // whether to antialias all line drawing
	float m_lineWidth;                  // width to use when drawing any line

	bool FlagShaders;                   // whether to render using shaders

	// Manage geodesic grid
	GeodesicGrid* geodesic_grid;

	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)
};

#endif // _CORE_H_
