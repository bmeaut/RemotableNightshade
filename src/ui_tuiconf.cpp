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

// Class which handles the Text User Interface

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "ui.h"
#include "stellastro.h"
#include "sky_localizer.h"

#include <dirent.h>
#include "nightshade.h"

// Draw simple gravity text ui.
void UI::draw_gravity_ui(void)
{
	// Normal transparency mode
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (FlagShowTuiDateTime) {
		double jd = core->getJDay();
		ostringstream os;

		os << app->get_printable_date_local(jd) << " " << app->get_printable_time_local(jd);

		// label location if not on earth
		if (core->getObservatory()->getHomePlanetEnglishName() != "Earth") {
			os << " " << _(core->getObservatory()->getHomePlanetEnglishName());
		}

		if (FlagShowFov) os << " fov " << setprecision(3) << core->getFov();
		if (FlagShowFps) os << "  FPS " << int(app->fps + 0.5);

		glColor3f(0.5,1,0.5);
		//		core->printGravity(tuiFont, x - shift + 38, y - 38, os.str(), 0);
		core->printGravity(tuiFont, 5, 105, os.str(), 1);
	}

	if (core->getFlagHasSelected() && FlagShowTuiShortObjInfo) {
		string info = core->getSelectedObjectShortInfo();
		glColor3fv(core->getSelectedObjectInfoColor());
		//core->printGravity(tuiFont, x + shift - 38, y + 38, info, 0);
		core->printGravity(tuiFont, 5, 285, info, 1);
	}
}

// Create all the components of the text user interface
// should be safe to call more than once but not recommended
// since lose states - try localizeTui() instead
void UI::init_tui(void)
{
	// Menu root branch
	ScriptDirectoryRead = 0;
	ScriptDirectoryRead2 = 0;

	// If already initialized before, delete existing objects
	if (tui_root) delete tui_root;
	if (tuiFont) delete tuiFont;

	// Load standard font based on app locale
	string fontFile, tmpstr;
	float fontScale, tmpfloat;

	core->getFontForLocale(app->getAppLanguage(), fontFile, fontScale, tmpstr, tmpfloat);

	tuiFont = new s_font(TuiFontSize*fontScale, fontFile);
	if (!tuiFont) {
		printf("ERROR WHILE CREATING FONT\n");
		exit(-1);
	}

	tui_root = new s_tui::Branch();

	// Submenus
	tui_menu_location = new s_tui::MenuBranch(string("1. ") );
	tui_menu_time = new s_tui::MenuBranch(string("2. ") );
	tui_menu_general = new s_tui::MenuBranch(string("3. ") );
	tui_menu_stars = new s_tui::MenuBranch(string("4. ") );
	tui_menu_colors = new s_tui::MenuBranch(string("5. ") );
	tui_menu_effects = new s_tui::MenuBranch(string("6. ") );
	tui_menu_scripts = new s_tui::MenuBranch(string("7. ") );
	tui_menu_administration = new s_tui::MenuBranch(string("8. ") );

	tui_root->addComponent(tui_menu_location);
	tui_root->addComponent(tui_menu_time);
	tui_root->addComponent(tui_menu_general);
	tui_root->addComponent(tui_menu_stars);
	tui_root->addComponent(tui_menu_colors);
	tui_root->addComponent(tui_menu_effects);
	tui_root->addComponent(tui_menu_scripts);
	tui_root->addComponent(tui_menu_administration);

	// 1. Location
	tui_location_latitude = new s_tui::Decimal_item(-90., 90., 0.,string("1.1 ") );
	tui_location_latitude->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_setlocation));
	tui_location_longitude = new s_tui::Decimal_item(-720., 720., 0.,string("1.2 "), 1, true);
	tui_location_longitude->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_setlocation));

