/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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

#include "app.h"

#include "viewport_distorter.h"
#include "stellastro.h"
#include "utility.h"
#include "callbacks.hpp"
#include "shared_data.h"
#include <fastdb/fastdb.h>
#include <exception>
#include <Magick++.h>
#include <iomanip>
#include <boost/function.hpp>

REGISTER(ObjectRecord);
REGISTER(TZRecord);

static ConnectCallback connectCb("Bowen");

App::App( SDLFacade* const sdl ) :
		frame(0), timefr(0), timeBase(0), fps(0), maxfps(10000.f),  FlagTimePause(0),
		is_mouse_moving_horiz(false), is_mouse_moving_vert(false), draw_mode(App::DM_NONE),
		initialized(0), GMT_shift(0), m_OutputFrameNumber(0)
{
	Magick::InitializeMagick(NULL);
	// Ensure shared data structures are initialized early
	SharedData::Instance();
	AudioPlayer::Instance();

	// Construct an a function object to be called when/if a connection to cover light controller succeeds
	boost::function<void(void)> f = connectCb;
	NamedSockets::Instance().SetConnectCallback( f );
	if( AppSettings::Instance()->Digitarium() )
		NamedSockets::Instance().CreateOnCurrentSubnet( "Bowen", "245", 6005 );

	m_sdl = sdl;
	SelectedScript = SelectedScriptDirectory = "";
	AppSettings* settings = AppSettings::Instance();
	core = new Core(settings->getlDir(), settings->getDataRoot(), boost::callback<void, string>(this, &App::recordCommand));
	ui = new UI(core, this);
	commander = new AppCommandInterface(core, this);
	scripts = new ScriptMgr(commander, core->getDataDir());
	time_multiplier = 1;
	distorter = 0;
	cout.precision(16);
}


App::~App()
{
	delete ui;
	delete scripts;
	delete commander;
	delete core;
	if (distorter) delete distorter;

	NamedSockets::Clear();
	// This should be last, other objects may try to hit
	// shared data on destruction
	SharedData::Destroy();
}

void App::setViewPortDistorterType(const string &type)
{
	Uint16 w, h;
	m_sdl->getResolution( &w, &h );

	if (distorter) {
		if (distorter->getType() == type) return;
		delete distorter;
		distorter = 0;
	}
	distorter = ViewportDistorter::create(type, w, h, core);
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());
	distorter->init(conf);
}

string App::getViewPortDistorterType(void) const
{
	if (distorter) return distorter->getType();
	return "none";
}

string App::getVideoModeList(void) const {
	return m_sdl->getVideoModeList();
}


void App::quit(void)
{
	static SDL_Event Q;						// Send a SDL_QUIT event
	Q.type = SDL_QUIT;						// To the SDL event queue
	if (SDL_PushEvent(&Q) == -1) {			// Try to send the event
		printf("SDL_QUIT event can't be pushed: %s\n", SDL_GetError() );
		exit(-1);
	}
}

// Load configuration from disk

