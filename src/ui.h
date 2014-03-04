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

// Class which handles the User Interface
// TODO : get rid of the SDL macro def and types
// need the creation of an interface between s_gui and SDL

#ifndef _UI_H
#define _UI_H

#include "app_command_interface.h"
#include "nightshade.h"
#include "s_gui.h"
#include "s_tui.h"
#include "init_parser.h"

#define TUI_SCRIPT_MSG "Select and exit to run."

using namespace std;
using namespace s_gui;

class App;
class Core;
class AppCommandInterface;

class UI
{
	friend class AppCommandInterface;
	friend class App;
public:
	UI(Core *, App * _app);	// Create the ui. Need to call init() before use
	virtual ~UI();		// Delete the ui
	void init(const InitParser& conf);		// Initialize the ui.
	void draw(void);		// Display the ui
	void gui_update_widgets(int delta_time);		// Update changing values

	void draw_gravity_ui(void);	// Draw simple gravity text ui.

	// Handle mouse clics
	int handle_clic(Uint16 x, Uint16 y, S_GUI_VALUE button, S_GUI_VALUE state);
	// Handle mouse move
	int handle_move(int x, int y);
	// Handle key press and release
	int handle_keys(SDLKey key, SDLMod mod, Uint16 unicode, S_GUI_VALUE state);

	// Text UI
	void init_tui(void);
	void localizeTui(void);
	void draw_tui(void);		// Display the tui
	int handle_keys_tui(Uint16 key, s_tui::S_TUI_VALUE state);
	// Update all the tui widgets with values taken from the core parameters
	void tui_update_widgets(void);
	void tuiUpdateIndependentWidgets(void); // For widgets that aren't tied directly to current settings
	void show_message(string _message, int _time_out=0);
	void setStarAutoComplete(vector<string> _autoComplete ) {
		star_edit->setAutoCompleteOptions(_autoComplete);
	}
	void setTitleObservatoryName(const string& name);
	string getTitleWithAltitude(void);
	bool isInitialised(void) {
		return initialised;
	}

	void setColorScheme(const string& skinFile, const string& section);
	double getMouseCursorTimeout() {
		return MouseCursorTimeout;
	}

	bool getFlagShowGravityUi() {
		return FlagShowGravityUi;
	}
	bool getFlagShowTuiDateTime() {
		return FlagShowTuiDateTime;
	}
	bool getFlagShowTuiShortObjInfo() {
		return FlagShowTuiShortObjInfo;
	}

	// Whether tui menu is currently displayed
	void setFlagShowTuiMenu(const bool flag);
	bool getFlagShowTuiMenu() {
		return FlagShowTuiMenu;
	}


private:
	Core * core;		// The main core can be accessed because UI is a friend class (TODO fix that)
	App * app;			// The main application instance

	bool shiftModifier;
	bool specialModifier;

	bool initialised;
	s_font * baseFont;		// The standard font
	s_font * courierFont;	// The standard fixed size font
	s_font * tuiFont;		// The standard tui font - separate from gui so can reload on the fly
	s_texture * baseTex;	// The standard fill texture
	s_texture * flipBaseTex;	// The standard fill texture
	s_texture * tex_up;		// Up arrow texture
	s_texture * tex_down;	// Down arrow texture

	// Flags and variables (moved from Core)
	int FlagShowTopBar;
	int FlagShowFps;
	int FlagShowTime;
	int FlagShowDate;
	int FlagShowAppName;
	int FlagShowFov;
	int FlagMenu;
	int FlagHelp;
	int FlagInfos;
	int FlagConfig;
	int FlagSearch;
	int FlagShowSelectedObjectInfo;
	int FlagShowScriptBar;
	float BaseFontSize;
	string BaseFontName;
	float BaseCFontSize;
	string BaseCFontName;

	// Text UI
	float TuiFontSize;
	bool FlagEnableTuiMenu;
	bool FlagShowGravityUi;
	bool FlagShowTuiMenu;
	bool FlagShowTuiDateTime;
	bool FlagShowTuiShortObjInfo;