//	tui_location_altitude = new s_tui::Integer_item(-500, 10000, 0,string("1.3 ") );
	tui_location_altitude = new s_tui::Integer_item_logstep(-500, 100000000, 0,string("1.3 ") );

	tui_location_altitude->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_setlocation));

	// Home planet only changed if hit enter to accept because
	// switching planet instantaneously as select is hard on a planetarium audience
	tui_location_planet = new s_tui::MultiSet2_item<string>(string("1.4 ") );
	tui_location_planet->addItemList(core->getPlanetHashString());
	//	tui_location_planet->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_location_change_planet));
	tui_location_planet->set_OnTriggerCallback(callback<void>(this, &UI::tui_cb_location_change_planet));

	tui_menu_location->addComponent(tui_location_latitude);
	tui_menu_location->addComponent(tui_location_longitude);
	tui_menu_location->addComponent(tui_location_altitude);
	tui_menu_location->addComponent(tui_location_planet);

	tui_location_heading = new s_tui::Decimal_item(-180, 180, 0, string("7.8 "), 1, true);
	tui_location_heading->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_viewport_related));
	tui_menu_location->addComponent(tui_location_heading);


	// 2. Time
	tui_time_skytime = new s_tui::Time_item(string("2.1 ") );
	tui_time_skytime->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_sky_time));
	tui_time_settmz = new s_tui::Time_zone_item(core->getDataDir() + "zone.tab", string("2.2 ") );
	tui_time_settmz->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_settimezone));
	tui_time_settmz->settz(app->get_custom_tz_name());
	tui_time_day_key = new s_tui::MultiSet2_item<string>(string("2.2 ") );
	tui_time_day_key->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_day_key));

	tui_time_presetskytime = new s_tui::Time_item(string("2.3 ") );
	tui_time_presetskytime->set_OnChangeCallback(callback<void>(this, &UI::tui_cb1));
	tui_time_startuptime = new s_tui::MultiSet2_item<string>(string("2.4 ") );
	tui_time_startuptime->set_OnChangeCallback(callback<void>(this, &UI::tui_cb1));
	tui_time_displayformat = new s_tui::MultiSet_item<string>(string("2.5 ") );
	tui_time_displayformat->addItem("24h");
	tui_time_displayformat->addItem("12h");
	tui_time_displayformat->addItem("system_default");
	tui_time_displayformat->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_settimedisplayformat));
	tui_time_dateformat = new s_tui::MultiSet_item<string>(string("2.6 ") );
	tui_time_dateformat->addItem("yyyymmdd");
	tui_time_dateformat->addItem("ddmmyyyy");
	tui_time_dateformat->addItem("mmddyyyy");
	tui_time_dateformat->addItem("system_default");
	tui_time_dateformat->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_settimedisplayformat));

	tui_menu_time->addComponent(tui_time_skytime);
	tui_menu_time->addComponent(tui_time_settmz);
	tui_menu_time->addComponent(tui_time_day_key);
	tui_menu_time->addComponent(tui_time_presetskytime);
	tui_menu_time->addComponent(tui_time_startuptime);
	tui_menu_time->addComponent(tui_time_displayformat);
	tui_menu_time->addComponent(tui_time_dateformat);

	// 3. General settings

	tui_general_landscape = new s_tui::MultiSet_item<string>(string("3.1 ") );
	tui_general_landscape->addItemList(string(Landscape::get_file_content(core->getDataDir() + "landscapes.ini")));

	tui_general_landscape->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_tui_general_change_landscape));
	tui_menu_general->addComponent(tui_general_landscape);


	// sky culture goes here
	tui_general_sky_culture = new s_tui::MultiSet2_item<string>(string("3.1 ") );
	tui_general_sky_culture->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_tui_general_change_sky_culture));
	tui_menu_general->addComponent(tui_general_sky_culture);

	tui_general_sky_locale = new s_tui::MultiSet_item<string>(string("3.2 ") );
	tui_general_sky_locale->addItemList(string(Translator::getAvailableLanguagesCodes(core->getLocaleDir())));

	tui_general_sky_locale->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_tui_general_change_sky_locale));
	tui_menu_general->addComponent(tui_general_sky_locale);


	// 4. Stars
	tui_stars_show = new s_tui::Boolean_item(false, string("4.1 ") );
	tui_stars_show->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_stars));
	tui_star_magscale = new s_tui::Decimal_item(0,30, 1, string("4.2 "), 0.1 );
	tui_star_magscale->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_stars));
	tui_star_labelmaxmag = new s_tui::Decimal_item(-1.5, 99., 2, string("4.3 ") );
	tui_star_labelmaxmag->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_stars));
	tui_stars_twinkle = new s_tui::Decimal_item(0., 1., 0.3, string("4.4 "), 0.1);
	tui_stars_twinkle->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_stars));
	tui_star_limitingmag = new s_tui::Decimal_item(0., 7., 6.5, string("4.5 "), 0.1);
	tui_star_limitingmag->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_stars));

	tui_menu_stars->addComponent(tui_stars_show);
	tui_menu_stars->addComponent(tui_star_magscale);
	tui_menu_stars->addComponent(tui_star_labelmaxmag);
	tui_menu_stars->addComponent(tui_stars_twinkle);
	tui_menu_stars->addComponent(tui_star_limitingmag);

	// 5. Colors
	tui_colors_const_line_color = new s_tui::Vector_item(string("5.1 "));
	tui_colors_const_line_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_const_line_color);

	tui_colors_const_label_color = new s_tui::Vector_item(string("5.2 "));
	tui_colors_const_label_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_const_label_color);

	tui_colors_const_art_intensity = new s_tui::Decimal_item(0,1,1,string("5.3 "),0.05);
	tui_colors_const_art_intensity->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_const_art_intensity);

	tui_colors_const_art_color = new s_tui::Vector_item(string("5.2 "));
	tui_colors_const_art_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_const_art_color);

	tui_colors_const_boundary_color = new s_tui::Vector_item(string("5.4 "));
	tui_colors_const_boundary_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_const_boundary_color);

	tui_colors_cardinal_color = new s_tui::Vector_item(string("5.5 "));
	tui_colors_cardinal_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_cardinal_color);

	tui_colors_planet_names_color = new s_tui::Vector_item(string("5.6 "));
	tui_colors_planet_names_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_planet_names_color);

	tui_colors_planet_orbits_color = new s_tui::Vector_item(string("5.7 "));
	tui_colors_planet_orbits_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_planet_orbits_color);

	tui_colors_satellite_orbits_color = new s_tui::Vector_item(string("5.8 "));
	tui_colors_satellite_orbits_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_satellite_orbits_color);

	tui_colors_object_trails_color = new s_tui::Vector_item(string("5.9 "));
	tui_colors_object_trails_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_object_trails_color);

	tui_colors_meridian_color = new s_tui::Vector_item(string("5.10 "));
	tui_colors_meridian_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_meridian_color);

	tui_colors_azimuthal_color = new s_tui::Vector_item(string("5.11 "));
	tui_colors_azimuthal_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_azimuthal_color);

	tui_colors_equatorial_color = new s_tui::Vector_item(string("5.12 "));
	tui_colors_equatorial_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_equatorial_color);

	tui_colors_equator_color = new s_tui::Vector_item(string("5.13 "));
	tui_colors_equator_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_equator_color);

	tui_colors_ecliptic_color = new s_tui::Vector_item(string("5.14 "));
	tui_colors_ecliptic_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_ecliptic_color);

	tui_colors_nebula_label_color = new s_tui::Vector_item(string("5.15 "));
	tui_colors_nebula_label_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_nebula_label_color);

	tui_colors_nebula_circle_color = new s_tui::Vector_item(string("5.16 "));
	tui_colors_nebula_circle_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_nebula_circle_color);

	tui_colors_precession_circle_color = new s_tui::Vector_item(string("5.17 "));
	tui_colors_precession_circle_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_precession_circle_color);

	tui_colors_circumpolar_circle_color = new s_tui::Vector_item(string("5.18 "));
	tui_colors_circumpolar_circle_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_circumpolar_circle_color);

	tui_colors_galactic_color = new s_tui::Vector_item(string("5.19 "));
	tui_colors_galactic_color->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_change_color));
	tui_menu_colors->addComponent(tui_colors_galactic_color);


	// *** Effects

	tui_effect_light_pollution = new s_tui::Decimal_item(0.5, 7, 6.5, string("5.9 "), 0.5 );
	tui_effect_light_pollution->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_light_pollution);

	tui_effect_manual_zoom = new s_tui::Boolean_item(false, string("5.2 ") );
	tui_effect_manual_zoom->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_manual_zoom);

	tui_effect_pointobj = new s_tui::Boolean_item(false, string("5.3 ") );
	tui_effect_pointobj->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_pointobj);

	tui_effect_object_scale = new s_tui::Decimal_item(0, 25, 1, string("5.4 "), 0.05);  // changed to .05 delta
	tui_effect_object_scale->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_object_scale);

	tui_effect_star_size_limit = new s_tui::Decimal_item(0.25, 25, 5, string("5.4 "), 0.25);
	tui_effect_star_size_limit->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_star_size_limit);

	tui_effect_planet_size_limit = new s_tui::Decimal_item(0, 25, 4, string("5.4 "), 0.25);
	tui_effect_planet_size_limit->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_planet_size_limit);



	tui_effect_milkyway_intensity = new s_tui::Decimal_item(0, 100, 1, string("5.5 "), .1);  // cvs
	tui_effect_milkyway_intensity->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects_milkyway_intensity));
	tui_menu_effects->addComponent(tui_effect_milkyway_intensity);

	tui_effect_nebulae_label_magnitude = new s_tui::Decimal_item(0, 100, 1, string("5.6 "), .5);
	tui_effect_nebulae_label_magnitude->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects_nebulae_label_magnitude));
	tui_menu_effects->addComponent(tui_effect_nebulae_label_magnitude);

	tui_effect_view_offset = new s_tui::Decimal_item(-0.5, 0.5, 0, string("7.7 "), 0.05 );
	tui_effect_view_offset->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_viewport_related));
	tui_menu_effects->addComponent(tui_effect_view_offset);


	tui_effect_zoom_duration = new s_tui::Decimal_item(1, 10, 2, string("5.7 ") );
	tui_effect_zoom_duration->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_zoom_duration);

	tui_effect_flight_duration = new s_tui::Decimal_item(1, 2000, 10, string("5.7 ") );
	tui_effect_flight_duration->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_flight_duration);

	tui_effect_cursor_timeout = new s_tui::Decimal_item(0, 60, 1, string("5.8 ") );
	tui_effect_cursor_timeout->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_cursor_timeout);

	tui_effect_light_travel = new s_tui::Boolean_item(false, string("6.10 ") );
	tui_effect_light_travel->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_light_travel);

	tui_effect_antialias = new s_tui::Boolean_item(false, string("6.11 ") );
	tui_effect_antialias->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_antialias);

	tui_effect_line_width = new s_tui::Decimal_item(0.125, 5, 1, string("6.12 "), 0.125 );
	tui_effect_line_width->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_effects));
	tui_menu_effects->addComponent(tui_effect_line_width);

	// *** Scripts
	tui_scripts_local = new s_tui::List_item<string>(string("6.1 ") );
	//	tui_scripts_local->addItemList(string(TUI_SCRIPT_MSG) + string("\n")
	//			       + string(app->scripts->get_script_list(core->getDataDir() + "scripts/")));
	tui_scripts_local->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_scripts_local));
	tui_menu_scripts->addComponent(tui_scripts_local);

	tui_scripts_internal = new s_tui::List_item<string>(string("6.1 ") );
	tui_scripts_internal->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_scripts_internal));

	tui_scripts_usb = new s_tui::List_item<string>(string("6.3 ") );
	tui_scripts_usb->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_scripts_usb));

	bool usb=0;
	bool hdd=0;
