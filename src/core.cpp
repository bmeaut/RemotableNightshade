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

// Main class for the application
// Manage all the objects to be used in the program

#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "core.h"
#include "stellastro.h"
#include "utility.h"
#include "hip_star_mgr.h"

// TODO this needs to be replaced with a scriptable spherical image feature
void Core::milkyswap(string mdir) {
  milky_way->set_texture(mdir);
}

Core::Core(const string& LDIR, const string& DATA_ROOT, const boost::callback<void, string>& recordCallback) :
		skyTranslator(APP_NAME, LOCALEDIR, ""),
		projection(NULL), selected_object(NULL), hip_stars(NULL),
		nebulas(NULL), ssystem(NULL), milky_way(NULL), 
		deltaFov(0.),
		deltaAlt(0.), deltaAz(0.), move_speed(0.00025), firstTime(1)
{
	localeDir = LDIR;
	dataRoot = DATA_ROOT;
	recordActionCallback = recordCallback;

	projection = Projector::create(Projector::PERSPECTIVE_PROJECTOR, Vec4i(0,0,800,600), 60);
	glFrontFace(projection->needGlFrontFaceCW()?GL_CW:GL_CCW);

	tone_converter = new ToneReproductor();
	atmosphere = new Atmosphere();
	ssystem = new SolarSystem();
	observatory = new Observer(*ssystem);
	navigation = new Navigator(observatory);
	nebulas = new NebulaMgr();
	milky_way = new MilkyWay();
	equ_grid = new SkyGrid(SkyGrid::EQUATORIAL);
	azi_grid = new SkyGrid(SkyGrid::ALTAZIMUTAL);
	gal_grid = new SkyGrid(SkyGrid::GALACTIC);
	equator_line = new SkyLine(SkyLine::EQUATOR);
	ecliptic_line = new SkyLine(SkyLine::ECLIPTIC);
	precession_circle = new SkyLine(SkyLine::PRECESSION);
	circumpolar_circle = new SkyLine(SkyLine::CIRCUMPOLAR);
	tropic_line = new SkyLine(SkyLine::TROPIC);
	meridian_line = new SkyLine(SkyLine::MERIDIAN, 1, 36);
	cardinals_points = new Cardinals();
	meteors = new MeteorMgr(10, 60);
	landscape = new LandscapeOldStyle();
	skyloc = new SkyLocalizer(getDataDir()+ "sky_cultures");
	hip_stars = new HipStarMgr();
	asterisms = new ConstellationMgr(hip_stars);

	// Set textures directory and suffix
	s_texture::set_texDir(getDataRoot() + "/textures/");

	object_pointer_visibility = 1;

	aboveHomePlanet = false;
}

Core::~Core()
{
	// release the previous Object:
	selected_object = Object();
	delete navigation;
	delete projection;
	delete asterisms;
	delete hip_stars;
	delete nebulas;
	delete equ_grid;
	delete azi_grid;
	delete gal_grid;
	delete equator_line;
	delete ecliptic_line;
	delete precession_circle;
	delete circumpolar_circle;
	delete tropic_line;
	delete meridian_line;
	delete cardinals_points;
	delete landscape;
	landscape = NULL;
	delete observatory;
	observatory = NULL;
	delete milky_way;
	delete meteors;
	meteors = NULL;
	delete atmosphere;
	delete tone_converter;
	delete ssystem;
	delete skyloc;
	skyloc = NULL;
	Object::delete_textures(); // Unload the pointer textures
}