	double MouseCursorTimeout;  // seconds to hide cursor when not used.  0 means no timeout


	Container * desktop;	// The container which contains everything
	bool opaqueGUI;
	// The top bar containing the main infos (date, time, fps etc...)
	FilledContainer * top_bar_ctr;
	Label * top_bar_date_lbl;
	Label * top_bar_hour_lbl;
	Label * top_bar_fps_lbl;
	Label * top_bar_appName_lbl;
	Label * top_bar_fov_lbl;
	Component* createTopBar(void);
	void updateTopBar(void);

	// Flags buttons (the buttons in the bottom left corner)
	FilledContainer * bt_flag_ctr;		// The container for the button
	FlagButton * bt_flag_constellation_draw;
	FlagButton * bt_flag_constellation_name;
	FlagButton * bt_flag_constellation_art;
	FlagButton * bt_flag_azimuth_grid;
	FlagButton * bt_flag_equator_grid;
	FlagButton * bt_flag_ground;
	FlagButton * bt_flag_cardinals;
	FlagButton * bt_flag_atmosphere;
	FlagButton * bt_flag_nebula_name;
	FlagButton * bt_flag_help;
	FlagButton * bt_flag_equatorial_mode;
	FlagButton * bt_flag_config;
	//FlagButton * bt_flag_chart;
	FlagButton * bt_flag_planet;
	FlagButton * bt_flag_search;
	EditBox * bt_script;
	FlagButton * bt_flag_goto;
	FlagButton * bt_flip_horz;
	FlagButton * bt_flip_vert;
	FlagButton * bt_flag_quit;

	void cbEditScriptInOut(void);
	void cbEditScriptPress(void);
	void cbEditScriptExecute(void);
	void cbEditScriptKey(void);
	void cbEditScriptWordCount(void);

	Component* createFlagButtons(const InitParser &conf);
	void cb(void);
	void bt_flag_ctrOnMouseInOut(void);
	void cbr(void);

	// Tile control buttons
	FilledContainer * bt_time_control_ctr;
	FlagButton * bt_dec_time_speed;
	FlagButton * bt_real_time_speed;
	FlagButton * bt_inc_time_speed;
	FlagButton * bt_time_now;
	Component* createTimeControlButtons(void);
	void bt_time_control_ctrOnMouseInOut(void);
	void bt_dec_time_speed_cb(void);
	void bt_real_time_speed_cb(void);
	void bt_inc_time_speed_cb(void);
	void bt_time_now_cb(void);
	void tcbr(void);

	// The dynamic information about the button under the mouse
	Label * bt_flag_help_lbl;
	Label * bt_flag_time_control_lbl;

	// The TextLabel displaying the infos about the selected object
	Container * info_select_ctr;
	TextLabel * info_select_txtlbl;
	void updateInfoSelectString(void);

	// The window containing the info (licence)
	StdBtWin * licence_win;
	TextLabel * licence_txtlbl;
	Component* createLicenceWindow(void);

	// The window containing the help info
	StdBtWin * help_win;
	TextLabel * help_txtlbl;
	HashBox * help_text;
	Component* createHelpWindow(void);
	void help_win_hideBtCallback(void);

	// window for transient messages
	StdTransBtWin * message_win;
	TextLabel * message_txtlbl;

	// The window managing the configuration
	StdBtWin* config_win;
	Component* createConfigWindow(void);
	void config_win_hideBtCallback(void);

	TabContainer * config_tab_ctr;

	void load_cities(const string & fileName);
	vector<City*> cities;

	// The window managing the search - Tony
	StdBtWin* search_win;
	Component* createSearchWindow(void);
	void search_win_hideBtCallback(void);
	EditBox* star_edit;
	void autoCompleteSearchedObject(void);
	void gotoSearchedObject(void);
	Label *lblSearchMessage;