#ifndef DESKTOP
	// Only add to tui if drive exists
	DIR *dp;
	if ((dp = opendir((core->getScriptDir() + SCRIPT_INTERNAL_DISK).c_str())) == NULL) {
		cout << "* NO * internal drive support on this system." << endl;
	} else {
		closedir(dp);
		hdd = 1;
		cout << "Internal drive support on this system." << endl;
		tui_menu_scripts->addComponent(tui_scripts_internal);
	}

	// Only add to tui if usb support exists
	if ((dp = opendir((core->getScriptDir() + SCRIPT_USB_DISK).c_str())) == NULL) {
		cout << "* NO * USB support on this system for scripts." << endl;
	} else {
		closedir(dp);
		usb = 1;
		tui_menu_scripts->addComponent(tui_scripts_usb);
		cout << "USB support on this system for scripts." << endl;
	}

#endif

	tui_scripts_removeable = new s_tui::List_item<string>(string("6.2 ") );
	//	tui_scripts_removeable->addItem(_(TUI_SCRIPT_MSG));
	tui_scripts_removeable->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_scripts_removeable));
#ifndef DESKTOP
	tui_menu_scripts->addComponent(tui_scripts_removeable);
#endif


	// 7. Administration
	tui_admin_loaddefault = new s_tui::ActionConfirm_item(string("7.1 ") );
	tui_admin_loaddefault->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_load_default));
	tui_admin_savedefault = new s_tui::ActionConfirm_item(string("7.2 ") );
	tui_admin_savedefault->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_save_default));
	tui_admin_shutdown = new s_tui::ActionConfirm_item(string("7.3 ") );
	tui_admin_shutdown->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_shutdown));
	tui_admin_updateme = new s_tui::ActionConfirm_item(string("7.4 ") );
	tui_admin_updateme->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_updateme));
	tui_menu_administration->addComponent(tui_admin_loaddefault);
	tui_menu_administration->addComponent(tui_admin_savedefault);

	if (usb) tui_menu_administration->addComponent(tui_admin_shutdown);

#ifndef DESKTOP
	tui_menu_administration->addComponent(tui_admin_updateme);
#endif

	tui_admin_setlocale = new s_tui::MultiSet_item<string>("7.5 ");
	tui_admin_setlocale->addItemList(string(Translator::getAvailableLanguagesCodes(core->getLocaleDir())));
	tui_admin_setlocale->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_set_locale));
	tui_menu_administration->addComponent(tui_admin_setlocale);

	tui_admin_offset_x = new s_tui::Decimal_item(-30, 30, 0, string("7.7 "), 1 );
	tui_admin_offset_x->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_lens_position));
	//tui_menu_administration->addComponent(tui_admin_offset_x);

	tui_admin_offset_y = new s_tui::Decimal_item(-30, 30, 0, string("7.7 "), 1 );
	tui_admin_offset_y->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_lens_position));
	tui_menu_administration->addComponent(tui_admin_offset_y);


	// TODO only if local drive exists!
	tui_admin_syncdrive = new s_tui::ActionConfirm_item(string("7.8 ") );
	tui_admin_syncdrive->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_sync_drive));
	if (hdd) tui_menu_administration->addComponent(tui_admin_syncdrive);


	tui_admin_lens_position = new s_tui::MultiSet2_item<string>(string("7.6 ") );
	tui_admin_lens_position->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_lens_position));

	// Only make visible if projector supports different configuration options

	if (core->projectorConfigurationSupported()) {
		tui_menu_administration->addComponent(tui_admin_lens_position);
	}


	// get system info
	string systemInfo;

#ifdef DESKTOP
	systemInfo = APP_NAME;
#else
	FILE *pd = popen( (core->getDataDir() + "script_system_info").c_str(), "r");
	if (pd==0) {
		systemInfo = APP_NAME;
	} else {
		char buffer[512] = {0};
		if ( fgets(buffer, sizeof(buffer), pd) > 0 ) {
			systemInfo = buffer;
		} else {
			systemInfo = APP_NAME;
		}
		pclose(pd);
	}