// Load core data and initialize with default values
void Core::init(const InitParser& conf, const int viewW, const int viewH )
{
	baseFontFile = getDataDir() + conf.get_str("gui", "base_font_name", "DejaVuSans.ttf");

	// Video Section
	setViewportSize(viewW, viewH);
	setViewportHorizontalOffset(conf.get_int    ("video:horizontal_offset"));
	setViewportVerticalOffset(conf.get_int    ("video:vertical_offset"));

	// Rendering options
	setLineWidth(conf.get_double("rendering", "line_width", 1));
	setFlagAntialiasLines(conf.get_boolean("rendering", "flag_antialias_lines", false));
	setFlagShaders(conf.get_boolean("rendering", "flag_shaders", true));

	// Projector
	string tmpstr = conf.get_str("projection:type");
	setProjectionType(tmpstr);

	tmpstr = conf.get_str("projection:viewport");
	const Projector::PROJECTOR_MASK_TYPE projMaskType = Projector::stringToMaskType(tmpstr);
	projection->setMaskType(projMaskType);

// (test) for better depth testing 20080709
	// LEQUAL rather than LESS required for multipass rendering
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);

	// Start splash with no fonts due to font collection delays
	if (firstTime) {
		LoadingBar splash(projection, FontSizeGeneral, "", "logo24bits.png", getViewportWidth(), getViewportHeight(), VERSION, 20, 330, 111);
		splash.Draw(0);

		// This may take longer as fonts initialized
		cout << "Initializing Fonts.\n";
		LoadingBar lb(projection, FontSizeGeneral, baseFontFile, "logo24bits.png", getViewportWidth(), getViewportHeight(), VERSION, 20, 330, 111);
		lb.Draw(0);

		// Init the solar system first
		ssystem->load(getDataDir() + "ssystem.ini", lb);
		// Init stars
		hip_stars->init(FontSizeGeneral, baseFontFile, lb, conf);
		hip_stars->load_sci_names(getDataDir() + "name.fab");

		// Init nebulas
		nebulas->read(FontSizeGeneral, baseFontFile, getDataDir() + "ngc2000.dat", getDataDir() + "ngc2000names.dat", getDataDir() + "nebula_textures.fab", lb);
	}

	// Astro section
	setFlagStars(conf.get_boolean("astro:flag_stars"));
	setFlagStarName(conf.get_boolean("astro:flag_star_name"));
	setStarScale(conf.get_double ("stars", "star_scale", 1));
	setFlagStarTwinkle(conf.get_boolean("stars", "flag_star_twinkle", true));
	setStarTwinkleAmount(conf.get_double ("stars", "star_twinkle_amount", 0.3));
	setMaxMagStarName(conf.get_double ("stars", "max_mag_star_name", 1.5));
	setStarMagScale(conf.get_double ("stars", "star_mag_scale", 1));
	setFlagPointStar(conf.get_boolean("stars", "flag_point_star", false));
	setMagConverterMaxFov(conf.get_double("stars","mag_converter_max_fov",60.0));
	setMagConverterMinFov(conf.get_double("stars","mag_converter_min_fov",0.1));
	setMagConverterMagShift(conf.get_double("stars","mag_converter_mag_shift",0.0));
	setMagConverterMaxMag(conf.get_double("stars","mag_converter_max_mag",30.0));
	setStarSizeLimit(conf.get_double("astro","star_size_limit",5.0));
	setStarLimitingMag(conf.get_double("stars","star_limiting_mag",6.5f));
	setFlagPlanets(conf.get_boolean("astro:flag_planets"));
	setFlagPlanetsHints(conf.get_boolean("astro:flag_planets_hints"));
	setFlagPlanetsOrbits(conf.get_boolean("astro:flag_planets_orbits"));
	setFlagLightTravelTime(conf.get_boolean("astro", "flag_light_travel_time", 0));
	setFlagPlanetsTrails(conf.get_boolean("astro", "flag_object_trails", false));
	startPlanetsTrails(conf.get_boolean("astro", "flag_object_trails", false));
	setFlagNebula(conf.get_boolean("astro:flag_nebula"));
	setFlagNebulaHints(conf.get_boolean("astro:flag_nebula_name"));
	setNebulaMaxMagHints(conf.get_double("astro", "max_mag_nebula_name", 99));
	setNebulaCircleScale(conf.get_double("astro", "nebula_scale",1.0f));
	setFlagNebulaDisplayNoTexture(conf.get_boolean("astro", "flag_nebula_display_no_texture", false));
	setFlagMilkyWay(conf.get_boolean("astro:flag_milky_way"));
	setMilkyWayIntensity(conf.get_double("astro","milky_way_intensity",1.));
	setFlagBrightNebulae(conf.get_boolean("astro:flag_bright_nebulae"));

	setPlanetsScale(getStarScale());
	setPlanetsSizeLimit(conf.get_double("astro", "planet_size_marginal_limit", 4));

	ssystem->setFont(FontSizeSolarSystem, baseFontFile);
	setFlagClouds(true);

	observatory->load(conf, "init_location");

	// make sure nothing selected or tracked
	deselect();
	navigation->set_flag_traking(0);
	navigation->set_flag_lock_equ_pos(0);

	navigation->set_time_speed(JD_SECOND);  // reset to real time

	navigation->set_JDay(NShadeDateTime::JulianFromSys());
	navigation->set_local_vision(Vec3f(1,1e-05,0.2));

	// Init fonts : should be moved into the constructor
	equ_grid->set_font(FontSizeGeneral, baseFontFile);
	azi_grid->set_font(FontSizeGeneral, baseFontFile);
	gal_grid->set_font(FontSizeGeneral, baseFontFile);
	equator_line->set_font(FontSizeGeneral, baseFontFile);
	ecliptic_line->set_font(FontSizeGeneral, baseFontFile);
	precession_circle->set_font(FontSizeGeneral, baseFontFile);
	circumpolar_circle->set_font(FontSizeGeneral, baseFontFile);
	tropic_line->set_font(FontSizeGeneral, baseFontFile);
	meridian_line->set_font(FontSizeGeneral, baseFontFile);
	cardinals_points->set_font(FontSizeCardinalPoints, baseFontFile);

	// Init milky way and set default texture
	if (firstTime) milky_way->set_texture(getDataRoot() + "/textures/milkyway.jpg", true, true);
	else milky_way->set_texture("default");

	setLandscape(observatory->get_landscape_name());

	// Load the pointer textures
	Object::init_textures();

	// now redo this so we fill the autocomplete dialogs now UI inititalised
	// set_system_locale_by_name(SkyLocale); // and UILocale are the same but different format fra vs fr_FR!!!! TONY

	tone_converter->set_world_adaptation_luminance(3.75f + atmosphere->get_intensity()*40000.f);

	// Compute planets data and init viewing position
	// Position of sun and all the satellites (ie planets)
	ssystem->computePositions(navigation->get_JDay(),
	                          navigation->getHomePlanet());

	// Compute transform matrices between coordinates systems
	navigation->update_transform_matrices();
	navigation->update_model_view_mat(projection, projection->get_fov());

	setPlanetsSelected("");	// Fix a bug on macosX! Thanks Fumio!

	string skyLocaleName = conf.get_str("localization", "sky_locale", "system");
	constellationFontSize = conf.get_double("viewing","constellation_font_size",FontSizeConstellations);
	setSkyLanguage(skyLocaleName);

	int grid_level = hip_stars->getMaxGridLevel();
	geodesic_grid = new GeodesicGrid(grid_level);
	hip_stars->setGrid(geodesic_grid);

	FlagEnableZoomKeys	= conf.get_boolean("navigation:flag_enable_zoom_keys");
	FlagEnableMoveKeys  = conf.get_boolean("navigation:flag_enable_move_keys");
	setFlagManualAutoZoom( conf.get_boolean("navigation:flag_manual_zoom") );

	setAutomoveDuration( conf.get_double ("navigation","auto_move_duration",1.5) );
	move_speed			= conf.get_double("navigation","move_speed",0.0004);
	zoom_speed			= conf.get_double("navigation","zoom_speed", 0.0004);
	setFlightDuration( conf.get_double ("navigation","flight_duration",20.) );

	// Viewing Mode
	tmpstr = conf.get_str("navigation:viewing_mode");
	if (tmpstr=="equator") 	navigation->set_viewing_mode(Navigator::VIEW_EQUATOR);
	else {
		if (tmpstr=="horizon") navigation->set_viewing_mode(Navigator::VIEW_HORIZON);
		else {
			cerr << "ERROR : Unknown viewing mode type : " << tmpstr << endl;
			assert(0);
		}
	}

	InitFov				= conf.get_double ("navigation","init_fov",60.);
	projection->set_fov(InitFov);

	double heading = conf.get_double ("navigation","heading",0.);
	navigation->set_defaultHeading(heading);
	navigation->set_heading(heading);

	InitViewPos = Utility::str_to_vec3f(conf.get_str("navigation:init_view_pos").c_str());

	double viewOffset = conf.get_double ("navigation","view_offset",0.);

	setViewOffset(viewOffset);

	// Load constellations from the correct sky culture
	string tmp = conf.get_str("localization", "sky_culture", "western");
	setSkyCultureDir(tmp);
	skyCultureDir = tmp;

	// Landscape section
	setFlagLandscape(conf.get_boolean("landscape", "flag_landscape", conf.get_boolean("landscape", "flag_ground", 1)));  // name change
	setFlagFog(conf.get_boolean("landscape:flag_fog"));
	setFlagAtmosphere(conf.get_boolean("landscape:flag_atmosphere"));
	setAtmosphereFadeDuration(conf.get_double("landscape","atmosphere_fade_duration",1.5));

	// Viewing section
	setFlagConstellationLines(		conf.get_boolean("viewing:flag_constellation_drawing"));
	setFlagConstellationNames(		conf.get_boolean("viewing:flag_constellation_name"));
	setFlagConstellationBoundaries(	conf.get_boolean("viewing","flag_constellation_boundaries",false));
	setFlagConstellationArt(		conf.get_boolean("viewing:flag_constellation_art"));
	setFlagConstellationIsolateSelected(conf.get_boolean("viewing", "flag_constellation_isolate_selected",conf.get_boolean("viewing", "flag_constellation_pick", 0)));
	setConstellationArtIntensity(conf.get_double("viewing","constellation_art_intensity", 0.5));
	setConstellationArtFadeDuration(conf.get_double("viewing","constellation_art_fade_duration",2.));

	setFlagAzimutalGrid(conf.get_boolean("viewing:flag_azimutal_grid"));
	setFlagEquatorGrid(conf.get_boolean("viewing:flag_equatorial_grid"));
	setFlagGalacticGrid(conf.get_boolean("viewing:flag_galactic_grid"));
	setFlagEquatorLine(conf.get_boolean("viewing:flag_equator_line"));
	setFlagEclipticLine(conf.get_boolean("viewing:flag_ecliptic_line"));
	setFlagPrecessionCircle(conf.get_boolean("viewing:flag_precession_circle"));
	setFlagCircumpolarCircle(conf.get_boolean("viewing:flag_circumpolar_circle"));
	setFlagTropicLines(conf.get_boolean("viewing", "flag_tropic_lines", false));
	setFlagMeridianLine(conf.get_boolean("viewing:flag_meridian_line"));
	cardinals_points->setFlagShow(conf.get_boolean("viewing:flag_cardinal_points"));
	setFlagGravityLabels( conf.get_boolean("viewing:flag_gravity_labels") );
	setFlagMoonScaled(conf.get_boolean("viewing", "flag_moon_scaled", conf.get_boolean("viewing", "flag_init_moon_scaled", false)));  // name change
	setMoonScale(conf.get_double ("viewing","moon_scale",5.));

	setLightPollutionLimitingMagnitude(conf.get_double("viewing","light_pollution_limiting_magnitude", 6.5));

	setMeteorsRate(conf.get_int("astro", "meteor_rate", 10));

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	firstTime = 0;
}