	// standard dialogs
	StdDlgWin* dialog_win;
	void dialogCallback(void);

	// Rendering options
	LabeledCheckBox* stars_cbx;
	LabeledCheckBox* star_names_cbx;
	FloatIncDec* max_mag_star_name;
	LabeledCheckBox* star_twinkle_cbx;
	FloatIncDec* star_twinkle_amount;
	LabeledCheckBox* constellation_cbx;
	LabeledCheckBox* constellation_name_cbx;
	LabeledCheckBox* constellation_boundaries_cbx;
	LabeledCheckBox* sel_constellation_cbx;
	LabeledCheckBox* nebulas_names_cbx;
	LabeledCheckBox* nebulas_no_texture_cbx;
	FloatIncDec* max_mag_nebula_name;
	LabeledCheckBox* planets_cbx;
	LabeledCheckBox* planets_hints_cbx;
	LabeledCheckBox* moon_x4_cbx;
	Label* meteorlbl;
	LabeledCheckBox* meteor_rate_10;
	LabeledCheckBox* meteor_rate_80;
	LabeledCheckBox* meteor_rate_10000;
	LabeledCheckBox* meteor_rate_144000;
	LabeledCheckBox* meteor_rate_perseids;
	LabeledCheckBox* equator_grid_cbx;
	LabeledCheckBox* azimuth_grid_cbx;
	LabeledCheckBox* equator_cbx;
	LabeledCheckBox* ecliptic_cbx;
	LabeledCheckBox* ground_cbx;
	LabeledCheckBox* cardinal_cbx;
	LabeledCheckBox* atmosphere_cbx;
	LabeledCheckBox* fog_cbx;
	void saveRenderOptions(void);
	void saveLandscapeOptions(void);
	void saveLanguageOptions(void);

	// Location options
	bool waitOnLocation;
	MapPicture* earth_map;
	s_texture* earth;  // Current planet texture (loaded separately from planet!)
	string lastHomePlanet;
	Label *lblMapLocation;
	Label *lblMapPointer;
	FloatIncDec* lat_incdec, * long_incdec;
	IntIncDec* alt_incdec;
	void setObserverPositionFromMap(void);
	void setCityFromMap(void);
	void setObserverPositionFromIncDec(void);
	void doSaveObserverPosition(const string &name);
	void saveObserverPosition(void); // callback to the MapPicture change

	// Date & Time options
	Time_item* time_current;
	LabeledCheckBox* system_tz_cbx;
	void setCurrentTimeFromConfig(void);

	Time_zone_item* tzselector;
	Label* system_tz_lbl2;
	Label* time_speed_lbl2;

	// Video Options
	StringList* projection_sl;
	LabeledCheckBox* disk_viewport_cbx;
	ListBox* screen_size_sl;
	void setVideoOption(void);
	void updateVideoVariables(void);

	void updateConfigVariables(void);
	void updateConfigVariables2(void);
	void updateConfigForm(void);

	bool LocaleChanged;  // flag to watch for need to rebuild TUI

	// Landscape options
	ListBox* landscape_sl;
	void setLandscape(void);
	void saveLandscapeOption(void);
	Label* landscape_authorlb;
	TextLabel* landscape_descriptionlb;

	// Language options
	ListBox* language_lb;
	ListBox* languageSky_lb;
	ListBox* skyculture_lb;
	void setAppLanguage(void);
	void setSkyLanguage(void);
	void setSkyCulture(void);

	// Mouse control options
	bool is_dragging, has_dragged;
	int previous_x, previous_y;

	////////////////////////////////////////////////////////////////////////////
	// Text UI components
	s_tui::Branch* tui_root;

	// Menu branches
	s_tui::MenuBranch* tui_menu_location;
	s_tui::MenuBranch* tui_menu_time;
	s_tui::MenuBranch* tui_menu_general;
	s_tui::MenuBranch* tui_menu_stars;
	s_tui::MenuBranch* tui_menu_colors;
	s_tui::MenuBranch* tui_menu_effects;
	s_tui::MenuBranch* tui_menu_scripts;
	s_tui::MenuBranch* tui_menu_administration;