#endif

	tui_admin_info = new s_tui::Display(string("Label: "), string(systemInfo));
	tui_menu_administration->addComponent(tui_admin_info);

	tui_admin_shear = new s_tui::Decimal_item(90, 100, 100, string("7.7 "), 0.1 );
	tui_admin_shear->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_admin_lens_position));

	tui_admin_center_hoffset = new s_tui::Integer_item(-50, 50, 0, string("7.7 "));
	tui_admin_center_hoffset->set_OnChangeCallback(callback<void>(this, &UI::tui_cb_center_hoffset));

	if ( core->getLens() == 3 ) {
		tui_menu_administration->addComponent(tui_admin_shear);
		tui_menu_administration->addComponent(tui_admin_center_hoffset);
	} else {
		// For safety set to defaults if can't actually edit
		core->setProjectorShearHorz(1);
		core->setCenterHorizontalOffset(0);
	}

	tui_admin_password = new s_tui::Password_item(string("********"), string("8.12 "),
			string("8.12 ") + _("Re-enter password: "), _("Passwords do not match"),
			_("Password updated successfully"),
			_("Password must be more than 3 characters") );

#ifndef DESKTOP
	if( Utility::ProcessExists("console.fcgi") )
		tui_menu_administration->addComponent(tui_admin_password);
#endif

	// Now add in translated labels
	localizeTui();

}

// Update fonts, labels and lists for a new app locale
void UI::localizeTui(void)
{

	cout << "Localizing TUI for locale: " << app->getAppLanguage() << endl;
	if (tuiFont) delete tuiFont;

	// Load standard font based on app locale
	string fontFile, tmpstr;
	float fontScale, tmpfloat;

	core->getFontForLocale(app->getAppLanguage(), fontFile, fontScale, tmpstr, tmpfloat);

	// Reinit since locale specfici fontScale may be different
	tuiFont = new s_font(TuiFontSize*fontScale, fontFile);
	if (!tuiFont) {
		printf("ERROR WHILE CREATING FONT\n");
		exit(-1);
	}

	if (!tui_root) return; // not initialized yet

	tui_menu_location->setLabel(string("1. ") + _("Set Location "));
	tui_menu_time->setLabel(string("2. ") + _("Set Time "));
	tui_menu_general->setLabel(string("3. ") + _("General "));
	tui_menu_stars->setLabel(string("4. ") + _("Stars "));
	tui_menu_colors->setLabel(string("5. ") + _("Colors "));
	tui_menu_effects->setLabel(string("6. ") + _("Effects "));
	tui_menu_scripts->setLabel(string("7. ") + _("Scripts "));
	tui_menu_administration->setLabel(string("8. ") + _("Administration "));

	// 1. Location
	tui_location_latitude->setLabel(string("1.1 ") + _("Latitude: "));
	tui_location_longitude->setLabel(string("1.2 ") + _("Longitude: "));
	tui_location_altitude->setLabel(string("1.3 ") + _("Altitude (m): "));
	tui_location_planet->setLabel(string("1.4 ") + _("Solar System Body: "));
	tui_location_planet->replaceItemList(core->getPlanetHashString(),0);
	tui_location_heading->setLabel(string("1.5 ") + _("Heading: "));

	// 2. Time
	tui_time_skytime->setLabel(string("2.1 ") + _("Sky Time: "));
	tui_time_settmz->setLabel(string("2.2 ") + _("Set Time Zone: "));
// DIGITLAIS 20070123 add to svn
	tui_time_day_key->setLabel(string("2.3 ") + _("Day keys: "));
	tui_time_day_key->replaceItemList(_("Calendar") + string("\ncalendar\n")
	                                  + _("Sidereal") + string("\nsidereal\n"), 0);
	tui_time_presetskytime->setLabel(string("2.4 ") + _("Preset Sky Time: "));
	tui_time_startuptime->setLabel(string("2.5 ") + _("Sky Time At Start-up: "));
	tui_time_startuptime->replaceItemList(_("Actual Time") + string("\nActual\n")
	                                      + _("Preset Time") + string("\nPreset\n"), 0);
	tui_time_displayformat->setLabel(string("2.6 ") + _("Time Display Format: "));
	tui_time_dateformat->setLabel(string("2.7 ") + _("Date Display Format: "));

	// 3. General settings

	tui_general_landscape->setLabel(string("3.1 ") + _("Landscape: "));

	// sky culture goes here
	tui_general_sky_culture->setLabel(string("3.2 ") + _("Sky Culture: "));
	tui_general_sky_culture->replaceItemList(core->getSkyCultureHash(), 0);  // human readable names
	// wcout << "sky culture list is: " << core->getSkyCultureHash() << endl;

	tui_general_sky_locale->setLabel(string("3.3 ") + _("Sky Language: "));

	// 4. Stars
	tui_stars_show->setLabel(string("4.1 ") + _("Show: "), _("Yes"),_("No"));
	tui_star_magscale->setLabel(string("4.2 ") + _("Star Value Multiplier: "));
	tui_star_labelmaxmag->setLabel(string("4.3 ") + _("Maximum Magnitude to Label: "));
	tui_stars_twinkle->setLabel(string("4.4 ") + _("Twinkling: "));
	tui_star_limitingmag->setLabel(string("4.5 ") + _("Limiting Magnitude: "));

	// 5. Colors
	tui_colors_const_line_color->setLabel(string("5.1 ") + _("Constellation Lines") + ": ");
	tui_colors_const_label_color->setLabel(string("5.2 ") + _("Constellation Labels")+": ");
	tui_colors_const_art_intensity->setLabel(string("5.3 ") + _("Constellation Art Intensity") + ": ");
	tui_colors_const_art_color->setLabel(string("5.4 ") + _("Constellation Art")+": ");
	tui_colors_const_boundary_color->setLabel(string("5.5 ") + _("Constellation Boundaries") + ": ");
	tui_colors_cardinal_color->setLabel(string("5.6 ") + _("Cardinal Points")+ ": ");
	tui_colors_planet_names_color->setLabel(string("5.7 ") + _("Body Labels") + ": ");
	tui_colors_planet_orbits_color->setLabel(string("5.8 ") + _("Body Orbits") + ": ");
	tui_colors_satellite_orbits_color->setLabel(string("5.9 ") + _("Satellite Orbits") + ": ");
	tui_colors_object_trails_color->setLabel(string("5.10 ") + _("Body Trails") + ": ");  // TODO: Should be Body Trails
	tui_colors_meridian_color->setLabel(string("5.11 ") + _("Meridian Line") + ": ");
	tui_colors_azimuthal_color->setLabel(string("5.12 ") + _("Azimuthal Grid") + ": ");
	tui_colors_equatorial_color->setLabel(string("5.13 ") + _("Equatorial Grid") + ": ");
	tui_colors_equator_color->setLabel(string("5.14 ") + _("Equator Line") + ": ");
	tui_colors_ecliptic_color->setLabel(string("5.15 ") + _("Ecliptic Line") + ": ");
	tui_colors_nebula_label_color->setLabel(string("5.16 ") + _("Nebula Labels") + ": ");
	tui_colors_nebula_circle_color->setLabel(string("5.17 ") + _("Nebula Circles") + ": ");
	tui_colors_precession_circle_color->setLabel(string("5.18 ") + _("Precession Circle") + ": ");
	tui_colors_circumpolar_circle_color->setLabel(string("5.19 ") + _("Circumpolar Circle") + ": ");
	tui_colors_galactic_color->setLabel(string("5.20 ") + _("Galactic Grid") + ": ");


	// *** Effects

	tui_effect_light_pollution->setLabel(string("6.1 ") + _("Light Pollution Limiting Magnitude: "));

	tui_effect_manual_zoom->setLabel(string("6.2 ") + _("Manual zoom: "), _("Yes"),_("No"));
	tui_effect_pointobj->setLabel(string("6.3 ") + _("Object Sizing Rule: "), _("Point"),_("Magnitude"));
	tui_effect_object_scale->setLabel(string("6.4 ") + _("Magnitude Scaling Multiplier: "));
	// TODO need to renumber or move
	tui_effect_star_size_limit->setLabel(string("6.5 ") + _("Star Size Limit: "));
	tui_effect_planet_size_limit->setLabel(string("6.6 ") + _("Planet Size Marginal Limit: "));

	tui_effect_milkyway_intensity->setLabel(string("6.7 ") + _("Milky Way intensity: "));
	tui_effect_nebulae_label_magnitude->setLabel(string("6.8 ") + _("Maximum Nebula Magnitude to Label: "));

	tui_effect_view_offset->setLabel(string("6.9 ") + _("Zoom Offset: "));

	tui_effect_zoom_duration->setLabel(string("6.10 ") + _("Zoom Duration: "));
	tui_effect_flight_duration->setLabel(string("6.11 ") + _("Flight Duration: "));
	tui_effect_cursor_timeout->setLabel(string("6.12 ") + _("Cursor Timeout: "));
	tui_effect_light_travel->setLabel(string("6.13 ") + _("Correct for light travel time: "), _("Yes"),_("No"));

	tui_effect_antialias->setLabel(string("6.14 ") + _("Antialias Lines: "), _("Yes"),_("No"));
	tui_effect_line_width->setLabel(string("6.15 ") + _("Line Width: "));


	// 7. Scripts
	tui_scripts_local->setLabel(string("7.1 ") + _("Local Script: "));
	tui_scripts_local->replaceItemList(_(TUI_SCRIPT_MSG) + string("\n")
	                                   + string(app->scripts->get_script_list(core->getScriptDir() + SCRIPT_LOCAL_DISK)), 0);

	tui_scripts_internal->setLabel(string("7.2 ") + _("Internal Script: "));
	tui_scripts_internal->replaceItemList(_(TUI_SCRIPT_MSG) + string("\n")
										  + string(app->scripts->get_script_list(core->getScriptDir() + SCRIPT_INTERNAL_DISK + "/scripts/")), 0); 

	tui_scripts_usb->setLabel(string("7.3 ") + _("USB Script: "));
	tui_scripts_usb->replaceItemList(_(TUI_SCRIPT_MSG), 0);

	tui_scripts_removeable->setLabel(string("7.4 ") + _("CD/DVD Script: "));
	tui_scripts_removeable->replaceItemList(_(TUI_SCRIPT_MSG), 0);

#ifdef LSS
	tui_scripts_local->setLabel(string("7.1 ") + _("Shows: "));
	tui_scripts_internal->setLabel(string("7.2 ") + _("Basis: "));
	tui_scripts_usb->setLabel(string("7.3 ") + _("Planets: "));
	tui_scripts_removeable->setLabel(string("7.4 ") + _("Deepsky: "));
#endif

	// 8. Administration
	tui_admin_loaddefault->setLabel(string("8.1 ") + _("Load Default Configuration: "));
	tui_admin_loaddefault->translateActions();

	tui_admin_savedefault->setLabel(string("8.2 ") + _("Save Current Configuration as Default: "));
	tui_admin_savedefault->translateActions();

	tui_admin_shutdown->setLabel(string("8.3 ") + _("Shut Down: "));
	tui_admin_shutdown->translateActions();

	tui_admin_updateme->setLabel(string("8.4 ") + _("Update me via Internet: "));
	tui_admin_updateme->translateActions();

	tui_admin_setlocale->setLabel(string("8.5 ") + _("Set UI Locale: "));
	tui_admin_offset_y->setLabel(string("8.6 ") + _("Projector Offset (percent of dome radius): "));
	tui_admin_syncdrive->setLabel(string("8.7 ") + _("Synchronize Internal Drive from USB Drive: "));
	tui_admin_syncdrive->translateActions();

	tui_admin_lens_position->setLabel(string("8.8 ") + _("Projection Configuration: "));
	tui_admin_lens_position->replaceItemList(_("Truncated Projection") + string("\n0\n")
	        + _("Lens at Dome Center") + string("\n1\n")
	        + _("Lens Below Dome Center") + string("\n2\n") , 0);

	tui_admin_info->setLabel(string("8.9 ") + _("Info: "));

	tui_admin_shear->setLabel(string("8.10 ") + _("Video Shear: "));

	tui_admin_center_hoffset->setLabel(string("8.11 ") + _("Video Offset: "));

	tui_admin_password->setLabel(string("8.12 ") + _("Reset Password: "));
	tui_admin_password->translateActions();

}