// Update all the objects in function of the time
void Core::update(int delta_time)
{
   if( firstTime ) // Trystan 7-8-10: Do not update prior to Init. Causes intermittent problems at startup
      return;

	// Update the position of observation and time etc...
	observatory->update(delta_time);
	navigation->update_time(delta_time);

	// Position of sun and all the satellites (ie planets)
	ssystem->computePositions(navigation->get_JDay(),
	                          navigation->getHomePlanet());

	// Transform matrices between coordinates systems
	navigation->update_transform_matrices();
	// Direction of vision
	navigation->update_vision_vector(delta_time, selected_object);
	// Field of view
	projection->update_auto_zoom(delta_time, FlagManualZoom);

	// update faders and Planet trails (call after nav is updated)
	ssystem->update(delta_time, navigation);

	// Move the view direction and/or fov
	updateMove(delta_time);

	// Update info about selected object
	selected_object.update();

	// Update faders
	equ_grid->update(delta_time);
	azi_grid->update(delta_time);
	gal_grid->update(delta_time);
	equator_line->update(delta_time);
	ecliptic_line->update(delta_time);
	precession_circle->update(delta_time);
	circumpolar_circle->update(delta_time);
	tropic_line->update(delta_time);
	meridian_line->update(delta_time);
	asterisms->update(delta_time);
	atmosphere->update(delta_time);
	landscape->update(delta_time);
	hip_stars->update(delta_time);
	nebulas->update(delta_time);
	cardinals_points->update(delta_time);
	milky_way->update(delta_time);

	// Compute the sun position in local coordinate
	Vec3d temp(0.,0.,0.);
	Vec3d sunPos = navigation->helio_to_local(temp);


	// Compute the moon position in local coordinate
	Vec3d moon = ssystem->getMoon()->get_heliocentric_ecliptic_pos();
	Vec3d moonPos = navigation->helio_to_local(moon);

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color - update 200608230
	projection->set_modelview_matrices( navigation->get_earth_equ_to_eye_mat(),
	                                    navigation->get_helio_to_eye_mat(),
	                                    navigation->get_local_to_eye_mat(),
	                                    navigation->get_j2000_to_eye_mat(),
										navigation->get_galactic_to_eye_mat(),
	                                    navigation->get_dome_mat(),
										navigation->get_dome_fixed_mat());

	// Compute the atmosphere color and intensity
	atmosphere->compute_color(navigation->get_JDay(), sunPos, moonPos,
	                          ssystem->getMoon()->get_phase(ssystem->getEarth()->get_heliocentric_ecliptic_pos()),
	                          tone_converter, projection, observatory->get_latitude(), observatory->get_altitude(),
	                          15.f, 40.f);	// Temperature = 15c, relative humidity = 40%
	tone_converter->set_world_adaptation_luminance(atmosphere->get_world_adaptation_luminance());

	sunPos.normalize();
	moonPos.normalize();
	// compute global sky brightness TODO : make this more "scientifically"
	// TODO: also add moonlight illumination

	if (sunPos[2] < -0.1/1.5 ) sky_brightness = 0.01;
	else sky_brightness = (0.01 + 1.5*(sunPos[2]+0.1/1.5));

	// TODO make this more generic for non-atmosphere planets
	if (atmosphere->get_fade_intensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky
		// otherwise we just use the sun position calculation above
		sky_brightness *= (atmosphere->get_intensity()+0.1);
	}

	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	landscape->set_sky_brightness(sky_brightness+0.05);

	// - if above troposphere equivalent on Earth in altitude
	// TODO also do not draw atmosphere off Earth or landscape on hidden or artificial planet
	aboveHomePlanet =
		(observatory->get_altitude() / (navigation->getHomePlanet()->getRadius() * AU) > 3);
	if (aboveHomePlanet) {
		setFlagAtmosphere(0);
	}
}

// Execute all the drawing functions
double Core::draw(int delta_time)
{

	// Init openGL viewing with fov, screen size and clip planes
	//	projection->set_clipping_planes(0.000001 ,50);
	projection->set_clipping_planes(0.000001 ,200);  // allow for Eris, etc.


	// Init viewport to current projector values
	projection->applyViewport();

	// User supplied line width value
	glLineWidth(m_lineWidth);

	// Set openGL drawings in equatorial coordinates
	navigation->switch_to_earth_equatorial();

	glBlendFunc(GL_ONE, GL_ONE);

	// Draw the milky way.
	milky_way->draw(tone_converter, projection, navigation);

	// Draw the nebula
	nebulas->draw(projection, navigation, tone_converter, getFlagAtmosphere() ? sky_brightness : 0);

	// Draw all the constellations
	asterisms->draw(projection, navigation);

	// Draw the hipparcos stars
// for onscreen test with offset view
//	Vec3d center = projection->getViewportCenter();
//	Vec3d tempv;
//	projection->unproject_j2000(center[0], center[1], tempv);
//	Vec3f temp(tempv[0],tempv[1],tempv[2]);
	hip_stars->draw(this, tone_converter, projection);

	// Draw the equatorial grid
	equ_grid->draw(projection);

	// Draw the altazimutal grid
	azi_grid->draw(projection);

	// Draw the galactic grid
	gal_grid->draw(projection);

	// Draw the celestial equator line
	equator_line->draw(projection, navigation);

	// Draw the ecliptic line
	ecliptic_line->draw(projection, navigation);

	// Draw the precession circle
	precession_circle->draw(projection, navigation);

	// Draw the circumpolar circle
	circumpolar_circle->draw(projection, navigation);

	// Draw the tropic lines
	tropic_line->draw(projection, navigation);

	// Draw the meridian line
	meridian_line->draw(projection, navigation);

	// Draw the planets
	double squaredDistance = ssystem->draw(projection,
	                                       navigation,
	                                       tone_converter,
	                                       getFlagPointStar(),
	                                       aboveHomePlanet );

	// Draw the pointer on the currently selected object
	// TODO: this would be improved if pointer was drawn at same time as object for correct depth in scene
	if (selected_object && object_pointer_visibility) selected_object.draw_pointer(delta_time, projection, navigation);

	// Set openGL drawings in local coordinates i.e. generally altazimuthal coordinates
	navigation->switch_to_local();

	// Update meteors
	meteors->update(projection, navigation, tone_converter, delta_time);

	if (!aboveHomePlanet && (!getFlagAtmosphere() || sky_brightness<0.1)) {
		projection->set_orthographic_projection();
		meteors->draw(projection, navigation);
		projection->reset_perspective_projection();
	}

	// Draw the atmosphere
	atmosphere->draw(projection, delta_time);

	// Draw the landscape
	if (!aboveHomePlanet) // TODO decide if useful or too confusing to leave alone
		landscape->draw(tone_converter, projection, navigation);

	// Draw the cardinal points
	//if (FlagCardinalPoints)
	cardinals_points->draw(projection, observatory->get_latitude());

	// draw all loaded images, by a script or remote interface
	projection->set_orthographic_projection();
	ImageMgr::drawAll(navigation, projection);

	projection->reset_perspective_projection();

	projection->draw_viewport_shape();

	return squaredDistance;
}

bool Core::setLandscape(const string& new_landscape_name)
{
	if (new_landscape_name.empty()) return 0;
	Landscape* newLandscape = Landscape::create_from_file(getDataDir() + "landscapes.ini", new_landscape_name);

	if (!newLandscape) return 0;

	if (landscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(landscape->getFlagShow());
		newLandscape->setFlagShowFog(landscape->getFlagShowFog());
		delete landscape;
		landscape = newLandscape;
	}
	observatory->set_landscape_name(new_landscape_name);
	return 1;
}


//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
//! and make it the current landscape
bool Core::loadLandscape(stringHash_t& param)
{

	Landscape* newLandscape = Landscape::create_from_hash(param);
	if (!newLandscape) return 0;

	if (landscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(landscape->getFlagShow());
		newLandscape->setFlagShowFog(landscape->getFlagShowFog());
		delete landscape;
		landscape = newLandscape;
	}
	observatory->set_landscape_name(param["name"]);
	// probably not particularly useful, as not in landscape.ini file

	return 1;
}