void App::init(void)
{
	if (!initialized)
		Translator::initSystemLanguage();

	// Initialize video device and other sdl parameters
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	// forced settings
	conf.set_double("video:horizontal_offset", 0);
	conf.set_double("video:vertical_offset", 0);
	//conf.set_double("navigation:init_fov", 180);

#ifndef DESKTOP
	conf.set_boolean("navigation:flag_enable_move_mouse", 0);
	conf.set_double("viewing:constellation_art_fade_duration", 2);
	conf.set_double("landscape:constellation_art_fade_duration", 4);
	conf.set_double("gui:base_font_size", 12);
#endif

	// Clear screen, this fixes a strange artifact at loading time in the upper top corner.
	glClear(GL_COLOR_BUFFER_BIT);

	maxfps 				= conf.get_double ("video","maximum_fps",10000);
	minfps 				= conf.get_double ("video","minimum_fps",10000);
	videoRecordFps		= conf.get_double ("video","video_record_fps",30);
	string appLocaleName = conf.get_str("localization", "app_locale", "system");
	time_format = string_to_s_time_format(conf.get_str("localization:time_display_format"));
	date_format = string_to_s_date_format(conf.get_str("localization:date_display_format"));
	setAppLanguage(appLocaleName);
	scripts->set_allow_ui( conf.get_boolean("gui","flag_script_allow_ui",0) );

	// time_zone used to be in init_location section of config,
	// so use that as fallback when reading config - Rob
	string tzstr = conf.get_str("localization", "time_zone", conf.get_str("init_location", "time_zone", "system_default"));
	if (tzstr == "system_default") {
		time_zone_mode = S_TZ_SYSTEM_DEFAULT;
		// Set the program global intern timezones variables from the system locale
		tzset();
	} else {
		if (tzstr == "gmt+x") { // TODO : handle GMT+X timezones form
			time_zone_mode = S_TZ_GMT_SHIFT;
			// GMT_shift = x;
		} else {
			// We have a custom time zone name
			time_zone_mode = S_TZ_CUSTOM;
			set_custom_tz_name(tzstr);
		}
	}

	Uint16 w, h;
	m_sdl->getResolution( &w, &h );
	core->init(conf, w, h);

	string tmpstr = conf.get_str("projection:viewport");
	if (tmpstr=="maximized") core->setMaximizedViewport(w, h);
	else
		//	  if (tmpstr=="square" || tmpstr=="disk") {

		if (tmpstr=="square") {
			core->setSquareViewport(w, h,
			                        conf.get_int("video:horizontal_offset"),
			                        conf.get_int("video:horizontal_offset"));

		} else if (tmpstr=="disk") {
			core->setViewportMaskDisk(w/2, h/2, w, h, h/2);

		} else {
			cerr << "ERROR : Unknown viewport type : " << tmpstr << endl;
			exit(-1);
		}

	// Digitalis projector configuration type, MUST call after setting disk viewport above
#ifdef DESKTOP
	// do not use truncated as default
	int projConf = conf.get_int("projection", "projector_configuration", 1);
# else
	int projConf = conf.get_int("projection", "projector_configuration", 0);
#endif

	core->setProjectorConfiguration( projConf );

	core->setProjectorOffset( conf.get_double("projection", "projector_offset_x", 0),
	                          conf.get_double("projection", "projector_offset_y", 0));

	core->setProjectorShearHorz(conf.get_double("projection", "horizontal_shear", 1));

	core->setCenterHorizontalOffset(conf.get_int("video", "center_horizontal_offset", 0));



	// Navigation section
	PresetSkyTime 		= conf.get_double ("navigation","preset_sky_time",2451545.);
	StartupTimeMode 	= conf.get_str("navigation:startup_time_mode");	// Can be "now" or "preset"
	FlagEnableMoveMouse	= conf.get_boolean("navigation","flag_enable_move_mouse",1);
	MouseZoom			= conf.get_int("navigation","mouse_zoom",30);

	DayKeyMode = conf.get_str("navigation","day_key_mode","calendar");  // calendar or sidereal
	//	cout << "Read daykeymode as <" << DayKeyMode << ">\n";

	if (StartupTimeMode=="preset" || StartupTimeMode=="Preset")
		core->setJDay(PresetSkyTime - get_GMT_shift(PresetSkyTime) * JD_HOUR);
	else core->setTimeNow();

	// initialisation of the User Interface

	// TODO: Need way to update settings from config without reinitializing whole gui
	ui->init(conf);

	if (!initialized) ui->init_tui(); // don't reinit tui since probably called from there
	else ui->localizeTui();  // update translations/fonts as needed

	// Initialisation of the color scheme
	draw_mode = App::DM_NONE;  // fool caching
	setVisionModeNormal();
	if (conf.get_boolean("viewing:flag_chart")) setVisionModeChart();
	if (conf.get_boolean("viewing:flag_night")) setVisionModeNight();

	if (distorter == 0) {
		setViewPortDistorterType(conf.get_str("video","distorter","none"));
	}

#ifndef DESKTOP
	// system integration hook
	system((core->getDataDir() + "script_app_init").c_str());

	// This did not seem to be required
	// and should be done by script_mgr anyway
	//	remove(SCRIPT_IS_PLAYING_FILE);
#endif

	// play startup script, if available
	if (!initialized && scripts) scripts->play_startup_script();

	initialized = 1;

	// Make 'static' data available to shared memory clients
	SharedData::Instance()->CopyStaticData();

//	cout << "Init complete\n";
}

void App::UpdateSharedData( void ) {
	string cmdData, response;
	ISharedData* shared = SharedData::Instance();

	bool have_sych_data = shared->Read( cmdData );
	unsigned long delay = 0;
	if( have_sych_data ) {
		// Execute command untrusted
		if( !commander->execute_command(cmdData, delay, 1) ) {
			response = commander->getErrorString();
			if( response.empty() )
				response = _("Unrecognized or malformed command name.");
		}
		else
			response = _("Success");

		shared->Write(response);
	}

	// Asynchronous channel
	bool have_asynch_data = shared->ReadRT( cmdData );
	if( have_asynch_data ) {
		if( !m_skyCommander.execute_command(cmdData) ) {
			commander->execute_command(cmdData, delay, 1);
		}
	}

	// Nightshade 'dynamic' state push
	shared->DataPush();
}

void App::update(int delta_time)
{
	++frame;
	timefr+=delta_time;
	if (timefr-timeBase > 1000) {
		fps=frame*1000.0/(timefr-timeBase);				// Calc the FPS rate
		frame = 0;
		timeBase+=1000;
	}

	// change time rate if needed to fast forward scripts
	delta_time *= time_multiplier;

	// keep audio position updated if changing time multiplier
	if (!scripts->is_paused()) commander->update(delta_time);

	// run command from a running script
	scripts->update(delta_time);

	// run any incoming command from shared memory interface
	UpdateSharedData();

	ui->gui_update_widgets(delta_time);
	ui->tui_update_widgets();

	if (!scripts->is_paused()) {
		ImageMgr::getImageMgr("script").update(delta_time);
		ImageMgr::getImageMgr("media").update(delta_time);
	}

	core->update(delta_time);
}

//! Main drawinf function called at each frame
double App::draw(int delta_time)
{
	Uint16 w, h;
	m_sdl->getResolution( &w, &h );
// - 20061020 - clear areas not covered by main viewport
	glDisable(GL_BLEND);
	glColor3f(0.f,0.f,0.f);
	set2DfullscreenProjection();
	glBegin(GL_QUADS);
	{
		glVertex2f(0,h);
		glVertex2f(w,h);
		glVertex2f(w,0);
		glVertex2f(0,0);
	}
	glEnd();
	restoreFrom2DfullscreenProjection();

	// Render all the main objects of the application
	double squaredDistance = core->draw(delta_time);

	// Draw the Graphical ui and the Text ui
	ui->draw();

	distorter->distort();

	return squaredDistance;
}

//! @brief Set the application locale. This apply to GUI, console messages etc..
void App::setAppLanguage(const std::string& newAppLocaleName)
{
	// Update the translator with new locale name
	Translator::globalTranslator = Translator(PACKAGE, core->getLocaleDir(), newAppLocaleName);
	cout << "Application locale is " << Translator::globalTranslator.getLocaleName() << endl;

	// update translations and font in tui
	ui->localizeTui();

	SettingsState state;
	strcpy( state.m_state.selected_language, newAppLocaleName.c_str() );
	SharedData::Instance()->Settings( state );

	// TODO: GUI needs to be reinitialized to load new translations and/or fonts
}