// Display the tui
void UI::draw_tui(void)
{

	// Normal transparency mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	Vec3d center = core->getViewportCenter();
	float x = center[0];
	float y = center[1];
	float shift = center[2];  // viewport radius

	////	int shift = (int)(M_SQRT2 / 2 * MY_MIN(core->getViewportWidth()/2, core->getViewportHeight()/2));
	//	int shift = MY_MIN(core->getViewportWidth()/2, core->getViewportHeight()/2);

	if (!core->getFlagGravityLabels()) {
		// for horizontal tui move to left edge of screen kludge
		shift = 0;
		x = core->getViewportPosX() + int(0.1*core->getViewportWidth());
		y = core->getViewportPosY() + int(0.1*core->getViewportHeight());
	}

	if (tui_root) {
		glColor3f(1,1,1);
		//		core->printGravity(tuiFont, x+38, y-shift + 38, s_tui::start_tui + tui_root->getString() + s_tui::end_tui, 0);
		core->printGravity(tuiFont, 5, 190, 
						   s_tui::start_tui + tui_root->getString() + s_tui::end_tui, 1, 0);  // DO NOT CACHE!
	}
}

int UI::handle_keys_tui(Uint16 key, s_tui::S_TUI_VALUE state)
{
	return tui_root->onKey(key, state);
}

// Update all the core parameters with values taken from the tui widgets
void UI::tui_cb1(void)
{
	// 2. Date & Time
	app->PresetSkyTime 		= tui_time_presetskytime->getJDay();
	app->StartupTimeMode 		= string(tui_time_startuptime->getCurrent());

}