//! Load a solar system body based on a hash of parameters mirroring the ssystem.ini file
string Core::addSolarSystemBody(stringHash_t& param)
{
	return ssystem->addBody(param);
}

string Core::removeSolarSystemBody(string name)
{

	// Make sure this object is not already selected so won't crash
	if (selected_object.get_type()==ObjectRecord::OBJECT_PLANET &&
	        selected_object.getEnglishName() == name) {
		unSelect();
	}

	// Make sure not standing on this object!
	const Planet *p = navigation->getHomePlanet();
	if (p->getEnglishName() == name) {
		//    const Planet *par = p->get_parent();
		//if( par->getEnglishName() == parent || (parent=="none" && par==NULL)) {
		return (string("Can not delete current home planet ") + name);
		//}
	}

	return ssystem->removeBody(name);
}

string Core::removeSupplementalSolarSystemBodies()
{


	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;

	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.get_type()==ObjectRecord::OBJECT_PLANET
	        && selected_object.isDeleteable() ) {
		unSelect();

	}

	// Make sure not standing on an object we will delete!
	const Planet *p = navigation->getHomePlanet();

	return ssystem->removeSupplementalBodies(p);

}


// get selected object name if it's a planet only
// for setting home planet to selection with keystroke
string Core::getSelectedPlanetEnglishName() const
{

	// Make sure this object is a planet
	if (selected_object.get_type()==ObjectRecord::OBJECT_PLANET)
		return selected_object.getEnglishName();

	return ""; // not a planet
}


void Core::setViewportSize(int w, int h)
{
	if (w==getViewportWidth() && h==getViewportHeight()) return;
	projection->setViewportWidth(w);
	projection->setViewportHeight(h);
}

Object Core::searchByNameI18n(const string &name) const
{
	Object rval;
	rval = ssystem->searchByNamesI18(name);
	if (rval) return rval;
	rval = nebulas->searchByNameI18n(name);
	if (rval) return rval;
	rval = hip_stars->searchByNameI18n(name).get();
	if (rval) return rval;
	rval = asterisms->searchByNameI18n(name);
	return rval;
}

//! Find and select an object from its translated name
//! @param nameI18n the case sensitive object translated name
//! @return true if an object was found with the passed name
bool Core::findAndSelectI18n(const string &nameI18n)
{
	// Then look for another object
	Object obj = searchByNameI18n(nameI18n);
	if (!obj) return false;
	else return selectObject(obj);
}


//! Find and select an object based on selection type and standard name or number
//! @return true if an object was selected

bool Core::selectObject(const string &type, const string &id)
{
	/*
	  std::wostringstream oss;
	  oss << id.c_str();
	  return findAndSelectI18n(oss.str());
	*/
	if (type=="hp") {
		unsigned int hpnum;
		std::istringstream istr(id);
		istr >> hpnum;
		selected_object = hip_stars->searchHP(hpnum).get();
		asterisms->setSelected(selected_object);
		setPlanetsSelected("");

	} else if (type=="star") {
		selected_object = hip_stars->search(id).get();
		asterisms->setSelected(selected_object);
		setPlanetsSelected("");

	} else if (type=="planet") {
		setPlanetsSelected(id);
		selected_object = ssystem->getSelected();
		asterisms->setSelected(Object());

	} else if (type=="nebula") {
		selected_object = nebulas->search(id);
		setPlanetsSelected("");
		asterisms->setSelected(Object());

	} else if (type=="constellation") {

		// Select only constellation, nothing else
		asterisms->setSelected(id);

		selected_object = NULL;
		setPlanetsSelected("");

	} else if (type=="constellation_star") {

		// For Find capability, select a star in constellation so can center view on constellation
		asterisms->setSelected(id);

		selected_object = asterisms->getSelected()
		                  .getBrightestStarInConstellation().get();

/// what is this?
/// 1) Find the hp-number of the 1st star in the selected constellation,
/// 2) find the star of this hpnumber
/// 3) select the constellation of this star ???
///		const unsigned int hpnum = asterisms->getFirstSelectedHP();
///		selected_object = hip_stars->searchHP(hpnum);
///		asterisms->setSelected(selected_object);

		setPlanetsSelected("");

///		// Some stars are shared, so now force constellation
///		asterisms->setSelected(id);
	} else {
		cerr << "Invalid selection type specified: " << type << endl;
		return 0;
	}


	if (selected_object) {
		if (navigation->get_flag_traking()) navigation->set_flag_lock_equ_pos(1);
		navigation->set_flag_traking(0);

		return 1;
	}

	return 0;
}


//! Find and select an object near given equatorial position
bool Core::findAndSelect(const Vec3d& pos)
{
	Object tempselect = clever_find(pos);
	return selectObject(tempselect);
}

//! Find and select an object near given screen position
bool Core::findAndSelect(int x, int y)
{
	Vec3d v;
	projection->unproject_earth_equ(x,getViewportHeight()-y,v);
	return findAndSelect(v);
}

//! Deselect all selected objects if any
//! Does deselect selected constellations
void Core::deselect(void)
{
	unSelect();
	asterisms->deselect();
}

// - allow selection of large nearby planets more easily
// and do not select hidden planets

// Find an object in a "clever" way
Object Core::clever_find(const Vec3d& v) const
{
	Object sobj;
	Object default_object;
	bool is_default_object = false;

	vector<Object> candidates;
	vector<Object> temp;
	Vec3d winpos;

	// Field of view for a 30 pixel diameter circle on screen
	float fov_around = projection->get_fov()/MY_MIN(projection->getViewportWidth(), projection->getViewportHeight()) * 30.f;

	float xpos, ypos;
	projection->project_earth_equ(v, winpos);
	xpos = winpos[0];
	ypos = winpos[1];

	// Collect the planets inside the range
	if (getFlagPlanets()) {
		temp = ssystem->search_around(v, fov_around, navigation, projection,
		                              &is_default_object, aboveHomePlanet);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());

		if (is_default_object && temp.begin() != temp.end()) {
//			cout << "was default object\n";
			vector<Object>::iterator iter = temp.end();
			iter--;
			default_object = (*iter);
		} else {
			// should never get here
			is_default_object = false;
		}
	}

	// nebulas and stars used precessed equ coords
	Vec3d p = navigation->earth_equ_to_j2000(v);

	// The nebulas inside the range
	if (getFlagNebula()) {
		temp = nebulas->search_around(p, fov_around);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());
	}

	// And the stars inside the range
	if (getFlagStars()) {
		vector<ObjectBaseP > tmp = hip_stars->searchAround(p, fov_around, this);
		for( vector<ObjectBaseP >::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
			candidates.push_back( Object(itr->get()) );
		}
	}

	// Now select the object minimizing the function y = distance(in pixel) + magnitude
	float best_object_value;
	best_object_value = 100000.f;
	vector<Object>::iterator iter = candidates.begin();
	while (iter != candidates.end()) {
		projection->project_earth_equ((*iter).get_earth_equ_pos(navigation), winpos);

		float distance = sqrt((xpos-winpos[0])*(xpos-winpos[0]) + (ypos-winpos[1])*(ypos-winpos[1]));
		float mag = (*iter).get_mag(navigation);

		if ((*iter).get_type()==ObjectRecord::OBJECT_NEBULA) {
			if ( nebulas->getFlagHints() ) {
				// make very easy to select IF LABELED
				mag = -1;

			}
		}
		if ((*iter).get_type()==ObjectRecord::OBJECT_PLANET) {
			if ( getFlagPlanetsHints() ) {
				// easy to select, especially pluto
				mag -= 15.f;
			} else {
				mag -= 8.f;
			}
		}
		if (distance + mag < best_object_value) {
			best_object_value = distance + mag;
			sobj = *iter;
		}
		iter++;
	}