// Handle mouse clics
int App::handleClick(int x, int y, S_GUI_VALUE button, S_GUI_VALUE state)
{
	distorter->distortXY(x,y);
	return ui->handle_clic(x, y, button, state);
}

// Handle mouse move
int App::handleMove(int x, int y)
{
	distorter->distortXY(x,y);
	// Turn if the mouse is at the edge of the screen.
	// unless config asks otherwise
	if (FlagEnableMoveMouse) {
		if (x == 0) {
			core->turn_left(1);
			is_mouse_moving_horiz = true;
		} else if (x == core->getViewportWidth() - 1) {
			core->turn_right(1);
			is_mouse_moving_horiz = true;
		} else if (is_mouse_moving_horiz) {
			core->turn_left(0);
			is_mouse_moving_horiz = false;
		}

		if (y == 0) {
			core->turn_up(1);
			is_mouse_moving_vert = true;
		} else if (y == core->getViewportHeight() - 1) {
			core->turn_down(1);
			is_mouse_moving_vert = true;
		} else if (is_mouse_moving_vert) {
			core->turn_up(0);
			is_mouse_moving_vert = false;
		}
	}

	return ui->handle_move(x, y);

}

// Handle key press and release
int App::handleKeys(SDLKey key, SDLMod mod,
                    Uint16 unicode, s_gui::S_GUI_VALUE state)
{
	int retVal = 0;

	s_tui::S_TUI_VALUE tuiv;
	if (state == s_gui::S_GUI_PRESSED) tuiv = s_tui::S_TUI_PRESSED;
	else tuiv = s_tui::S_TUI_RELEASED;
	if (ui->FlagShowTuiMenu) {

		if (state==S_GUI_PRESSED && unicode=='m') {
			// leave tui menu
			ui->FlagShowTuiMenu = false;

			// If selected a script in tui, run that now
			if (SelectedScript!="")
// add to svn to allow spaces in scripts and dirs
				commander->execute_command("script action play filename \"" +  SelectedScript
				                           + "\" path \"" + SelectedScriptDirectory + "\"");

			// clear out now
			SelectedScriptDirectory = SelectedScript = "";
			return 1;
		}

		if( ui->handle_keys_tui(unicode, tuiv) )
			return 1;

		return 0;
	}

	if (ui->handle_keys(key, mod, unicode, state))
		retVal = 1;

	if (state == S_GUI_PRESSED) {
		// Direction and zoom deplacements
		if (key==SDLK_LEFT) core->turn_left(1);
		if (key==SDLK_RIGHT) core->turn_right(1);
		if (key==SDLK_UP) {
			if (mod & KMOD_CTRL) core->zoom_in(1);
			else core->turn_up(1);
		}
		if (key==SDLK_DOWN) {
			if (mod & KMOD_CTRL) core->zoom_out(1);
			else core->turn_down(1);
		}
		if (key==SDLK_PAGEUP) core->zoom_in(1);
		if (key==SDLK_PAGEDOWN) core->zoom_out(1);
	} else {
		// When a deplacement key is released stop mooving
		if (key==SDLK_LEFT) core->turn_left(0);
		if (key==SDLK_RIGHT) core->turn_right(0);
		if (mod & KMOD_CTRL) {
			if (key==SDLK_UP) core->zoom_in(0);
			if (key==SDLK_DOWN) core->zoom_out(0);
		} else {
			if (key==SDLK_UP) core->turn_up(0);
			if (key==SDLK_DOWN) core->turn_down(0);
		}
		if (key==SDLK_PAGEUP) core->zoom_in(0);
		if (key==SDLK_PAGEDOWN) core->zoom_out(0);
	}
	return retVal;
}


//! Set the drawing mode in 2D for drawing in the full screen
void App::set2DfullscreenProjection(void) const
{
	Uint16 w, h;
	m_sdl->getResolution( &w, &h );
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);		// projection matrix mode
	glPushMatrix();						// store previous matrix
	glLoadIdentity();
	gluOrtho2D(	0, w,
	            0, h);			// set a 2D orthographic projection
	glMatrixMode(GL_MODELVIEW);			// modelview matrix mode
	glPushMatrix();
	glLoadIdentity();
}