// Update all the tui widgets with values taken from the core parameters
void UI::tui_update_widgets(void)
{
	if (!FlagShowTuiMenu) return;

	// 1. Location
	tui_location_latitude->setValue(core->getObservatory()->get_latitude());
	tui_location_longitude->setValue(core->getObservatory()->get_longitude());
	tui_location_altitude->setValue(core->getObservatory()->get_altitude());
	tui_location_heading->setValue(core->getHeading());


	// 2. Date & Time
	tui_time_skytime->setJDay(core->getJDay() + app->get_GMT_shift(core->getJDay())*JD_HOUR);
	tui_time_settmz->settz(app->get_custom_tz_name());
	tui_time_presetskytime->setJDay(app->PresetSkyTime);
	tui_time_startuptime->setCurrent(string(app->StartupTimeMode));
	tui_time_displayformat->setCurrent(string(app->get_time_format_str()));
	tui_time_dateformat->setCurrent(string(app->get_date_format_str()));

	//	cout << "Updating tui value to " << app->DayKeyMode << endl;
	tui_time_day_key->setCurrent(string(app->DayKeyMode));

	// 3. general
	tui_general_landscape->setValue(string(core->getObservatory()->get_landscape_name()));
	tui_general_sky_culture->setValue(string(core->getSkyCultureDir()));
	tui_general_sky_locale->setValue(string(core->getSkyLanguage()));

	// 4. Stars
	tui_stars_show->setValue(core->getFlagStars());
	tui_star_labelmaxmag->setValue(core->getMaxMagStarName());
	tui_stars_twinkle->setValue(core->getStarTwinkleAmount());
	tui_star_magscale->setValue(core->getStarMagScale());
	tui_star_limitingmag->setValue(core->getStarLimitingMag());

	// 5. Colors
	tui_colors_const_line_color->setVector(core->getColorConstellationLine());
	tui_colors_const_label_color->setVector(core->getColorConstellationNames());
	tui_colors_cardinal_color->setVector(core->getColorCardinalPoints());
	tui_colors_const_art_intensity->setValue(core->getConstellationArtIntensity());
	tui_colors_const_art_color->setVector(core->getColorConstellationArt());
	tui_colors_const_boundary_color->setVector(core->getColorConstellationBoundaries());
	tui_colors_planet_names_color->setVector(core->getColorPlanetsNames());
	tui_colors_planet_orbits_color->setVector(core->getColorPlanetsOrbits());
	tui_colors_satellite_orbits_color->setVector(core->getColorSatelliteOrbits());
	tui_colors_object_trails_color->setVector(core->getColorPlanetsTrails());
	tui_colors_meridian_color->setVector(core->getColorMeridianLine());
	tui_colors_azimuthal_color->setVector(core->getColorAzimutalGrid());
	tui_colors_equatorial_color->setVector(core->getColorEquatorGrid());
	tui_colors_equator_color->setVector(core->getColorEquatorLine());
	tui_colors_ecliptic_color->setVector(core->getColorEclipticLine());
	tui_colors_nebula_label_color->setVector(core->getColorNebulaLabels());
	tui_colors_nebula_circle_color->setVector(core->getColorNebulaCircle());
	tui_colors_precession_circle_color->setVector(core->getColorPrecessionCircle());
	tui_colors_circumpolar_circle_color->setVector(core->getColorCircumpolarCircle());
	tui_colors_galactic_color->setVector(core->getColorGalacticGrid());

	// *** Effects
	tui_effect_pointobj->setValue(core->getFlagPointStar());
	tui_effect_zoom_duration->setValue(core->getAutomoveDuration());
	tui_effect_flight_duration->setValue(core->getFlightDuration());
	tui_effect_manual_zoom->setValue(core->getFlagManualAutoZoom());
	tui_effect_object_scale->setValue(core->getStarScale());
	tui_effect_star_size_limit->setValue(core->getStarSizeLimit());
	tui_effect_planet_size_limit->setValue(core->getPlanetsSizeLimit());
	tui_effect_milkyway_intensity->setValue(core->getMilkyWayIntensity());
	tui_effect_cursor_timeout->setValue(MouseCursorTimeout);
	tui_effect_light_pollution->setValue(core->getLightPollutionLimitingMagnitude());
	tui_effect_nebulae_label_magnitude->setValue(core->getNebulaMaxMagHints());
	tui_effect_light_travel->setValue(core->getFlagLightTravelTime());
	tui_effect_view_offset->setValue(core->getViewOffset());
	tui_effect_antialias->setValue(core->getFlagAntialiasLines());
	tui_effect_line_width->setValue(core->getLineWidth());

	// 7. Scripts
	// each fresh time enter needs to reset to select message
	if (app->SelectedScript=="") {
		tui_scripts_local->setCurrent(_(TUI_SCRIPT_MSG));

		tui_scripts_internal->setCurrent(_(TUI_SCRIPT_MSG));

// changes 20070103 to split usb and cd
		if (ScriptDirectoryRead) {
			tui_scripts_removeable->setCurrent(_(TUI_SCRIPT_MSG));
		} else {
			// no directory mounted, so put up message
			tui_scripts_removeable->replaceItemList(_("Arrow down to load list."),0);
		}

		if (ScriptDirectoryRead2) {
			tui_scripts_usb->setCurrent(_(TUI_SCRIPT_MSG));
		} else {
			tui_scripts_usb->replaceItemList(_("Arrow down to load list."),0);
		}
	}

	// 8. admin
	tui_admin_setlocale->setValue( string(app->getAppLanguage()) );
	tui_admin_lens_position->setValue( Utility::intToString(
	                                       core->getProjectorConfiguration()));

	tui_admin_offset_x->setValue( 100 * core->getProjectorOffsetX() );
	tui_admin_offset_y->setValue( 100 * core->getProjectorOffsetY() );
	tui_admin_shear->setValue( core->getProjectorShearHorz() * 100 );
	tui_admin_center_hoffset->setValue( core->getCenterHorizontalOffset() );
}

// Launch script to set time zone in the system locales
// TODO : this works only if the system manages the TZ environment
// variables of the form "Europe/Paris". This is not the case on windows
// so everything migth have to be re-done internaly :(
void UI::tui_cb_settimezone(void)
{
	// Don't call the script anymore coz it's pointless
	// system( ( core->getDataDir() + "script_set_time_zone " + tui_time_settmz->getCurrent() ).c_str() );
	app->set_custom_tz_name(tui_time_settmz->gettz());
}

// Set time format mode
void UI::tui_cb_settimedisplayformat(void)
{
	app->set_time_format_str(string(tui_time_displayformat->getCurrent()));
	app->set_date_format_str(string(tui_time_dateformat->getCurrent()));
}