// when large planet disk is hiding anything else
	if (is_default_object && sobj.get_type()!=ObjectRecord::OBJECT_PLANET)
		return default_object;

	return sobj;
}

Object Core::clever_find(int x, int y) const
{
	Vec3d v;
	projection->unproject_earth_equ(x,y,v);
	return clever_find(v);
}

// Go and zoom to the selected object.
void Core::autoZoomIn(float move_duration, bool allow_manual_zoom)
{
	float manual_move_duration;

	if (!selected_object) return;

	if (!navigation->get_flag_traking()) {
		navigation->set_flag_traking(true);
		navigation->move_to(selected_object.get_earth_equ_pos(navigation),
		                    move_duration, false, 1);
		manual_move_duration = move_duration;
	} else {
		// faster zoom in manual zoom mode once object is centered
		manual_move_duration = move_duration*.66f;
	}

	if ( allow_manual_zoom && FlagManualZoom ) {
		// if manual zoom mode, user can zoom in incrementally
		float newfov = projection->get_fov()*0.5f;
		projection->zoom_to(newfov, manual_move_duration);

	} else {
		float satfov = selected_object.get_satellites_fov(navigation);

		if (satfov>0.0 && projection->get_fov()*0.9>satfov)
			projection->zoom_to(satfov, move_duration);
		else {
			float closefov = selected_object.get_close_fov(navigation);
			if (projection->get_fov()>closefov)
				projection->zoom_to(closefov, move_duration);
		}
	}
}


// Unzoom and go to the init position
void Core::autoZoomOut(float move_duration, bool full, bool allow_manual_zoom)
{

	if (selected_object && !full) {


		// Handle manual unzoom
		if ( allow_manual_zoom && FlagManualZoom ) {
			// if manual zoom mode, user can zoom out incrementally
			float newfov = projection->get_fov()*2.f;
			if (newfov >= InitFov ) {

				// Need to go to init fov/direction
				projection->zoom_to(InitFov, move_duration);
				navigation->move_to(InitViewPos, move_duration, true, -1);
				navigation->set_flag_traking(false);
				navigation->set_flag_lock_equ_pos(0);

				return;

			} else {

				// faster zoom in manual zoom with object centered
				float manual_move_duration = move_duration*.66f;

				projection->zoom_to(newfov, manual_move_duration);
				return;
			}
		}


		// If the selected object has satellites, unzoom to satellites view
		// unless specified otherwise
		float satfov = selected_object.get_satellites_fov(navigation);

// Saturn wasn't untracking from moon issue
		if (satfov>0.0 && projection->get_fov()<=satfov*0.9 && satfov < .9*InitFov) {
			projection->zoom_to(satfov, move_duration);
			return;
		}

		// If the selected object is part of a Planet subsystem (other than sun),
		// unzoom to subsystem view
		satfov = selected_object.get_parent_satellites_fov(navigation);

		//    cout << "Unzoom to parent sat fov: " << satfov << endl;

// Charon wasn't untracking from Pluto issue
		if (satfov>0.0 && projection->get_fov()<=satfov*0.9 && satfov < .9*InitFov) {
			projection->zoom_to(satfov, move_duration);
			return;
		}
	}

	//  cout << "Unzoom to initfov\n";
	projection->zoom_to(InitFov, move_duration);
	navigation->move_to(InitViewPos, move_duration, true, -1);
	navigation->set_flag_traking(false);
	navigation->set_flag_lock_equ_pos(0);

}

// Set the current sky culture according to passed name
bool Core::setSkyCulture(const string& cultureName)
{
	return setSkyCultureDir(skyloc->skyCultureToDirectory(cultureName));
}

// Set the current sky culture from the passed directory
bool Core::setSkyCultureDir(const string& cultureDir)
{
	//	cout << "Set sky culture to: " << cultureDir << "(skyCultureDir: " << skyCultureDir << endl;

	if (skyCultureDir == cultureDir) return 1;

	// make sure culture definition exists before attempting or will die
	// Do not comment this out! Rob
	if (skyloc->directoryToSkyCultureEnglish(cultureDir) == "") {
		cerr << "Invalid sky culture directory: " << cultureDir << endl;
		return 0;
	}

	skyCultureDir = cultureDir;

	if (!asterisms) return 0;

	LoadingBar lb(projection, FontSizeGeneral, baseFontFile, "logo24bits.png", getViewportWidth(), getViewportHeight(), VERSION, 30, 320, 91);

	asterisms->loadLinesAndArt(getDataDir() + "sky_cultures/" + skyCultureDir, lb);

	asterisms->loadNames(getDataDir() + "sky_cultures/" + skyCultureDir + "/constellation_names.eng.fab");

	// Re-translated constellation names
	asterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.get_type()==ObjectRecord::OBJECT_STAR) {
		asterisms->setSelected(selected_object);
	} else {
		asterisms->setSelected(Object());
	}

	// Load culture star names in english
	hip_stars->load_common_names(getDataDir() + "sky_cultures/" + skyCultureDir + "/star_names.fab");
	// Turn on sci names for western culture only
	hip_stars->setFlagSciNames( skyCultureDir.compare(0, 7, "western") ==0 );

	// translate
	hip_stars->updateI18n(skyTranslator);

	ObjectsState state;
	strncpy( state.m_state.sky_culture, cultureDir.c_str(), sizeof(state.m_state.sky_culture) );
	state.m_state.sky_culture[sizeof(state.m_state.sky_culture) - 1] = '\0';
	SharedData::Instance()->Objects( state );

	return 1;
}


// For loading custom sky cultures from scripts, use any path
// Set the current sky culture from the arbitrary path
bool Core::loadSkyCulture(const string& culturePath)
{
	// TODO: how to deal with culture hash and current value
	skyCultureDir = "Custom";  // This allows reloading defaults correctly

	if (!asterisms) return 0;

	LoadingBar lb(projection, FontSizeGeneral, baseFontFile, "logo24bits.png", getViewportWidth(), getViewportHeight(), VERSION, 30, 320, 91);

	asterisms->loadLinesAndArt(culturePath, lb);

	asterisms->loadNames(culturePath + "/constellation_names.eng.fab");

	// Re-translated constellation names
	asterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.get_type()==ObjectRecord::OBJECT_STAR) {
		asterisms->setSelected(selected_object);
	} else {
		asterisms->setSelected(Object());
	}

	// Load culture star names in english
	hip_stars->load_common_names(culturePath + "/star_names.fab");
	// Turn on sci names for western culture only
	hip_stars->setFlagSciNames( culturePath.compare(0, 7, "western") ==0 );

	// translate
	hip_stars->updateI18n(skyTranslator);

	return 1;
}



//! @brief Set the sky locale and reload the sky objects names for gettext translation
void Core::setSkyLanguage(const std::string& newSkyLocaleName)
{
	if ( !hip_stars || !cardinals_points || !asterisms || !ecliptic_line) return; // objects not initialized yet

	string oldLocale = getSkyLanguage();

	// Update the translator with new locale name
	skyTranslator = Translator(PACKAGE, getLocaleDir(), newSkyLocaleName);
	// cout << "Sky locale is " << skyTranslator.getLocaleName() << endl;

	// see if fonts need to change
	string oldFontFile, newFontFile, tmpstr;
	float oldFontScale, newFontScale, tmpfloat;

	getFontForLocale(oldLocale, oldFontFile, oldFontScale, tmpstr, tmpfloat);
	getFontForLocale(newSkyLocaleName, newFontFile, newFontScale, tmpstr, tmpfloat);

	// If font has changed or init is being called for first time...
	if (oldFontFile != newFontFile || oldFontScale != newFontScale || firstTime) {

		cardinals_points->set_font(FontSizeCardinalPoints*newFontScale, newFontFile);
		ecliptic_line->set_font(FontSizeGeneral*newFontScale, newFontFile);
		asterisms->setFont(constellationFontSize*newFontScale, newFontFile);  // size is read from config
		ssystem->setFont(FontSizeSolarSystem*newFontScale, newFontFile);
		// not translating yet
		//		nebulas->setFont(FontSizeGeneral, font);
		hip_stars->setFont(FontSizeGeneral*newFontScale, newFontFile);

		// TODO: TUI short info font needs updating also
		// ui->setFonts(newFontScale, newFontFile, newFontScale, newFontFile);

	}

	// Translate all labels with the new language
	cardinals_points->translateLabels(skyTranslator);
	ecliptic_line->translateLabels(skyTranslator);
	asterisms->translateNames(skyTranslator);
	ssystem->translateNames(skyTranslator);
	nebulas->translateNames(skyTranslator);
	hip_stars->updateI18n(skyTranslator);

	SettingsState state;
	strcpy( state.m_state.sky_language, newSkyLocaleName.c_str() );
	SharedData::Instance()->Settings( state );
}