//! Restore previous projection mode
void App::restoreFrom2DfullscreenProjection(void) const
{
	glMatrixMode(GL_PROJECTION);		// Restore previous matrix
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

//! Set flag for activating night vision mode
void App::setVisionModeNight(void)
{
	if (!getVisionModeNight()) {
		core->setColorScheme(AppSettings::Instance()->getConfigFile(), "night_color");
		ui->setColorScheme(AppSettings::Instance()->getConfigFile(), "night_color");
	}
	draw_mode=DM_NIGHT;
}

//! Set flag for activating chart vision mode
void App::setVisionModeChart(void)
{
	if (!getVisionModeChart()) {
		core->setColorScheme(AppSettings::Instance()->getConfigFile(), "chart_color");
		ui->setColorScheme(AppSettings::Instance()->getConfigFile(), "chart_color");
	}
	draw_mode=DM_CHART;
}

//! Set flag for activating chart vision mode
// ["color" section name used for easier backward compatibility for older configs - Rob]
void App::setVisionModeNormal(void)
{
	if (!getVisionModeNormal()) {
		core->setColorScheme(AppSettings::Instance()->getConfigFile(), "color");
		ui->setColorScheme(AppSettings::Instance()->getConfigFile(), "color");
	}
	draw_mode=DM_NORMAL;
}

double App::getMouseCursorTimeout()
{
	return ui->getMouseCursorTimeout();
}

// For use by TUI - saves all current settings
// TODO: Put in stel_core?

void App::saveCurrentConfig(const string& confFile)
{

	// No longer resaves everything, just settings user can change through UI

	cout << "Saving configuration file " << confFile << " ..." << endl;
	InitParser conf;
	conf.load(confFile);

	// Main section
	conf.set_str	("main:version", string(VERSION));

	// localization section
	conf.set_str    ("localization:sky_culture", core->getSkyCultureDir());
	conf.set_str    ("localization:sky_locale", core->getSkyLanguage());
	conf.set_str    ("localization:app_locale", getAppLanguage());
	conf.set_str	("localization:time_display_format", get_time_format_str());
	conf.set_str	("localization:date_display_format", get_date_format_str());
	if (time_zone_mode == S_TZ_CUSTOM) {
		conf.set_str("localization:time_zone", custom_tz_name);
	}
	if (time_zone_mode == S_TZ_SYSTEM_DEFAULT) {
		conf.set_str("localization:time_zone", "system_default");
	}
	if (time_zone_mode == S_TZ_GMT_SHIFT) {
		conf.set_str("localization:time_zone", "gmt+x");
	}

	// Rendering section
	conf.set_boolean("rendering:flag_antialias_lines", core->getFlagAntialiasLines());
	conf.set_double("rendering:line_width", core->getLineWidth());

	// viewing section
	conf.set_boolean("viewing:flag_constellation_drawing", core->getFlagConstellationLines());
	conf.set_boolean("viewing:flag_constellation_name", core->getFlagConstellationNames());
	conf.set_boolean("viewing:flag_constellation_art", core->getFlagConstellationArt());
	conf.set_boolean("viewing:flag_constellation_boundaries", core->getFlagConstellationBoundaries());
	conf.set_boolean("viewing:flag_constellation_pick", core->getFlagConstellationIsolateSelected());
	conf.set_double("viewing:moon_scale", core->getMoonScale());
	//conf.set_boolean("viewing:use_common_names", FlagUseCommonNames);
	conf.set_boolean("viewing:flag_equatorial_grid", core->getFlagEquatorGrid());
	conf.set_boolean("viewing:flag_azimutal_grid", core->getFlagAzimutalGrid());
	conf.set_boolean("viewing:flag_galactic_grid", core->getFlagGalacticGrid());
	conf.set_boolean("viewing:flag_equator_line", core->getFlagEquatorLine());
	conf.set_boolean("viewing:flag_ecliptic_line", core->getFlagEclipticLine());
	conf.set_boolean("viewing:flag_cardinal_points", core->getFlagCardinalsPoints());
	conf.set_boolean("viewing:flag_meridian_line", core->getFlagMeridianLine());
	conf.set_boolean("viewing:flag_precession_circle", core->getFlagPrecessionCircle());
	conf.set_boolean("viewing:flag_circumpolar_circle", core->getFlagCircumpolarCircle());
	conf.set_boolean("viewing:flag_tropic_lines", core->getFlagTropicLines());
	conf.set_boolean("viewing:flag_moon_scaled", core->getFlagMoonScaled());
	conf.set_double ("viewing:constellation_art_intensity", core->getConstellationArtIntensity());
	conf.set_double ("viewing:constellation_art_fade_duration", core->getConstellationArtFadeDuration());
	conf.set_double("viewing:light_pollution_limiting_magnitude", core->getLightPollutionLimitingMagnitude());

	// Landscape section
	conf.set_boolean("landscape:flag_landscape", core->getFlagLandscape());
	conf.set_boolean("landscape:flag_atmosphere", core->getFlagAtmosphere());
	conf.set_boolean("landscape:flag_fog", core->getFlagFog());
	//	conf.set_double ("viewing:atmosphere_fade_duration", core->getAtmosphereFadeDuration());

	// Star section
	conf.set_double ("stars:star_scale", core->getStarScale());
	conf.set_double ("stars:star_mag_scale", core->getStarMagScale());
	conf.set_boolean("stars:flag_point_star", core->getFlagPointStar());
	conf.set_double("stars:max_mag_star_name", core->getMaxMagStarName());
	conf.set_boolean("stars:flag_star_twinkle", core->getFlagStarTwinkle());
	conf.set_double("stars:star_twinkle_amount", core->getStarTwinkleAmount());
	conf.set_double("stars:star_limiting_mag", core->getStarLimitingMag());

	// Color section
	conf.set_str    ("color:azimuthal_color", Utility::vec3f_to_str(core->getColorAzimutalGrid()));
	conf.set_str    ("color:galactic_color", Utility::vec3f_to_str(core->getColorGalacticGrid()));
	conf.set_str    ("color:equatorial_color", Utility::vec3f_to_str(core->getColorEquatorGrid()));
	conf.set_str    ("color:equator_color", Utility::vec3f_to_str(core->getColorEquatorLine()));
	conf.set_str    ("color:ecliptic_color", Utility::vec3f_to_str(core->getColorEclipticLine()));
	conf.set_str    ("color:meridian_color", Utility::vec3f_to_str(core->getColorMeridianLine()));
	conf.set_str    ("color:const_lines_color", Utility::vec3f_to_str(core->getColorConstellationLine()));
	conf.set_str    ("color:const_names_color", Utility::vec3f_to_str(core->getColorConstellationNames()));
	conf.set_str    ("color:const_art_color", Utility::vec3f_to_str(core->getColorConstellationArt()));
	conf.set_str    ("color:const_boundary_color", Utility::vec3f_to_str(core->getColorConstellationBoundaries()));
	conf.set_str	("color:nebula_label_color", Utility::vec3f_to_str(core->getColorNebulaLabels()));
	conf.set_str	("color:nebula_circle_color", Utility::vec3f_to_str(core->getColorNebulaCircle()));
	conf.set_str	("color:precession_circle_color", Utility::vec3f_to_str(core->getColorPrecessionCircle()));
	conf.set_str	("color:circumpolar_circle_color", Utility::vec3f_to_str(core->getColorCircumpolarCircle()));
	conf.set_str    ("color:cardinal_color", Utility::vec3f_to_str(core->getColorCardinalPoints()));
	conf.set_str    ("color:planet_names_color", Utility::vec3f_to_str(core->getColorPlanetsNames()));
	conf.set_str    ("color:planet_orbits_color", Utility::vec3f_to_str(core->getColorPlanetsOrbits()));
	conf.set_str    ("color:satellite_orbits_color", Utility::vec3f_to_str(core->getColorSatelliteOrbits()));
	conf.set_str    ("color:object_trails_color", Utility::vec3f_to_str(core->getColorPlanetsTrails()));
	//  Are these used?
	//	conf.set_str    ("color:star_label_color", Utility::vec3f_to_str(core->getColorStarNames()));
	//  conf.set_str    ("color:star_circle_color", Utility::vec3f_to_str(core->getColorStarCircles()));

	// gui section
	conf.set_double("gui:mouse_cursor_timeout",getMouseCursorTimeout());

	// not user settable yet
	// conf.set_str	("gui:gui_base_color", Utility::vec3f_to_str(GuiBaseColor));
	// conf.set_str	("gui:gui_text_color", Utility::vec3f_to_str(GuiTextColor));
	// conf.set_double ("gui:base_font_size", BaseFontSize);

	// Text ui section
	conf.set_boolean("tui:flag_show_gravity_ui", ui->getFlagShowGravityUi());
	conf.set_boolean("tui:flag_show_tui_datetime", ui->getFlagShowTuiDateTime());
	conf.set_boolean("tui:flag_show_tui_short_obj_info", ui->getFlagShowTuiShortObjInfo());

	// Navigation section
	conf.set_boolean("navigation:flag_manual_zoom", core->getFlagManualAutoZoom());
	conf.set_double ("navigation:auto_move_duration", core->getAutoMoveDuration());
	conf.set_double ("navigation:zoom_speed", core->getZoomSpeed());
	conf.set_double ("navigation:flight_duration", core->getFlightDuration());
	conf.set_double ("navigation:preset_sky_time", PresetSkyTime);
	conf.set_str	("navigation:startup_time_mode", StartupTimeMode);
	conf.set_str	("navigation:day_key_mode", DayKeyMode);

	conf.set_double ("navigation:heading", core->getHeading());
	core->getNavigation()->set_defaultHeading(core->getHeading());
	conf.set_double ("navigation:view_offset", core->getViewOffset());

	// Astro section
	conf.set_boolean("astro:flag_object_trails", core->getFlagPlanetsTrails());
	conf.set_boolean("astro:flag_bright_nebulae", core->getFlagBrightNebulae());
	conf.set_boolean("astro:flag_stars", core->getFlagStars());
	conf.set_boolean("astro:flag_star_name", core->getFlagStarName());
	conf.set_boolean("astro:flag_nebula", core->getFlagNebula());
	conf.set_boolean("astro:flag_nebula_name", core->getFlagNebulaHints());
	conf.set_double("astro:max_mag_nebula_name", core->getNebulaMaxMagHints());
	conf.set_boolean("astro:flag_planets", core->getFlagPlanets());
	conf.set_boolean("astro:flag_planets_hints", core->getFlagPlanetsHints());
	conf.set_boolean("astro:flag_planets_orbits", core->getFlagPlanetsOrbits());
	conf.set_boolean("astro:flag_light_travel_time", core->getFlagLightTravelTime());

	conf.set_boolean("astro:flag_milky_way", core->getFlagMilkyWay());
	conf.set_double("astro:milky_way_intensity", core->getMilkyWayIntensity());

	conf.set_double("astro:star_size_limit", core->getStarSizeLimit());
	conf.set_double("astro:planet_size_marginal_limit", core->getPlanetsSizeLimit());

// projection section
	conf.set_int("projection:projector_configuration", core->getProjectorConfiguration());
	conf.set_double("projection:projector_offset_x", core->getProjectorOffsetX());
	conf.set_double("projection:projector_offset_y", core->getProjectorOffsetY());

	conf.set_double("projection:horizontal_shear", core->getProjectorShearHorz() );

	conf.set_int("video:center_horizontal_offset", core->getCenterHorizontalOffset() );


	// Get landscape and other observatory info
	// TODO: shouldn't observer already know what section to save in?
	(core->getObservatory())->setConf(conf, "init_location");

	conf.save(confFile);

}


void App::recordCommand(string commandline)
{
	scripts->record_command(commandline);
}


// Return a string with the UTC date formated according to the date_format variable
string App::get_printable_date_UTC(double JD) const
{
	struct tm time_utc;
	NShadeDateTime::TimeTmFromJulian(JD, &time_utc);

	static char date[255];
	switch (date_format) {
	case S_DATE_SYSTEM_DEFAULT :
		NShadeDateTime::my_strftime(date, 254, "%x", &time_utc);
		break;
	case S_DATE_MMDDYYYY :
		NShadeDateTime::my_strftime(date, 254, "%m/%d/%Y", &time_utc);
		break;
	case S_DATE_DDMMYYYY :
		NShadeDateTime::my_strftime(date, 254, "%d/%m/%Y", &time_utc);
		break;
	case S_DATE_YYYYMMDD :
		NShadeDateTime::my_strftime(date, 254, "%Y-%m-%d", &time_utc);
		break;
	}
	return date;
}

// Return a string with the UTC time formated according to the time_format variable
// TODO : for some locales (french) the %p returns nothing
string App::get_printable_time_UTC(double JD) const
{
	struct tm time_utc;
	NShadeDateTime::TimeTmFromJulian(JD, &time_utc);

	static char heure[255];
	switch (time_format) {
	case S_TIME_SYSTEM_DEFAULT :
		NShadeDateTime::my_strftime(heure, 254, "%X", &time_utc);
		break;
	case S_TIME_24H :
		NShadeDateTime::my_strftime(heure, 254, "%H:%M:%S", &time_utc);
		break;
	case S_TIME_12H :
		NShadeDateTime::my_strftime(heure, 254, "%I:%M:%S %p", &time_utc);
		break;
	}
	return heure;
}

// Return the time in ISO 8601 format that is : %Y-%m-%d %H:%M:%S
string App::get_ISO8601_time_local(double JD) const
{
	struct tm time_local;
	if (time_zone_mode == S_TZ_GMT_SHIFT)
		NShadeDateTime::TimeTmFromJulian(JD + GMT_shift, &time_local);
	else
		NShadeDateTime::TimeTmFromJulian(JD + NShadeDateTime::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char isotime[255];
	NShadeDateTime::my_strftime(isotime, 254, "%Y-%m-%d %H:%M:%S", &time_local);
	return isotime;
}


// Return a string with the local date formated according to the date_format variable
string App::get_printable_date_local(double JD) const
{
	struct tm time_local;

	if (time_zone_mode == S_TZ_GMT_SHIFT)
		NShadeDateTime::TimeTmFromJulian(JD + GMT_shift, &time_local);
	else
		NShadeDateTime::TimeTmFromJulian(JD + NShadeDateTime::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char date[255];
	switch (date_format) {
	case S_DATE_SYSTEM_DEFAULT :
		NShadeDateTime::my_strftime(date, 254, "%x", &time_local);
		break;
	case S_DATE_MMDDYYYY :
		NShadeDateTime::my_strftime(date, 254, "%m/%d/%Y", &time_local);
		break;
	case S_DATE_DDMMYYYY :
		NShadeDateTime::my_strftime(date, 254, "%d/%m/%Y", &time_local);
		break;
	case S_DATE_YYYYMMDD :
		NShadeDateTime::my_strftime(date, 254, "%Y-%m-%d", &time_local);
		break;
	}

	return date;
}

// Return a string with the local time (according to time_zone_mode variable) formated
// according to the time_format variable
string App::get_printable_time_local(double JD) const
{
	struct tm time_local;

	if (time_zone_mode == S_TZ_GMT_SHIFT)
		NShadeDateTime::TimeTmFromJulian(JD + GMT_shift, &time_local);
	else
		NShadeDateTime::TimeTmFromJulian(JD + NShadeDateTime::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char heure[255];
	switch (time_format) {
	case S_TIME_SYSTEM_DEFAULT :
		NShadeDateTime::my_strftime(heure, 254, "%X", &time_local);
		break;
	case S_TIME_24H :
		NShadeDateTime::my_strftime(heure, 254, "%H:%M:%S", &time_local);
		break;
	case S_TIME_12H :
		NShadeDateTime::my_strftime(heure, 254, "%I:%M:%S %p", &time_local);
		break;
	}
	return heure;
}

// Convert the time format enum to its associated string and reverse
App::S_TIME_FORMAT App::string_to_s_time_format(const string& tf) const
{
	if (tf == "system_default") return S_TIME_SYSTEM_DEFAULT;
	if (tf == "24h") return S_TIME_24H;
	if (tf == "12h") return S_TIME_12H;
	cout << "ERROR : unrecognized time_display_format : " << tf << " system_default used." << endl;
	return S_TIME_SYSTEM_DEFAULT;
}

string App::s_time_format_to_string(S_TIME_FORMAT tf) const
{
	if (tf == S_TIME_SYSTEM_DEFAULT) return "system_default";
	if (tf == S_TIME_24H) return "24h";
	if (tf == S_TIME_12H) return "12h";
	cout << "ERROR : unrecognized time_display_format value : " << tf << " system_default used." << endl;
	return "system_default";
}

// Convert the date format enum to its associated string and reverse
App::S_DATE_FORMAT App::string_to_s_date_format(const string& df) const
{
	if (df == "system_default") return S_DATE_SYSTEM_DEFAULT;
	if (df == "mmddyyyy") return S_DATE_MMDDYYYY;
	if (df == "ddmmyyyy") return S_DATE_DDMMYYYY;
	if (df == "yyyymmdd") return S_DATE_YYYYMMDD;  // iso8601
	cout << "ERROR : unrecognized date_display_format : " << df << " system_default used." << endl;
	return S_DATE_SYSTEM_DEFAULT;
}

string App::s_date_format_to_string(S_DATE_FORMAT df) const
{
	if (df == S_DATE_SYSTEM_DEFAULT) return "system_default";
	if (df == S_DATE_MMDDYYYY) return "mmddyyyy";
	if (df == S_DATE_DDMMYYYY) return "ddmmyyyy";
	if (df == S_DATE_YYYYMMDD) return "yyyymmdd";
	cout << "ERROR : unrecognized date_display_format value : " << df << " system_default used." << endl;
	return "system_default";
}

void App::set_custom_tz_name(const string& tzname)
{
	custom_tz_name = tzname;
	time_zone_mode = S_TZ_CUSTOM;

	if ( custom_tz_name != "") {
		ObserverState state;
		strncpy( state.tz, tzname.c_str(), sizeof(state.tz) );
		state.tz[sizeof(state.tz)-1] = '\0';
		SharedData::Instance()->Observer(state);

		// set the TZ environement variable and update c locale stuff
		putenv(strdup((string("TZ=") + custom_tz_name).c_str()));
		tzset();
	}
}

float App::get_GMT_shift(double JD, bool _local) const
{
	if (time_zone_mode == S_TZ_GMT_SHIFT) return GMT_shift;
	else return NShadeDateTime::GMTShiftFromSystem(JD,_local);
}

// Terminate the application
void App::terminateApplication(void)
{
	static SDL_Event Q;						// Send a SDL_QUIT event
	Q.type = SDL_QUIT;						// To the SDL event queue
	if (SDL_PushEvent(&Q) == -1) {			// Try to send the event
		printf("SDL_QUIT event can't be pushed: %s\n", SDL_GetError() );
		exit(-1);
	}
}

enum {
	USER_EVENT_TICK
};

Uint32 mytimer_callback(Uint32 interval, void *param)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.type = SDL_USEREVENT;
	event.user.code = USER_EVENT_TICK;
	event.user.data1 = NULL;
	event.user.data2 = NULL;
	if (SDL_PushEvent(&event) == -1) {
		printf("User tick event can't be pushed: %s\n", SDL_GetError() );
		exit(-1);
	}

	// End this timer.
	return 0;
}

void App::start_main_loop()
{
	bool AppVisible = true;			// At The Beginning, Our App Is Visible
	enum S_GUI_VALUE bt;
	Uint32 last_event_time = SDL_GetTicks();
	// How fast the objects on-screen are moving, in pixels/millisecond.
	double animationSpeed = 0;
	SDL_Event	E;
	// Hold the value of SDL_GetTicks at start of main loop (set 0 time)
	static Uint32 LastCount = SDL_GetTicks();
	static Uint32 TickCount = 0;

	// Start the main loop
	while (1) {
		if (SDL_PollEvent(&E)) {	// Fetch The First Event Of The Queue
			if (E.type != SDL_USEREVENT) {
				last_event_time = SDL_GetTicks();
			}

			switch (E.type) {	// And Processing It
			case SDL_QUIT:
				ImageMgr::cleanUp();
				return;
				break;

			case SDL_VIDEORESIZE:
				// Recalculate The OpenGL Scene Data For The New Window
				if (E.resize.h && E.resize.w) core->setViewportSize(E.resize.w, E.resize.h);
				break;

			case SDL_ACTIVEEVENT:
				if (E.active.state & SDL_APPACTIVE) {
					// Activity level changed (ie. iconified)
					if (E.active.gain) AppVisible = true; // Activity's been gained
					else AppVisible = false;
				}
				break;

			case SDL_MOUSEMOTION:
				handleMove(E.motion.x,E.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				// Convert the name from GLU to my GUI
				switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=S_GUI_MOUSE_MIDDLE;
					break;
				case SDL_BUTTON_WHEELUP :
					bt=S_GUI_MOUSE_WHEELUP;
					break;
				case SDL_BUTTON_WHEELDOWN :
					bt=S_GUI_MOUSE_WHEELDOWN;
					break;
				default :
					bt=S_GUI_MOUSE_LEFT;
				}
				handleClick(E.button.x,E.button.y,bt,S_GUI_PRESSED);
				break;

			case SDL_MOUSEBUTTONUP:
				// Convert the name from GLU to my GUI
				switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=S_GUI_MOUSE_MIDDLE;
					break;
				case SDL_BUTTON_WHEELUP :
					bt=S_GUI_MOUSE_WHEELUP;
					break;
				case SDL_BUTTON_WHEELDOWN :
					bt=S_GUI_MOUSE_WHEELDOWN;
					break;
				default :
					bt=S_GUI_MOUSE_LEFT;
				}
				handleClick(E.button.x,E.button.y,bt,S_GUI_RELEASED);
				break;

			case SDL_KEYDOWN:
				/* Fumio patch... can't use because ignores unicode values and hence is US keyboard specific.
				  - what was the reasoning?
				if ((E.key.keysym.sym >= SDLK_LAST && !core->handle_keys(E.key.keysym.unicode,S_GUI_PRESSED)) ||
				(E.key.keysym.sym < SDLK_LAST && !core->handle_keys(E.key.keysym.sym,S_GUI_PRESSED)))
				*/
#ifdef DESKTOP
				// shift-ctrl-v starts recording video
				if (E.key.keysym.sym==SDLK_v && (E.key.keysym.mod & COMPATIBLE_KMOD_CTRL) && 
					(E.key.keysym.mod & KMOD_SHIFT)) {
					if(m_OutputFrameNumber) {
						m_OutputFrameNumber = 0;
						ui->show_message(_("Stopped recording video frames."), 5000);  // TODO not drawing
						cout << _("Stopped recording video frames.") << endl;
					} else {
						m_OutputFrameNumber = 1;
						ui->show_message(_("Now recording video frames!\nPress CTRL-SHIFT-V to stop."), 1000);
					}

					break;
				}

				// ctrl-s saves screenshot
				if (E.key.keysym.unicode==0x0013 &&  (m_sdl->getScreen()->flags & SDL_OPENGL)) {
					string tempName = getNextScreenshotFilename();
					writeScreenshot(tempName);
					cout << _("Saved screenshot to file: ") << tempName << endl;

					break;
				}
#endif
#ifdef LSS
				if ((E.key.keysym.sym==SDLK_F11) && (E.key.keysym.mod & COMPATIBLE_KMOD_CTRL) && (E.key.keysym.mod & KMOD_SHIFT)) {
#else
				if (E.key.keysym.sym==SDLK_F11) {
#endif
					SDL_WM_ToggleFullScreen(m_sdl->getScreen()); // Try fullscreen
					break;
				}

				// Rescue escape in case of lock : CTRL + ESC forces brutal quit
				if (E.key.keysym.sym==SDLK_ESCAPE && (SDL_GetModState() & KMOD_CTRL)) {
					terminateApplication();
					break;
				}

				// Send the event to the gui and stop if it has been intercepted
				// use unicode translation, since not keyboard dependent
				// however, for non-printing keys must revert to just keysym... !
				if (!handleKeys(E.key.keysym.sym,E.key.keysym.mod,E.key.keysym.unicode,S_GUI_PRESSED) )
					handleKeys(E.key.keysym.sym,E.key.keysym.mod,E.key.keysym.sym,S_GUI_PRESSED);

				break;

			case SDL_KEYUP:
				handleKeys(E.key.keysym.sym,E.key.keysym.mod,E.key.keysym.sym,S_GUI_RELEASED);
			}
		} else { // No events to poll
			// If the application is not visible
			if (!AppVisible) {
				// Leave the CPU alone, don't waste time, simply wait for an event
				SDL_WaitEvent(NULL);
			} else {
				// Compute how many fps we should run at to get 1 pixel movement each frame.
				double frameRate = 1000. * animationSpeed;
				// If there was user action in the last 2.5 seconds, shoot for the max framerate.
				if (SDL_GetTicks() - last_event_time < 2500 || frameRate > maxfps) {
					frameRate = maxfps;
				}
				if (frameRate < minfps) {
					frameRate = minfps;
				}

				TickCount = SDL_GetTicks();			// Get present ticks
				// Wait a while if drawing a frame right now would exceed our preferred framerate.
				if (TickCount-LastCount < 1000./frameRate) {
					unsigned int delay = (unsigned int) (1000./frameRate) - (TickCount-LastCount);
//					printf("delay=%d\n", delay);
					if (delay < 15) {
						// Less than 15ms, just do a dumb wait.
						SDL_Delay(delay);
					} else {
						// A longer delay. Use this timer song and dance so
						// that the app is still responsive if the user does something.
						if (!SDL_AddTimer(delay, mytimer_callback, NULL)) {
							cerr << "Error: couldn't create an SDL timer: " << SDL_GetError() << endl;
						}
						SDL_WaitEvent(NULL);
					}
				}

				TickCount = SDL_GetTicks();			// Get present ticks

				// If outputting video frames, force output frame rate
				if(m_OutputFrameNumber) {
					TickCount = LastCount + 1000./videoRecordFps; 
				}

				this->update(TickCount-LastCount);	// And update the motions and data
				double squaredDistance = this->draw(TickCount-LastCount);	// Do the drawings!

				// write out video frame if recording video
				if(m_OutputFrameNumber) {
					char c[5];
					snprintf(c,5,"%04u",m_OutputFrameNumber);
					writeScreenshot(getScreenshotDirectory() + APP_LOWER_NAME + "-frame-" + string(c) + ".bmp");
					m_OutputFrameNumber++;
				}

				animationSpeed = sqrt(squaredDistance) / (TickCount-LastCount);
				LastCount = TickCount;				// Save the present tick probing
				SDL_GL_SwapBuffers();				// And swap the buffers
			}
		}
	}
}


// Write current video frame to a specified file
void App::writeScreenshot(string filename) 
{

	SDL_Surface * temp = SDL_CreateRGBSurface(SDL_SWSURFACE, m_sdl->getScreen()->w, m_sdl->getScreen()->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
											  0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
											  0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
		);
	if (temp == NULL) return;

	unsigned char * pixels = (unsigned char *) malloc(3 * m_sdl->getScreen()->w * m_sdl->getScreen()->h);
	if (pixels == NULL) {
		SDL_FreeSurface(temp);
		return;
	}

	glReadPixels(0, 0, m_sdl->getScreen()->w, m_sdl->getScreen()->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	for (int i=0; i<m_sdl->getScreen()->h; i++) {
		memcpy(((char *) temp->pixels) + temp->pitch * i,
			   pixels + 3*m_sdl->getScreen()->w * (m_sdl->getScreen()->h-i-1), m_sdl->getScreen()->w*3);
	}
	free(pixels);

	SDL_SaveBMP(temp, filename.c_str());
	SDL_FreeSurface(temp);

}


// Determine where screenshot files should go on different platforms
string App::getScreenshotDirectory()
{
	string shotdir;
#if defined(WIN32)
	char path[MAX_PATH];
	path[MAX_PATH-1] = '\0';
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path))) {
		shotdir = string(path)+"\\";
	} else {
		if (getenv("USERPROFILE")!=NULL) {
			//for Win XP etc.
			shotdir = string(getenv("USERPROFILE")) + "\\My Documents\\";
		} else {
			//for Win 98 etc.
			shotdir = "C:\\My Documents\\";
		}
	}

#else
	shotdir = string(getenv("HOME")) + "/";
#endif
#ifdef MACOSX
	shotdir += "/Desktop/";
#endif

	return shotdir;
}


// Return the next sequential screenshot filename to use
string App::getNextScreenshotFilename()
{
	string tempName;
	char c[3];
	FILE *fp;

	string shotdir = getScreenshotDirectory();

	for (int j=0; j<=100; ++j) {
		snprintf(c,3,"%d",j);

		tempName = shotdir + APP_LOWER_NAME + c + ".bmp";
		fp = fopen(tempName.c_str(), "r");
		if (fp == NULL)
			break;
		else
			fclose(fp);
	}

	return tempName;

}