// 7. Administration actions functions

// Load default configuration
void UI::tui_cb_admin_load_default(void)
{
	app->init();
	tuiUpdateIndependentWidgets();

// - placing in init_parser.c would be safer, but more disruptive
#ifndef DESKTOP
	system( ( core->getDataDir() + "script_load_config_after " ).c_str() );
#endif

}

// Save to default configuration
void UI::tui_cb_admin_save_default(void)
{

// - placing in init_parser.c would be safer, but more disruptive
#ifndef DESKTOP
	system( ( core->getDataDir() + "script_save_config_before " ).c_str() );
#endif

	app->saveCurrentConfig(AppSettings::Instance()->getConfigFile());

#ifndef DESKTOP
	system( ( core->getDataDir() + "script_save_config_after " ).c_str() );
#endif


}

// Launch script for internet update
void UI::tui_cb_admin_updateme(void)
{
	// Cleanup all database contents, we're going down
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> obj_cursor(dbCursorForUpdate);
		if( obj_cursor.select() )
			obj_cursor.removeAll();

		dbCursor<TZRecord> tz_cursor(dbCursorForUpdate);
		if( tz_cursor.select() )
			tz_cursor.removeAll();

		SharedData::Instance()->DB()->commit();
	}

	// Release handle to shared memory and database
	SharedData::Destroy(true);

	::system( ( core->getDataDir() + "script_internet_update" ).c_str() );

	app->quit();
}


// Launch script for shutdown, then exit
void UI::tui_cb_admin_shutdown(void)
{
	if( AppSettings::Instance()->Digitarium() )
		::system( ( core->getDataDir() + "script_shutdown" ).c_str() );
	app->quit();
}


// Launch script for shutdown, then exit
void UI::tui_cb_admin_sync_drive(void)
{
	::system( ( core->getDataDir() + "script_sync_drive" ).c_str() );

	// Reread internal script directory
	tui_scripts_internal->replaceItemList(_(TUI_SCRIPT_MSG) + string("\n")
	                                      + string(app->scripts->get_script_list(core->getScriptDir() +  SCRIPT_INTERNAL_DISK + "/scripts/")), 0);
	tui_scripts_internal->setCurrent(_(TUI_SCRIPT_MSG));

}


// Set a new landscape skin
void UI::tui_cb_tui_general_change_landscape(void)
{
	app->commander->execute_command(string("set landscape_name " +  string(tui_general_landscape->getCurrent())));
}


// Set a new sky culture
void UI::tui_cb_tui_general_change_sky_culture(void)
{

	//	core->setSkyCulture(tui_general_sky_culture->getCurrent());
	app->commander->execute_command( string("set sky_culture ") + string(tui_general_sky_culture->getCurrent()));
}

// Set a new sky locale
void UI::tui_cb_tui_general_change_sky_locale(void)
{
	// wcout << "set sky locale to " << tui_general_sky_locale->getCurrent() << endl;
	app->commander->execute_command( string("set sky_locale " + string(tui_general_sky_locale->getCurrent())));
}


// callback for changing scripts from removeable media
void UI::tui_cb_scripts_removeable()
{

	if (!ScriptDirectoryRead) {
		// read scripts from mounted disk
		string script_list = app->scripts->get_script_list(core->getScriptDir() + SCRIPT_REMOVEABLE_DISK + "/scripts/");
		tui_scripts_removeable->replaceItemList(_(TUI_SCRIPT_MSG) + string("\n") + string(script_list),0);
		ScriptDirectoryRead = 1;
		tui_scripts_removeable->setCurrent(_(TUI_SCRIPT_MSG));
	}

	if (tui_scripts_removeable->getCurrent()==_(TUI_SCRIPT_MSG)) {
		app->SelectedScript = "";
	} else {
		app->SelectedScript = string(tui_scripts_removeable->getCurrent());
		app->SelectedScriptDirectory = core->getScriptDir() + SCRIPT_REMOVEABLE_DISK + "/scripts/";
		// to avoid confusing user, clear out local script selection as well
		tui_scripts_local->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_internal->setCurrent(_(TUI_SCRIPT_MSG));

		if (ScriptDirectoryRead2) tui_scripts_usb->setCurrent(_(TUI_SCRIPT_MSG));
	}
}

// callback for changing scripts from usb media
void UI::tui_cb_scripts_usb()
{

	if (!ScriptDirectoryRead2) {
		//// read scripts from mounted disk
		ScriptDirectoryRead2 = 1;

		string script_list = app->scripts->get_script_list(core->getScriptDir() + SCRIPT_USB_DISK + "/scripts/");
		tui_scripts_usb->replaceItemList(_(TUI_SCRIPT_MSG) + string("\n") + string(script_list),0);
		tui_scripts_usb->setCurrent(_(TUI_SCRIPT_MSG));
	}

	if (tui_scripts_usb->getCurrent()==_(TUI_SCRIPT_MSG)) {
		app->SelectedScript = "";
	} else {
		app->SelectedScript = string(tui_scripts_usb->getCurrent());
		app->SelectedScriptDirectory = core->getScriptDir() + SCRIPT_USB_DISK + "/scripts/";

		// to avoid confusing user, clear out local script selection as well
		tui_scripts_local->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_internal->setCurrent(_(TUI_SCRIPT_MSG));

		if (ScriptDirectoryRead) tui_scripts_removeable->setCurrent(_(TUI_SCRIPT_MSG));
	}
}



// callback for changing scripts from local directory
void UI::tui_cb_scripts_local()
{

	if (tui_scripts_local->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		app->SelectedScript = string(tui_scripts_local->getCurrent());
		app->SelectedScriptDirectory = core->getScriptDir() + SCRIPT_LOCAL_DISK;

		// to reduce confusion for user, clear out removeable script selection as well
		if (ScriptDirectoryRead) tui_scripts_removeable->setCurrent(_(TUI_SCRIPT_MSG));

		if (ScriptDirectoryRead2) tui_scripts_usb->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_internal->setCurrent(_(TUI_SCRIPT_MSG));

	} else {
		app->SelectedScript = "";
	}
}

// callback for changing scripts from internal directory
void UI::tui_cb_scripts_internal()
{

	if (tui_scripts_internal->getCurrent()!=_(TUI_SCRIPT_MSG)) {

		app->SelectedScript = string(tui_scripts_internal->getCurrent());
		app->SelectedScriptDirectory = core->getScriptDir() + SCRIPT_INTERNAL_DISK  + "/scripts/";

		// to reduce confusion for user, clear out removeable script selection as well
		if (ScriptDirectoryRead) tui_scripts_removeable->setCurrent(_(TUI_SCRIPT_MSG));

		if (ScriptDirectoryRead2) tui_scripts_usb->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_local->setCurrent(_(TUI_SCRIPT_MSG));

	} else {
		app->SelectedScript = "";
	}

}