// Please keep saveCurrentSettings up to date with any new color settings added here
void Core::setColorScheme(const string& skinFile, const string& section)
{
	InitParser conf;
	conf.load(skinFile);

	// simple default color, rather than black which doesn't show up
	string defaultColor = "0.6,0.4,0";

	// Load colors from config file
	nebulas->setLabelColor(Utility::str_to_vec3f(conf.get_str(section,"nebula_label_color", defaultColor)));
	nebulas->setCircleColor(Utility::str_to_vec3f(conf.get_str(section,"nebula_circle_color", defaultColor)));
	setColorPrecessionCircle(Utility::str_to_vec3f(conf.get_str(section,"precession_circle_color", defaultColor)));
	setColorCircumpolarCircle(Utility::str_to_vec3f(conf.get_str(section,"circumpolar_circle_color", defaultColor)));
	hip_stars->setLabelColor(Utility::str_to_vec3f(conf.get_str(section,"star_label_color", defaultColor)));
	hip_stars->setCircleColor(Utility::str_to_vec3f(conf.get_str(section,"star_circle_color", defaultColor)));
	ssystem->setLabelColor(Utility::str_to_vec3f(conf.get_str(section,"planet_names_color", defaultColor)));
	ssystem->setPlanetOrbitColor(Utility::str_to_vec3f(conf.get_str(section,"planet_orbits_color", defaultColor)));
	ssystem->setSatelliteOrbitColor(Utility::str_to_vec3f(conf.get_str(section,"satellite_orbits_color", "0.65,0.5,1.0")));
	ssystem->setTrailColor(Utility::str_to_vec3f(conf.get_str(section,"object_trails_color", defaultColor)));
	equ_grid->setColor(Utility::str_to_vec3f(conf.get_str(section,"equatorial_color", defaultColor)));
	//equ_grid->set_top_transparancy(draw_mode==DM_NORMAL);
	azi_grid->setColor(Utility::str_to_vec3f(conf.get_str(section,"azimuthal_color", defaultColor)));
	//azi_grid->set_top_transparancy(draw_mode==DM_NORMAL);
	gal_grid->setColor(Utility::str_to_vec3f(conf.get_str(section,"galactic_color", defaultColor)));
	setColorEquatorLine(Utility::str_to_vec3f(conf.get_str(section,"equator_color", defaultColor)));
	ecliptic_line->setColor(Utility::str_to_vec3f(conf.get_str(section,"ecliptic_color", defaultColor)));
	meridian_line->set_font(FontSizeGeneral, baseFontFile);
// default color override
	meridian_line->setColor(Utility::str_to_vec3f(conf.get_str(section,"meridian_color", "0,0.8,1.0")));
	cardinals_points->setColor(Utility::str_to_vec3f(conf.get_str(section,"cardinal_color", defaultColor)));
	asterisms->setLineColor(Utility::str_to_vec3f(conf.get_str(section,"const_lines_color", defaultColor)));
	asterisms->setBoundaryColor(Utility::str_to_vec3f(conf.get_str(section,"const_boundary_color", "0.8,0.3,0.3")));
	asterisms->setLabelColor(Utility::str_to_vec3f(conf.get_str(section,"const_names_color", defaultColor)));

	asterisms->setArtColor(Utility::str_to_vec3f(conf.get_str(section,"const_art_color", "1.0,1.0,1.0")));

	chartColor = Utility::str_to_vec3f(conf.get_str(section,"chart_color", defaultColor));
}

//! Get a color used to display info about the currently selected object
Vec3f Core::getSelectedObjectInfoColor(void) const
{
	if (!selected_object) {
		cerr << "WARNING: Core::getSelectedObjectInfoColor was called while no object is currently selected!!" << endl;
		return Vec3f(1, 1, 1);
	}
	if (selected_object.get_type()==ObjectRecord::OBJECT_NEBULA) return nebulas->getLabelColor();
	if (selected_object.get_type()==ObjectRecord::OBJECT_PLANET) return ssystem->getLabelColor();
	if (selected_object.get_type()==ObjectRecord::OBJECT_STAR) return selected_object.get_RGB();
	return Vec3f(1, 1, 1);
}

void Core::drawChartBackground(void)
{
	int stepX = projection->getViewportWidth();
	int stepY = projection->getViewportHeight();
	int viewport_left = projection->getViewportPosX();
	int view_bottom = projection->getViewportPosY();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor3fv(chartColor);
	projection->set_orthographic_projection();	// set 2D coordinate
	glBegin(GL_QUADS);
	glTexCoord2s(0, 0);
	glVertex2i(viewport_left, view_bottom);	// Bottom Left
	glTexCoord2s(1, 0);
	glVertex2i(viewport_left+stepX, view_bottom);	// Bottom Right
	glTexCoord2s(1, 1);
	glVertex2i(viewport_left+stepX,view_bottom+stepY);	// Top Right
	glTexCoord2s(0, 1);
	glVertex2i(viewport_left,view_bottom+stepY);	// Top Left
	glEnd();
	projection->reset_perspective_projection();
}

string Core::get_cursor_pos(int x, int y)
{
	Vec3d v;
	projection->unproject_earth_equ(x,y,v);
	float tempDE, tempRA;
	rect_to_sphe(&tempRA,&tempDE,v);

	return string("RA : ") + Utility::printAngleHMS(tempRA) + "\n" +
		"DE : " + Utility::printAngleDMS(tempDE);
}

void Core::setProjectionType(const string& sptype)
{
	Projector::PROJECTOR_TYPE pType = Projector::stringToType(sptype);
	if (projection->getType()==pType) return;
	Projector *const ptemp = Projector::create(pType,
	                         projection->getViewport(),
	                         projection->get_fov());
	ptemp->setMaskType(projection->getMaskType());
	ptemp->setFlagGravityLabels(projection->getFlagGravityLabels());
	delete projection;
	projection = ptemp;
	glFrontFace(projection->needGlFrontFaceCW()?GL_CW:GL_CCW);
}

void Core::setFlipHorz(bool flip)
{
	projection->setFlipHorz(flip);
	glFrontFace(projection->needGlFrontFaceCW()?GL_CW:GL_CCW);
}

void Core::setFlipVert(bool flip)
{
	projection->setFlipVert(flip);
	glFrontFace(projection->needGlFrontFaceCW()?GL_CW:GL_CCW);
}

void Core::turn_right(int s)
{
	if (s && FlagEnableMoveKeys) {
		deltaAz = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else deltaAz = 0;
}

void Core::turn_left(int s)
{
	if (s && FlagEnableMoveKeys) {
		deltaAz = -1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);

	} else deltaAz = 0;
}