	// 1. Location
	s_tui::Decimal_item* tui_location_latitude;
	s_tui::Decimal_item* tui_location_longitude;
	s_tui::Integer_item_logstep* tui_location_altitude;
	s_tui::MultiSet2_item<string>* tui_location_planet;
	s_tui::Decimal_item* tui_location_heading;

	// 2. Time & Date
	s_tui::Time_zone_item* tui_time_settmz;
	s_tui::Time_item* tui_time_skytime;
	s_tui::Time_item* tui_time_presetskytime;
	s_tui::MultiSet2_item<string>* tui_time_startuptime;
	s_tui::MultiSet_item<string>* tui_time_displayformat;
	s_tui::MultiSet_item<string>* tui_time_dateformat;
	s_tui::MultiSet2_item<string>* tui_time_day_key;

	// 3. General
	s_tui::MultiSet_item<string>* tui_general_landscape;
	s_tui::MultiSet2_item<string>* tui_general_sky_culture;
	s_tui::MultiSet_item<string>* tui_general_sky_locale;
	//	s_tui::MultiSet2_item<string>* tui_general_sky_locale; (if translate locales to names)

	// 4. Stars
	s_tui::Boolean_item* tui_stars_show;
	s_tui::Decimal_item* tui_star_labelmaxmag;
	s_tui::Decimal_item* tui_stars_twinkle;
	s_tui::Decimal_item* tui_star_magscale;
	s_tui::Decimal_item* tui_star_limitingmag;
	//  Colors
	s_tui::Vector_item* tui_colors_const_line_color;
	s_tui::Vector_item* tui_colors_const_label_color;
	s_tui::Vector_item* tui_colors_cardinal_color;
	s_tui::Vector_item* tui_colors_const_boundary_color;
	s_tui::Vector_item* tui_colors_planet_names_color;
	s_tui::Vector_item* tui_colors_planet_orbits_color;
	s_tui::Vector_item* tui_colors_satellite_orbits_color;
	s_tui::Vector_item* tui_colors_object_trails_color;
	s_tui::Vector_item* tui_colors_meridian_color;
	s_tui::Vector_item* tui_colors_azimuthal_color;
	s_tui::Vector_item* tui_colors_equatorial_color;
	s_tui::Vector_item* tui_colors_equator_color;
	s_tui::Vector_item* tui_colors_ecliptic_color;
	s_tui::Vector_item* tui_colors_nebula_label_color;
	s_tui::Vector_item* tui_colors_nebula_circle_color;
	s_tui::Vector_item* tui_colors_precession_circle_color;
	s_tui::Vector_item* tui_colors_circumpolar_circle_color;
	s_tui::Decimal_item* tui_colors_const_art_intensity;
	s_tui::Vector_item* tui_colors_const_art_color;
	s_tui::Vector_item* tui_colors_galactic_color;

	// 5. Effects
	s_tui::Boolean_item* tui_effect_pointobj;
	s_tui::Decimal_item* tui_effect_object_scale;
	s_tui::Decimal_item* tui_effect_star_size_limit;
	s_tui::Decimal_item* tui_effect_planet_size_limit;
	s_tui::Decimal_item* tui_effect_zoom_duration;
	s_tui::Decimal_item* tui_effect_flight_duration;
	s_tui::Decimal_item* tui_effect_milkyway_intensity;
	s_tui::Decimal_item* tui_effect_nebulae_label_magnitude;
	s_tui::Boolean_item* tui_effect_manual_zoom;
	s_tui::Decimal_item* tui_effect_cursor_timeout;
	s_tui::Decimal_item* tui_effect_light_pollution;
	s_tui::Boolean_item* tui_effect_light_travel;
	s_tui::Decimal_item* tui_effect_view_offset;
	s_tui::Boolean_item* tui_effect_antialias;
	s_tui::Decimal_item* tui_effect_line_width;