// change UI locale
void UI::tui_cb_admin_set_locale()
{

	app->setAppLanguage(string(tui_admin_setlocale->getCurrent()));
}

// change projector configuration
void UI::tui_cb_admin_lens_position()
{

	core->setProjectorConfiguration(
	    str_to_int(
	        string(
	            tui_admin_lens_position->getCurrent())));

	core->setProjectorOffset(tui_admin_offset_x->getValue()/100.0,
	                         tui_admin_offset_y->getValue()/100.0);
	core->setProjectorShearHorz(tui_admin_shear->getValue()/100.0);

}


// change heading or view offset
void UI::tui_cb_viewport_related()
{

	core->setHeading(tui_location_heading->getValue(),
	                 int(tui_effect_zoom_duration->getValue()*1000));  // TEMP temporarily using zoom duration
	core->setViewOffset(tui_effect_view_offset->getValue());
}


void UI::tui_cb_center_hoffset()
{

	core->setCenterHorizontalOffset(tui_admin_center_hoffset->getValue());

}



void UI::tui_cb_effects_milkyway_intensity()
{

	std::ostringstream oss;
	oss << "set milky_way_intensity " << tui_effect_milkyway_intensity->getValue();
	app->commander->execute_command(oss.str());
}

void UI::tui_cb_setlocation()
{

	// change to human readable coordinates with current values, then change
	core->getObservatory()->set_longitude(core->getObservatory()->get_longitude());

	core->getObservatory()->move_to(tui_location_latitude->getValue(),
	                               tui_location_longitude->getValue(),
	                               tui_location_altitude->getValue(),
	                               int(tui_effect_zoom_duration->getValue()*1000),  // TEMP temporarily using zoom duration
	                               string(""),
	                               1); // use relative calculated duration

}


void UI::tui_cb_stars()
{
	// 4. Stars
	std::ostringstream oss;

	oss << "flag stars " << tui_stars_show->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set max_mag_star_name " << tui_star_labelmaxmag->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set star_twinkle_amount " << tui_stars_twinkle->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set star_mag_scale " << tui_star_magscale->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set star_limiting_mag " << tui_star_limitingmag->getValue();
	app->commander->execute_command(oss.str());

}

void UI::tui_cb_effects()
{

	// *** Effects
	std::ostringstream oss;

	oss << "flag point_star " << tui_effect_pointobj->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set auto_move_duration " << tui_effect_zoom_duration->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set flight_duration " << tui_effect_flight_duration->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "flag manual_zoom " << tui_effect_manual_zoom->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set star_scale " << tui_effect_object_scale->getValue();
	app->commander->execute_command(oss.str());

	core->setStarSizeLimit( tui_effect_star_size_limit->getValue() );

	core->setPlanetsSizeLimit( tui_effect_planet_size_limit->getValue() );

	MouseCursorTimeout = tui_effect_cursor_timeout->getValue();  // never recorded

	oss.str("");
	oss << "set light_pollution_limiting_magnitude " << tui_effect_light_pollution->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "flag light_travel_time " << tui_effect_light_travel->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "set line_width " << tui_effect_line_width->getValue();
	app->commander->execute_command(oss.str());

	oss.str("");
	oss << "flag antialias_lines " << tui_effect_antialias->getValue();
	app->commander->execute_command(oss.str());
	

}


// set sky time
void UI::tui_cb_sky_time()
{
	std::ostringstream oss;
	oss << "date local " << string(tui_time_skytime->getDateString());
	app->commander->execute_command(oss.str());
}


// set nebula label limit
void UI::tui_cb_effects_nebulae_label_magnitude()
{
	std::ostringstream oss;
	oss << "set max_mag_nebula_name " << tui_effect_nebulae_label_magnitude->getValue();
	app->commander->execute_command(oss.str());
}


void UI::tui_cb_change_color()
{
	core->setColorConstellationLine( tui_colors_const_line_color->getVector() );
	core->setColorConstellationNames( tui_colors_const_label_color->getVector() );
	core->setColorCardinalPoints( tui_colors_cardinal_color->getVector() );
	core->setConstellationArtIntensity(tui_colors_const_art_intensity->getValue() );
	core->setColorConstellationArt( tui_colors_const_art_color->getVector() );
	core->setColorConstellationBoundaries(tui_colors_const_boundary_color->getVector() );
	// core->setColorStarNames(
	// core->setColorStarCircles(
	core->setColorPlanetsOrbits(tui_colors_planet_orbits_color->getVector() );
	core->setColorSatelliteOrbits(tui_colors_satellite_orbits_color->getVector() );
	core->setColorPlanetsNames(tui_colors_planet_names_color->getVector() );
	core->setColorPlanetsTrails(tui_colors_object_trails_color->getVector() );
	core->setColorAzimutalGrid(tui_colors_azimuthal_color->getVector() );
	core->setColorEquatorGrid(tui_colors_equatorial_color->getVector() );
	core->setColorEquatorLine(tui_colors_equator_color->getVector() );
	core->setColorEclipticLine(tui_colors_ecliptic_color->getVector() );
	core->setColorMeridianLine(tui_colors_meridian_color->getVector() );
	core->setColorNebulaLabels(tui_colors_nebula_label_color->getVector() );
	core->setColorNebulaCircle(tui_colors_nebula_circle_color->getVector() );
	core->setColorPrecessionCircle(tui_colors_precession_circle_color->getVector() );
	core->setColorCircumpolarCircle(tui_colors_circumpolar_circle_color->getVector() );
	core->setColorGalacticGrid(tui_colors_galactic_color->getVector() );
}


void UI::tui_cb_location_change_planet()
{
	//	core->setHomePlanet( string( tui_location_planet->getCurrent() ) );
	//	wcout << "set home planet " << tui_location_planet->getCurrent() << endl;
	app->commander->execute_command(string("set home_planet \"") +
	                                string( tui_location_planet->getCurrent() ) +
	                                "\"");
}

void UI::tui_cb_day_key()
{

	app->DayKeyMode = string(tui_time_day_key->getCurrent());
	//	cout << "Set from tui value DayKeyMode to " << app->DayKeyMode << endl;
}

// Update widgets that don't always match current settings with current settings
void UI::tuiUpdateIndependentWidgets(void)
{

	// Since some tui options don't immediately affect actual settings
	// reset those options to the current values now
	// (can not do this in tui_update_widgets)

	tui_location_planet->setValue(string(core->getObservatory()->getHomePlanetEnglishName()));

// - also clear out script lists as media may have changed
	ScriptDirectoryRead = 0;
	ScriptDirectoryRead2 = 0;
}