void Core::turn_up(int s)
{
	if (s && FlagEnableMoveKeys) {
		deltaAlt = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else deltaAlt = 0;
}

void Core::turn_down(int s)
{
	if (s && FlagEnableMoveKeys) {
		deltaAlt = -1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else deltaAlt = 0;
}


void Core::zoom_in(int s)
{
	if (FlagEnableZoomKeys) deltaFov = -1*(s!=0);
}

void Core::zoom_out(int s)
{
	if (FlagEnableZoomKeys) deltaFov = (s!=0);
}

//! Make the first screen position correspond to the second (useful for mouse dragging)
void Core::dragView(int x1, int y1, int x2, int y2)
{
	Vec3d tempvec1, tempvec2;
	double az1, alt1, az2, alt2;
	if (navigation->get_viewing_mode()==Navigator::VIEW_HORIZON) {
		projection->unproject_local(x2,getViewportHeight()-y2, tempvec2);
		projection->unproject_local(x1,getViewportHeight()-y1, tempvec1);
	} else {
		projection->unproject_earth_equ(x2,getViewportHeight()-y2, tempvec2);
		projection->unproject_earth_equ(x1,getViewportHeight()-y1, tempvec1);
	}
	rect_to_sphe(&az1, &alt1, tempvec1);
	rect_to_sphe(&az2, &alt2, tempvec2);
	navigation->update_move(projection, az2-az1, alt1-alt2, projection->get_fov());
	setFlagTracking(false);
	setFlagLockSkyPosition(false);
}

// Increment/decrement smoothly the vision field and position
void Core::updateMove(int delta_time)
{
	// the more it is zoomed, the more the mooving speed is low (in angle)
	double depl=move_speed*delta_time*projection->get_fov();
	double deplzoom=zoom_speed*delta_time*projection->get_fov();
	if (deltaAz<0) {
		deltaAz = -depl/30;
		if (deltaAz<-0.2) deltaAz = -0.2;
	} else {
		if (deltaAz>0) {
			deltaAz = (depl/30);
			if (deltaAz>0.2) deltaAz = 0.2;
		}
	}
	if (deltaAlt<0) {
		deltaAlt = -depl/30;
		if (deltaAlt<-0.2) deltaAlt = -0.2;
	} else {
		if (deltaAlt>0) {
			deltaAlt = depl/30;
			if (deltaAlt>0.2) deltaAlt = 0.2;
		}
	}

	if (deltaFov<0) {
		deltaFov = -deplzoom*5;
		if (deltaFov<-0.15*projection->get_fov()) deltaFov = -0.15*projection->get_fov();
	} else {
		if (deltaFov>0) {
			deltaFov = deplzoom*5;
			if (deltaFov>20) deltaFov = 20;
		}
	}

	//	projection->change_fov(deltaFov);
	//	navigation->update_move(projection, deltaAz, deltaAlt);

	if (deltaFov != 0 ) {
		projection->change_fov(deltaFov);
		std::ostringstream oss;
		oss << "zoom delta_fov " << deltaFov;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	}

	if (deltaAz != 0 || deltaAlt != 0) {
		navigation->update_move(projection, deltaAz, deltaAlt, projection->get_fov());
		std::ostringstream oss;
		oss << "look delta_az " << deltaAz << " delta_alt " << deltaAlt;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	} else {
		// must perform call anyway, but don't record!
		navigation->update_move(projection, deltaAz, deltaAlt, projection->get_fov());
	}
}


bool Core::setHomePlanet(string planet, float transit_time)
{

	// reset planet trails due to changed perspective
	ssystem->startTrails( ssystem->getFlagTrails() );

	// Unselect new home planet to be to avoid weirdness
	// unless above it and might want to look down at it
	/*
	  TODO: This no longer makes sense here, should go elsewhere
	if (!aboveHomePlanet && selected_object.get_type()==ObjectRecord::OBJECT_PLANET &&
		selected_object.getEnglishName()==planet)
		unSelect();
	*/

	if( planet == "selected" )
		planet = selected_object.getEnglishName();

	return observatory->setHomePlanet(planet, transit_time);
}


// For use by TUI
string Core::getPlanetHashString()
{
	return ssystem->getPlanetHashString();
}

//! Set simulation time to current real world time
void Core::setTimeNow()
{
	navigation->set_JDay(NShadeDateTime::JulianFromSys());
}

//! Get wether the current simulation time is the real world time
bool Core::getIsTimeNow(void) const
{
	// cache last time to prevent to much slow system call
	static double lastJD = getJDay();
	static bool previousResult = (fabs(getJDay()-NShadeDateTime::JulianFromSys())<JD_SECOND);
	if (fabs(lastJD-getJDay())>JD_SECOND/4) {
		lastJD = getJDay();
		previousResult = (fabs(getJDay()-NShadeDateTime::JulianFromSys())<JD_SECOND);
	}
	return previousResult;
}

//! Select passed object
//! @return true if the object was selected (false if the same was already selected)
bool Core::selectObject(const Object &obj)
{
	// Unselect if it is the same object
	if (obj && selected_object==obj) {
		unSelect();
		return true;
	}

	if (obj.get_type()==ObjectRecord::OBJECT_CONSTELLATION) {
		return selectObject(obj.getBrightestStarInConstellation().get());
	} else {
		selected_object = obj;

		// If an object has been found
		if (selected_object) {
			// If an object was selected keep the earth following
			if (getFlagTracking()) navigation->set_flag_lock_equ_pos(1);
			setFlagTracking(false);

			if (selected_object.get_type()==ObjectRecord::OBJECT_STAR) {
				asterisms->setSelected(selected_object);
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select " + selected_object.getEnglishName());
			} else {
				asterisms->setSelected(Object());
			}

			if (selected_object.get_type()==ObjectRecord::OBJECT_PLANET) {
				ssystem->setSelected(selected_object);
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select planet " + selected_object.getEnglishName());
			}

			if (selected_object.get_type()==ObjectRecord::OBJECT_NEBULA) {
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select nebula \"" + selected_object.getEnglishName() + "\"");
			}

			return true;
		} else {
			unSelect();
			return false;
		}
	}
	assert(0);	// Non reachable code
}


//! Find and return the list of at most maxNbItem objects auto-completing passed object I18 name
//! @param objPrefix the first letters of the searched object
//! @param maxNbItem the maximum number of returned object names
//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
vector<string> Core::listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	vector <string>::const_iterator iter;

	// Get matching planets
	vector<string> matchingPlanets = ssystem->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingPlanets.begin(); iter != matchingPlanets.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingPlanets.size();

	// Get matching constellations
	vector<string> matchingConstellations = asterisms->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingConstellations.begin(); iter != matchingConstellations.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingConstellations.size();

	// Get matching nebulae
	vector<string> matchingNebulae = nebulas->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingNebulae.begin(); iter != matchingNebulae.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingNebulae.size();

	// Get matching stars
	vector<string> matchingStars = hip_stars->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingStars.begin(); iter != matchingStars.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingStars.size();

	sort(result.begin(), result.end());

	return result;
}


//! font file and scaling to use for a given locale
void Core::getFontForLocale(const string &_locale, string &_fontFile, float &_fontScale,
                            string &_fixedFontFile, float &_fixedFontScale)
{

	// TODO: Cache in data structure
	// TODO: check that font files exist
	// TODO: Move to translation or font class

	// Hardcoded default fonts here (override in fontmap.dat file)
	_fontFile = _fixedFontFile = "Sans";
	_fontScale = _fixedFontScale = 1.;

	// Now see if another font should be used as default or for locale
	string mapFileName = getDataDir() + "fontmap.dat";
	ifstream *mapFile = new ifstream(mapFileName.c_str());

	if (! mapFile->is_open()) {
		cout << "WARNING: Unable to open " << mapFileName << " resorting to default fonts." << endl;
		return;
	}

	char buffer[1000];
	string locale, font, fixedFont;
	float scale, fixedScale;

	while (!mapFile->eof() && mapFile->getline (buffer,999)) {

		if ( buffer[0] != '#' && buffer[0] != 0) {

			//			printf("Buffer is: %s\n", buffer);
			istringstream record(buffer);

			record >> locale >> font >> scale >> fixedFont >> fixedScale;

			// find matching records in map file
			if (locale == _locale || locale == "default") {

				//				cout << "locale " << _locale << endl;

				_fontFile = font;
				_fontScale = scale;

				_fixedFontFile = fixedFont;
				_fixedFontScale = fixedScale;

				if (locale == _locale) {
					break;
				}
			}
		}
	}

	delete mapFile;
	return;

}