	// 6. Scripts
	s_tui::List_item<string>* tui_scripts_local;
	s_tui::List_item<string>* tui_scripts_removeable;
	bool flag_scripts_removeable_disk_mounted;  // is the removeable disk for scripts mounted?

	s_tui::List_item<string>* tui_scripts_usb;
	s_tui::List_item<string>* tui_scripts_internal;

	// 7. Administration
	s_tui::ActionConfirm_item* tui_admin_loaddefault;
	s_tui::ActionConfirm_item* tui_admin_savedefault;
	s_tui::ActionConfirm_item* tui_admin_shutdown;
	s_tui::ActionConfirm_item* tui_admin_updateme;
	s_tui::MultiSet_item<string>* tui_admin_setlocale;
// for geometric distortion correction
	s_tui::MultiSet2_item<string>* tui_admin_lens_position;
	s_tui::ActionConfirm_item* tui_admin_syncdrive;

	s_tui::Decimal_item* tui_admin_offset_x;
	s_tui::Decimal_item* tui_admin_offset_y;
	s_tui::Decimal_item* tui_admin_shear;

	s_tui::Display* tui_admin_info; // display about info
	s_tui::Integer_item* tui_admin_center_hoffset; // new version of horizontal video offset
	s_tui::Password_item* tui_admin_password;

	// Tui Callbacks
	void tui_cb1(void);									// Update all the core flags and params from the tui
	void tui_cb_settimezone(void);						// Set time zone
	void tui_cb_settimedisplayformat(void);				// Set 12/24h format
	void tui_cb_admin_load_default(void);				// Load default configuration
	void tui_cb_admin_save_default(void);				// Save default configuration
	void tui_cb_admin_set_locale(void);					// Set locale for UI (not sky)
	void tui_cb_admin_shutdown(void);					// Shut down
	void tui_cb_admin_updateme(void);					// Launch script for internet update
	void tui_cb_admin_lens_position();      			// change geometric distortion
	void tui_cb_admin_sync_drive();      				// sync drive utiltity DIGITALIS
	void tui_cb_center_hoffset();      					// update viewport center horizontal offset DIGITALIS
	void tui_cb_viewport_related();   					// Change heading or view offset
	void tui_cb_tui_general_change_landscape(void);		// Select a new landscape skin
	void tui_cb_tui_general_change_sky_culture(void);  	// select new sky culture
	void tui_cb_tui_general_change_sky_locale(void);  	// select new sky locale
	void tui_cb_scripts_removeable(void);    			// changed removeable disk script selection

	void tui_cb_scripts_internal();             		// changed internal script selection
	void tui_cb_scripts_usb(void);    					// changed removeable disk script selection
	void tui_cb_day_key(); 								// update day key mode

	void tui_cb_scripts_local();             			// changed local script selection
	void tui_cb_effects_milkyway_intensity();       	// change milky way intensity
	void tui_cb_effects_nebulae_label_magnitude();  	// change nebula label limiting magnitude
	void tui_cb_setlocation();        					// change observer position
	void tui_cb_stars();        						// change star parameters
	void tui_cb_effects();        						// change effect parameters
	void tui_cb_sky_time();       						// change effect parameters
	void tui_cb_change_color();        					// change colors
	void tui_cb_location_change_planet();  				// change planet of observer
	void tui_set_password();			  				// Set password for authentication via browser

	// Parse a file of type /usr/share/zoneinfo/zone.tab
	s_tui::MultiSet_item<string>* create_tree_from_time_zone_file(const string& zonetab);

	bool ScriptDirectoryRead;

	bool ScriptDirectoryRead2;  // USB

	double MouseTimeLeft;  // for cursor timeout (seconds)

	double ShiftTimeLeft;  // for shift timeout (seconds)
	double SpecialTimeLeft;  // for special keystroke timeout (seconds)
	int SpecialNumber;       // For direct script access
};

#endif  //_UI_H