void Core::setFlagTracking(bool b)
{

	if (!b || !selected_object) {
		navigation->set_flag_traking(0);
	} else if ( !navigation->get_flag_traking()) {
		navigation->move_to(selected_object.get_earth_equ_pos(navigation),
		                    getAutomoveDuration());
		navigation->set_flag_traking(1);
	}

}






void Core::setFlagStars(bool b)
{
	hip_stars->setFlagStars(b);
}
bool Core::getFlagStars(void) const
{
	return hip_stars->getFlagStars();
}

float Core::getStarSizeLimit(void) const {
	return hip_stars->getStarSizeLimit();
}
void Core::setStarSizeLimit(float f) {
	float planet_limit = getPlanetsSizeLimit();
	hip_stars->setStarSizeLimit(f);
	setPlanetsSizeLimit(planet_limit);
}

void Core::setFlagStarName(bool b)
{
	hip_stars->setFlagNames(b);
}
bool Core::getFlagStarName(void) const
{
	return hip_stars->getFlagNames();
}

void Core::setStarLimitingMag(float f)
{
	hip_stars->setMagConverterMaxScaled60DegMag(f);
}
float Core::getStarLimitingMag(void) const
{
	return hip_stars->getMagConverterMaxScaled60DegMag();
}

void Core::setFlagStarSciName(bool b)
{
	hip_stars->setFlagSciNames(b);
}
bool Core::getFlagStarSciName(void) const
{
	return hip_stars->getFlagSciNames();
}

void Core::setFlagStarTwinkle(bool b)
{
	hip_stars->setFlagTwinkle(b);
}
bool Core::getFlagStarTwinkle(void) const
{
	return hip_stars->getFlagTwinkle();
}

void Core::setFlagPointStar(bool b)
{
	hip_stars->setFlagPointStar(b);
}
bool Core::getFlagPointStar(void) const
{
	return hip_stars->getFlagPointStar();
}

void Core::setMaxMagStarName(float f)
{
	hip_stars->setMaxMagName(f);
}
float Core::getMaxMagStarName(void) const
{
	return hip_stars->getMaxMagName();
}

void Core::setMaxMagStarSciName(float f)
{
	hip_stars->setMaxMagName(f);
}
float Core::getMaxMagStarSciName(void) const
{
	return hip_stars->getMaxMagName();
}

void Core::setStarScale(float f)
{
	hip_stars->setScale(f);
}
float Core::getStarScale(void) const
{
	return hip_stars->getScale();
}

//! Set base planets display scaling factor
//! This is additive to star size limit above
//! since makes no sense to be less
//! ONLY SET THROUGH THIS METHOD
void Core::setPlanetsSizeLimit(float f) {
	ssystem->setSizeLimit(f + getStarSizeLimit());
	hip_stars->setObjectSizeLimit(f);
	SettingsState state;
	state.m_state.planet_size_limit = f;
	SharedData::Instance()->Settings(state);
}

void Core::setStarMagScale(float f)
{
	hip_stars->setMagScale(f);
}
float Core::getStarMagScale(void) const
{
	return hip_stars->getMagScale();
}

void Core::setStarTwinkleAmount(float f)
{
	hip_stars->setTwinkleAmount(f);
}
float Core:: getStarTwinkleAmount(void) const
{
	return hip_stars->getTwinkleAmount();
}

float Core::getMagConverterMagShift(void) const {
	//the global limiting magnitude, independent of the current field of view
	return hip_stars->getMagConverterMagShift();
}
void Core::setMagConverterMagShift(float f) {
	hip_stars->setMagConverterMagShift(f);
}

float Core::getMagConverterMaxScaled60DegMag(void) const {
	// the limiting magnitude for field of view
	return hip_stars->getMagConverterMaxScaled60DegMag();
}
void Core::setMagConverterMaxScaled60DegMag(float f) {
	return hip_stars->setMagConverterMaxScaled60DegMag(f);
}

float Core::getMagConverterMaxFov(void) const {
	// the maximum field of view for which the magnitude conversion routine is used
	return hip_stars->getMagConverterMaxFov();
}
void Core::setMagConverterMaxFov(float f) {
	return hip_stars->setMagConverterMaxFov(f);
}

float Core::getMagConverterMinFov(void) const {
	// the minimum field of view for which the magnitude conversion routine is used
	return hip_stars->getMagConverterMinFov();
}
void Core::setMagConverterMinFov(float f) {
	return hip_stars->setMagConverterMinFov(f);
}

float Core::getMagConverterMaxMag(void) const {
	// the maximum field of view for which the magnitude conversion routine is used
	return hip_stars->getMagConverterMaxMag();
}
void Core::setMagConverterMaxMag(float f) {
	return hip_stars->setMagConverterMaxMag(f);
}


Vec3f Core::getColorStarNames(void) const
{
	return hip_stars->getLabelColor();
}
Vec3f Core::getColorStarCircles(void) const
{
	return hip_stars->getCircleColor();
}

void Core::setColorStarNames(const Vec3f& v)
{
	hip_stars->setLabelColor(v);
}
void Core::setColorStarCircles(const Vec3f& v)
{
	hip_stars->setCircleColor(v);
}

// set zoom/center offset (percent of fov radius)
void Core::setViewOffset(double offset)
{

	double off = offset;

	// Error checking for allowed limits
	if (offset < -0.5) off = -0.5;
	if (offset > 0.5)  off =  0.5;

	// Update default view vector
	navigation->set_view_offset(off);

	// adjust view direction (if tracking, should be corrected before render)
	navigation->set_local_vision(InitViewPos);

}

double Core::getViewOffset()
{

	return navigation->get_view_offset();

}

// set environment rotation around observer
void Core::setHeading(double heading, int duration=0)
{

	navigation->change_heading(heading, duration);

}

double Core::getHeading()
{

	return navigation->get_heading();

}

bool Core::loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
					  string name, string filename, string credit, double texture_luminance_adjust,
					  double distance )
{

	bool created = false;
	bool updateSelection = false;

	// If this is a replacement nebula, change selection to new one
	// (also avoids segfault since old was possibly deleted)
	if (selected_object.get_type()==ObjectRecord::OBJECT_NEBULA &&
		boost::iequals(selected_object.getEnglishName(), name)) {
		updateSelection = true;
		selected_object=NULL;
	}

	if(nebulas) {
		created = nebulas->loadNebula(ra, de, magnitude, angular_size, rotation,
									  name, filename, credit, texture_luminance_adjust, distance);

		if( created && updateSelection ) {
			selected_object = nebulas->search(name);
		}
	}

	return created;

}


string Core::removeNebula(const string& name)
{

	bool updateSelection = false;

	// Make sure this object is not already selected so won't crash
	if (selected_object.get_type()==ObjectRecord::OBJECT_NEBULA &&
		boost::iequals(selected_object.getEnglishName(), name) && selected_object.isDeleteable()) {

		updateSelection = true;
		selected_object=NULL;
	}

	string error = nebulas->removeNebula(name, true);

	// Try to find original version, if any
	if( updateSelection ) selected_object = nebulas->search(name);

	return error;

}

string Core::removeSupplementalNebulae()
{

	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;

	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.get_type()==ObjectRecord::OBJECT_NEBULA
	        && selected_object.isDeleteable() ) {
		unSelect();
	}

	return nebulas->removeSupplementalNebulae();

}
