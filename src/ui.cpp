/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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

// Class which handles the User Interface

#include <iostream>
#include <iomanip>
#include <algorithm>
#include "app.h"
#include "ui.h"
#include "stellastro.h"

////////////////////////////////////////////////////////////////////////////////
//								CLASS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

UI::UI(Core * _core, App * _app) :
		shiftModifier(0), specialModifier(0),

		baseFont(NULL),
		courierFont(NULL),
		tuiFont(NULL),

		FlagHelp(false), FlagInfos(false), FlagConfig(false), FlagSearch(false), FlagShowTuiMenu(0),

		top_bar_ctr(NULL),
		top_bar_date_lbl(NULL),
		top_bar_hour_lbl(NULL),
		top_bar_fps_lbl(NULL),
		top_bar_appName_lbl(NULL),
		top_bar_fov_lbl(NULL),

		bt_flag_ctr(NULL),
		bt_flag_constellation_draw(NULL),
		bt_flag_constellation_name(NULL),
		bt_flag_constellation_art(NULL),
		bt_flag_azimuth_grid(NULL),
		bt_flag_equator_grid(NULL),
		bt_flag_ground(NULL),
		bt_flag_cardinals(NULL),
		bt_flag_atmosphere(NULL),
		bt_flag_nebula_name(NULL),
		bt_flag_help(NULL),
		bt_flag_equatorial_mode(NULL),
		bt_flag_config(NULL),
//		bt_flag_chart(NULL),
		bt_flag_planet(NULL),
		bt_flag_search(NULL),
		bt_script(NULL),
		bt_flag_goto(NULL),
		bt_flip_horz(NULL),
		bt_flip_vert(NULL),
		bt_flag_help_lbl(NULL),
		info_select_ctr(NULL),
		info_select_txtlbl(NULL),

		licence_win(NULL),
		licence_txtlbl(NULL),

		help_win(NULL),
		help_txtlbl(NULL),

		config_win(NULL),
		search_win(NULL),
		dialog_win(NULL),
		tui_root(NULL),

		ShiftTimeLeft(0),
		SpecialTimeLeft(0)

{
	if (!_core) {
		printf("ERROR : In stel_ui constructor, unvalid core.");
		exit(-1);
	}
	core = _core;
	app = _app;
	is_dragging = false;
	waitOnLocation = true;
	opaqueGUI = true;
	initialised = false;
}

/**********************************************************************************/
UI::~UI()
{
	delete desktop;
	desktop = NULL;
	delete baseFont;
	baseFont = NULL;
	delete baseTex;
	baseTex = NULL;
	delete flipBaseTex;
	flipBaseTex = NULL;
	delete courierFont;
	courierFont = NULL;
	delete tuiFont;
	tuiFont = NULL;
	delete tex_up;
	tex_up = NULL;
	delete tex_down;
	tex_down = NULL;
	if (tui_root) delete tui_root;
	tui_root=NULL;
	Component::deleteScissor();
}

////////////////////////////////////////////////////////////////////////////////
void UI::init(const InitParser& conf)
{

	if (initialised) {

		// delete existing objects before recreating
		if (baseFont) delete baseFont;
		if (courierFont) delete courierFont;
		if (baseTex) delete baseTex;
		if (flipBaseTex) delete flipBaseTex;
		if (tex_up) delete tex_up;
		if (tex_down) delete tex_down;
		if (desktop) delete desktop;
	}

	// Ui section
	FlagShowFps			= conf.get_boolean("gui:flag_show_fps");
	FlagMenu			= conf.get_boolean("gui:flag_menu");
	FlagHelp			= conf.get_boolean("gui:flag_help");
	FlagInfos			= conf.get_boolean("gui:flag_infos");
	FlagShowTopBar		= conf.get_boolean("gui:flag_show_topbar");
	FlagShowTime		= conf.get_boolean("gui:flag_show_time");
	FlagShowDate		= conf.get_boolean("gui:flag_show_date");
	FlagShowAppName		= conf.get_boolean("gui:flag_show_appname");
	FlagShowFov			= conf.get_boolean("gui:flag_show_fov");
	FlagShowSelectedObjectInfo = conf.get_boolean("gui:flag_show_selected_object_info");
	BaseFontSize		= conf.get_double ("gui","base_font_size",12);
	BaseFontName        = conf.get_str("gui", "base_font_name", "sans");  // deprecated
	FlagShowScriptBar	= conf.get_boolean("gui","flag_show_script_bar",false);
	MouseCursorTimeout  = conf.get_double("gui","mouse_cursor_timeout",0);

	// Text ui section
	TuiFontSize		    = conf.get_double ("tui","base_font_size",18);
	FlagEnableTuiMenu = conf.get_boolean("tui:flag_enable_tui_menu");
	FlagShowGravityUi = conf.get_boolean("tui:flag_show_gravity_ui");
	FlagShowTuiDateTime = conf.get_boolean("tui:flag_show_tui_datetime");
	FlagShowTuiShortObjInfo = conf.get_boolean("tui:flag_show_tui_short_obj_info");

	ReferenceState state;
	state.show_tui_date_time = FlagShowTuiDateTime;
	state.show_tui_short_obj_info = FlagShowTuiShortObjInfo;
	SharedData::Instance()->References( state );

	BaseFontName = core->getDataDir() + BaseFontName;

	// TODO: can we get rid of this second font requirement?
	BaseCFontSize		= conf.get_double ("gui","base_cfont_size",12.5);
	BaseCFontName = core->getDataDir() + conf.get_str("gui", "base_cfont_name", "DejaVuSansMono.ttf");

	// Load standard font
	baseFont = new s_font(BaseFontSize, BaseFontName);
	if (!baseFont) {
		printf("ERROR WHILE CREATING FONT\n");
		exit(-1);
	}

	courierFont = new s_font(BaseCFontSize, BaseCFontName);
	if (!courierFont) {
		printf("ERROR WHILE CREATING FONT\n");
		exit(-1);
	}

	// set up mouse cursor timeout
	MouseTimeLeft = MouseCursorTimeout*1000;

	// Create standard texture
	baseTex = new s_texture("backmenu.png", TEX_LOAD_TYPE_PNG_ALPHA);
	flipBaseTex = new s_texture("backmenu_flip.png", TEX_LOAD_TYPE_PNG_ALPHA);

	tex_up = new s_texture("up.png");
	tex_down = new s_texture("down.png");

	// Set default Painter
	Painter p(baseTex, baseFont, s_color(0.5, 0.5, 0.5), s_color(1., 1., 1.));
	Component::setDefaultPainter(p);

	Component::initScissor(core->getDisplayWidth(), core->getDisplayHeight());

	desktop = new Container(true);
	desktop->reshape(0,0,core->getDisplayWidth(),core->getDisplayHeight());

	bt_flag_help_lbl = new Label("ERROR...");
	bt_flag_help_lbl->setPos(3,core->getDisplayHeight()-41-(int)baseFont->getDescent());
	bt_flag_help_lbl->setVisible(0);

	bt_flag_time_control_lbl = new Label("ERROR...");
	bt_flag_time_control_lbl->setPos(core->getDisplayWidth()-210,core->getDisplayHeight()-41-(int)baseFont->getDescent());
	bt_flag_time_control_lbl->setVisible(0);

	// Info on selected object
	info_select_ctr = new Container();
	info_select_ctr->reshape(0,15,300,200);
	info_select_txtlbl = new TextLabel();
	info_select_txtlbl->reshape(5,5,550,202);
	info_select_ctr->setVisible(1);
	info_select_ctr->addComponent(info_select_txtlbl);
	info_select_ctr->setGUIColorSchemeMember(false);
	desktop->addComponent(info_select_ctr);

	// message window
	message_txtlbl = new TextLabel();
	message_txtlbl->adjustSize();
	message_txtlbl->setPos(10,10);
	message_win = new StdTransBtWin("Message", 5000);
	//message_win->setOpaque(opaqueGUI);
	message_win->reshape(core->getDisplayWidth()/2 -350, core->getDisplayHeight()/2 -100, 700, 200);
	message_win->addComponent(message_txtlbl);
	message_win->setVisible(false);
	desktop->addComponent(message_win);

	desktop->addComponent(createTopBar());
	desktop->addComponent(createFlagButtons(conf));
	desktop->addComponent(createTimeControlButtons());
	desktop->addComponent(bt_flag_help_lbl);
	desktop->addComponent(bt_flag_time_control_lbl);

	dialog_win = new StdDlgWin(APP_NAME);
	//dialog_win->setOpaque(opaqueGUI);
	dialog_win->setDialogCallback(callback<void>(this, &UI::dialogCallback));
	desktop->addComponent(dialog_win);

	desktop->addComponent(createLicenceWindow());
	desktop->addComponent(createHelpWindow());
	desktop->addComponent(createConfigWindow());
	desktop->addComponent(createSearchWindow());

	initialised = true;

	setTitleObservatoryName(getTitleWithAltitude());
}


////////////////////////////////////////////////////////////////////////////////
void UI::show_message(string _message, int _time_out)
{
	// draws a message window to display a message to user
	// if timeout is zero, won't time out
	// otherwise use miliseconds

	// TODO figure out how to size better for varying message lengths

	message_txtlbl->setLabel(_message);
	message_txtlbl->adjustSize();
	message_win->set_timeout(_time_out);
	message_win->setVisible(1);
}

////////////////////////////////////////////////////////////////////////////////
Component* UI::createTopBar(void)
{

	top_bar_date_lbl = new Label("-", baseFont);
	top_bar_date_lbl->setPos(2,1);
	top_bar_hour_lbl = new Label("-", baseFont);
	top_bar_hour_lbl->setPos(110,1);
	top_bar_fps_lbl = new Label("-", baseFont);
	top_bar_fps_lbl->setPos(core->getDisplayWidth()-100,1);
	top_bar_fov_lbl = new Label("-", baseFont);
	top_bar_fov_lbl->setPos(core->getDisplayWidth()-220,1);
	top_bar_appName_lbl = new Label(APP_NAME, baseFont);
	top_bar_appName_lbl->setPos(core->getDisplayWidth()/2-top_bar_appName_lbl->getSizex()/2,1);
	top_bar_ctr = new FilledContainer();
	top_bar_ctr->reshape(0,0,core->getDisplayWidth(),(int)(baseFont->getLineHeight()+0.5)+5);
	top_bar_ctr->addComponent(top_bar_date_lbl);
	top_bar_ctr->addComponent(top_bar_hour_lbl);
	top_bar_ctr->addComponent(top_bar_fps_lbl);
	top_bar_ctr->addComponent(top_bar_fov_lbl);
	top_bar_ctr->addComponent(top_bar_appName_lbl);
	return top_bar_ctr;
}

////////////////////////////////////////////////////////////////////////////////
void UI::updateTopBar(void)
{
	top_bar_ctr->setVisible(FlagShowTopBar);
	if (!FlagShowTopBar) return;

	double jd = core->getJDay();

	if (FlagShowDate) {
		top_bar_date_lbl->setLabel(app->get_printable_date_local(jd));
		top_bar_date_lbl->adjustSize();
	}
	top_bar_date_lbl->setVisible(FlagShowDate);

	if (FlagShowTime) {
		top_bar_hour_lbl->setLabel(app->get_printable_time_local(jd));
		top_bar_hour_lbl->adjustSize();
	}
	top_bar_hour_lbl->setVisible(FlagShowTime);

	top_bar_appName_lbl->setVisible(FlagShowAppName);

	if (FlagShowFov) {
		stringstream wos;
		wos << "FOV=" << setprecision(3) << core->getFov() << "\u00B0";
		top_bar_fov_lbl->setLabel(wos.str());
		top_bar_fov_lbl->adjustSize();
	}
	top_bar_fov_lbl->setVisible(FlagShowFov);

	if (FlagShowFps) {
		stringstream wos;
		wos << "FPS=" << int(app->fps + 0.5);
		top_bar_fps_lbl->setLabel(wos.str());
		top_bar_fps_lbl->adjustSize();
	}
	top_bar_fps_lbl->setVisible(FlagShowFps);
}

// Create the button panel in the lower left corner
#define UI_PADDING 5
#define UI_BT 25
#define UI_SCRIPT_BAR 400
Component* UI::createFlagButtons(const InitParser &conf)
{
	int x = 0;

	bt_flag_constellation_draw = new FlagButton(false, NULL, "bt_constellations.png");
	bt_flag_constellation_draw->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_constellation_draw->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_constellation_name = new FlagButton(false, NULL, "bt_const_names.png");
	bt_flag_constellation_name->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_constellation_name->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_constellation_art = new FlagButton(false, NULL, "bt_constart.png");
	bt_flag_constellation_art->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_constellation_art->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_azimuth_grid = new FlagButton(false, NULL, "bt_azgrid.png");
	bt_flag_azimuth_grid->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_azimuth_grid->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_equator_grid = new FlagButton(false, NULL, "bt_eqgrid.png");
	bt_flag_equator_grid->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_equator_grid->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_ground = new FlagButton(false, NULL, "bt_ground.png");
	bt_flag_ground->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_ground->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_cardinals = new FlagButton(false, NULL, "bt_cardinal.png");
	bt_flag_cardinals->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_cardinals->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_atmosphere = new FlagButton(false, NULL, "bt_atmosphere.png");
	bt_flag_atmosphere->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_atmosphere->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_nebula_name = new FlagButton(false, NULL, "bt_nebula.png");
	bt_flag_nebula_name->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_nebula_name->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_help = new FlagButton(false, NULL, "bt_help.png");
	bt_flag_help->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_help->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_equatorial_mode = new FlagButton(false, NULL, "bt_follow.png");
	bt_flag_equatorial_mode->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_equatorial_mode->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_config = new FlagButton(false, NULL, "bt_config.png");
	bt_flag_config->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_config->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));



	bt_flag_planet = new FlagButton(false, NULL, "bt_planet_label.png");
	bt_flag_planet->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_planet->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_quit = new FlagButton(true, NULL, "bt_quit.png");
	bt_flag_quit->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_quit->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_search = new FlagButton(true, NULL, "bt_search.png");
	bt_flag_search->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_search->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_script = new EditBox();
	bt_script->setAutoFocus(false);
	bt_script->setSize(UI_SCRIPT_BAR-1,24);
	bt_script->setOnKeyCallback(callback<void>(this, &UI::cbEditScriptKey));
	bt_script->setOnReturnKeyCallback(callback<void>(this, &UI::cbEditScriptExecute));
	bt_script->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_goto = new FlagButton(true, NULL, "bt_goto.png");
	bt_flag_goto->setOnPressCallback(callback<void>(this, &UI::cb));
	bt_flag_goto->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));

	bt_flag_ctr = new FilledContainer();
	bt_flag_ctr->addComponent(bt_flag_constellation_draw);
	bt_flag_constellation_draw->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_constellation_name);
	bt_flag_constellation_name->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_constellation_art);
	bt_flag_constellation_art->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_azimuth_grid);
	bt_flag_azimuth_grid->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_equator_grid);
	bt_flag_equator_grid->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_ground);
	bt_flag_ground->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_cardinals);
	bt_flag_cardinals->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_atmosphere);
	bt_flag_atmosphere->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_planet);
	bt_flag_planet->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_nebula_name);
	bt_flag_nebula_name->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_equatorial_mode);
	bt_flag_equatorial_mode->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_goto);
	bt_flag_goto->setPos(x,0);
	x+=UI_BT;
	if (conf.get_boolean("gui","flag_show_flip_buttons",false)) {
		bt_flip_horz = new FlagButton(true, NULL, "bt_flip_horz.png");
		bt_flip_horz->setOnPressCallback(callback<void>(this, &UI::cb));
		bt_flip_horz->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));
		bt_flag_ctr->addComponent(bt_flip_horz);
		bt_flip_horz->setPos(x,0);
		x+=UI_BT;
		bt_flip_vert = new FlagButton(true, NULL, "bt_flip_vert.png");
		bt_flip_vert->setOnPressCallback(callback<void>(this, &UI::cb));
		bt_flip_vert->setOnMouseInOutCallback(callback<void>(this, &UI::cbr));
		bt_flag_ctr->addComponent(bt_flip_vert);
		bt_flip_vert->setPos(x,0);
		x+=UI_BT;
	}

	x+= UI_PADDING;
	bt_flag_ctr->addComponent(bt_script);
	bt_script->setPos(x,0);
	if (!FlagShowScriptBar) {
		bt_script->setVisible(false);
	} else {
		x+=UI_SCRIPT_BAR;
		x+= UI_PADDING;
	}

	bt_flag_ctr->addComponent(bt_flag_search);
	bt_flag_search->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_config);
	bt_flag_config->setPos(x,0);
	x+=UI_BT;
//	bt_flag_ctr->addComponent(bt_flag_chart);			bt_flag_chart->setPos(x,0); x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_help);
	bt_flag_help->setPos(x,0);
	x+=UI_BT;
	bt_flag_ctr->addComponent(bt_flag_quit);
	bt_flag_quit->setPos(x,0);
	x+=UI_BT;

	bt_flag_ctr->setOnMouseInOutCallback(callback<void>(this, &UI::bt_flag_ctrOnMouseInOut));
	bt_flag_ctr->reshape(0, core->getDisplayHeight()-25, x-1, 25);

	return bt_flag_ctr;

}

// Create the button panel in the lower right corner
Component* UI::createTimeControlButtons(void)
{
	bt_dec_time_speed = new FlagButton(false, NULL, "bt_rwd.png");
	bt_dec_time_speed->setOnPressCallback(callback<void>(this, &UI::bt_dec_time_speed_cb));
	bt_dec_time_speed->setOnMouseInOutCallback(callback<void>(this, &UI::tcbr));

	bt_real_time_speed = new FlagButton(false, NULL, "bt_realtime.png");
	bt_real_time_speed->setSize(24,24);
	bt_real_time_speed->setOnPressCallback(callback<void>(this, &UI::bt_real_time_speed_cb));
	bt_real_time_speed->setOnMouseInOutCallback(callback<void>(this, &UI::tcbr));

	bt_inc_time_speed = new FlagButton(false, NULL, "bt_fwd.png");
	bt_inc_time_speed->setOnPressCallback(callback<void>(this, &UI::bt_inc_time_speed_cb));
	bt_inc_time_speed->setOnMouseInOutCallback(callback<void>(this, &UI::tcbr));

	bt_time_now = new FlagButton(false, NULL, "bt_now.png");
	bt_time_now->setOnPressCallback(callback<void>(this, &UI::bt_time_now_cb));
	bt_time_now->setOnMouseInOutCallback(callback<void>(this, &UI::tcbr));

	bt_time_control_ctr = new FilledContainer();
	bt_time_control_ctr->addComponent(bt_dec_time_speed);
	bt_dec_time_speed->setPos(0,0);
	bt_time_control_ctr->addComponent(bt_real_time_speed);
	bt_real_time_speed->setPos(25,0);
	bt_time_control_ctr->addComponent(bt_inc_time_speed);
	bt_inc_time_speed->setPos(50,0);
	bt_time_control_ctr->addComponent(bt_time_now);
	bt_time_now->setPos(75,0);

	bt_time_control_ctr->setOnMouseInOutCallback(callback<void>(this, &UI::bt_time_control_ctrOnMouseInOut));
	bt_time_control_ctr->reshape(core->getDisplayWidth()-4*25-1, core->getDisplayHeight()-25, 4*25, 25);

	return bt_time_control_ctr;
}

void UI::bt_dec_time_speed_cb(void)
{
	app->FlagTimePause = 0;
	double s = core->getTimeSpeed();
	if (s>JD_SECOND) s/=10.;
	else if (s<=-JD_SECOND) s*=10.;
	else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
	else if (s>0. && s<=JD_SECOND) s=0.;
	core->setTimeSpeed(s);
}

void UI::bt_inc_time_speed_cb(void)
{
	app->FlagTimePause = 0;
	double s = core->getTimeSpeed();
	if (s>=JD_SECOND) s*=10.;
	else if (s<-JD_SECOND) s/=10.;
	else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
	else if (s>=-JD_SECOND && s<0.) s=0.;
	core->setTimeSpeed(s);
}

void UI::bt_real_time_speed_cb(void)
{
	app->FlagTimePause = 0;
	core->setTimeSpeed(JD_SECOND);
}

void UI::bt_time_now_cb(void)
{
	core->setJDay(NShadeDateTime::JulianFromSys());
}

////////////////////////////////////////////////////////////////////////////////
// Script edit command line

void UI::cbEditScriptInOut(void)
{
	if (bt_script->getIsMouseOver()) {
		bt_flag_help_lbl->setLabel(_("Script commander"));
	}
}

void UI::cbEditScriptKey(void)
{
	if (bt_script->getLastKey() == SDLK_SPACE || bt_script->getLastKey() == SDLK_TAB) {
		string command = bt_script->getText();
		transform(command.begin(), command.end(), command.begin(), ::tolower);
		if (bt_script->getLastKey() == SDLK_SPACE) command = command.substr(0,command.length()-1);
	} else if	(bt_script->getLastKey() == SDLK_ESCAPE) {
		bt_script->clearText();
	}
}

void UI::cbEditScriptExecute(void)
{
	unsigned long int delay;
	string command_string = bt_script->getText();
	cout << "Executing command: " << command_string << endl;

	bt_script->clearText();
	bt_script->setEditing(false);

	if (!app->commander->execute_command(command_string, delay, false))  // Not trusted to enable script path defaults
		bt_flag_help_lbl->setLabel(_("Invalid Script command"));
}

////////////////////////////////////////////////////////////////////////////////
void UI::cb(void)
{
	core->setFlagConstellationLines(bt_flag_constellation_draw->getState());
	core->setFlagConstellationNames(bt_flag_constellation_name->getState());
	core->setFlagConstellationArt(bt_flag_constellation_art->getState());
	core->setFlagAzimutalGrid(bt_flag_azimuth_grid->getState());
	core->setFlagEquatorGrid(bt_flag_equator_grid->getState());
	core->setFlagLandscape(bt_flag_ground->getState());
	core->setFlagCardinalsPoints(bt_flag_cardinals->getState());
	core->setFlagAtmosphere(bt_flag_atmosphere->getState());
	core->setFlagNebulaHints( bt_flag_nebula_name->getState() );
	if (bt_flip_horz) core->setFlipHorz( bt_flip_horz->getState() );
	if (bt_flip_vert) core->setFlipVert( bt_flip_vert->getState() );
	FlagHelp 				= bt_flag_help->getState();
	help_win->setVisible(FlagHelp);
	core->setMountMode(bt_flag_equatorial_mode->getState() ? Core::MOUNT_EQUATORIAL : Core::MOUNT_ALTAZIMUTAL);
	FlagConfig			= bt_flag_config->getState();
	core->setFlagPlanetsHints( bt_flag_planet->getState());
	config_win->setVisible(FlagConfig);

	FlagSearch			= bt_flag_search->getState();
	search_win->setVisible(FlagSearch);
	if (bt_flag_goto->getState()) core->gotoSelectedObject();
	bt_flag_goto->setState(false);

	if (!bt_flag_quit->getState()) app->quit();
}

void UI::bt_flag_ctrOnMouseInOut(void)
{
	if (bt_flag_ctr->getIsMouseOver()) bt_flag_help_lbl->setVisible(1);
	else bt_flag_help_lbl->setVisible(0);
}

void UI::bt_time_control_ctrOnMouseInOut(void)
{
	if (bt_time_control_ctr->getIsMouseOver()) bt_flag_time_control_lbl->setVisible(1);
	else bt_flag_time_control_lbl->setVisible(0);
}

void UI::cbr(void)
{
	if (bt_flag_constellation_draw->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Constellation Lines"));
	if (bt_flag_constellation_name->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Constellation Labels"));
	if (bt_flag_constellation_art->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Constellation Art"));
	if (bt_flag_azimuth_grid->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Azimuthal Grid"));
	if (bt_flag_equator_grid->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Equatorial Grid"));
	if (bt_flag_ground->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Ground"));
	if (bt_flag_cardinals->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Cardinal Points"));
	if (bt_flag_atmosphere->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Atmosphere"));
	if (bt_flag_nebula_name->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Nebula Labels"));
	if (bt_flag_help->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Help"));
	if (bt_flag_equatorial_mode->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Equatorial/Altazimuthal Mount"));
	if (bt_flag_config->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Configuration"));

	if (bt_flag_planet->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Body Labels"));
	if (bt_flag_quit->getIsMouseOver())
#ifndef MACOSX
		bt_flag_help_lbl->setLabel(_("Quit [CTRL + Q]"));
#else
		bt_flag_help_lbl->setLabel(_("Quit [CMD + Q]"));
#endif
	if (bt_flag_search->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Search for Object"));
	if (bt_script->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Script Command"));
	if (bt_flag_goto->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Track Selected Object"));
	if (bt_flip_horz && bt_flip_horz->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Flip Horizontally"));
	if (bt_flip_vert && bt_flip_vert->getIsMouseOver())
		bt_flag_help_lbl->setLabel(_("Flip Vertically"));
}

void UI::tcbr(void)
{
	if (bt_dec_time_speed->getIsMouseOver())
		bt_flag_time_control_lbl->setLabel(_("Decrease Time Rate"));
	if (bt_real_time_speed->getIsMouseOver())
		bt_flag_time_control_lbl->setLabel(_("Real Time Rate"));
	if (bt_inc_time_speed->getIsMouseOver())
		bt_flag_time_control_lbl->setLabel(_("Increase Time Rate"));
	if (bt_time_now->getIsMouseOver())
		bt_flag_time_control_lbl->setLabel(_("Return to Current Time"));
}

// The window containing the info (licence)
Component* UI::createLicenceWindow(void)
{

// - removed escaped characters
	licence_txtlbl = new TextLabel(
	    string("                 *   " APP_NAME " - " EDITION " Edition    *\n\n") +
	    "*   Copyright 2003-2011 Digitalis Education Solutions, Inc. et al.\n" +
	    "*   Copyright 2000-2008 Fabien Chereau et al.\n\n" +
	    "*" + _("   Please check for newer versions and send bug reports\n\
    and comments to us at: http://nightshadesoftware.org\n\n") +
	    "*   This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 3\n\
of the License, or (at your option) any later version.\n\n" +
	    "This program is distributed in the hope that it will be useful, but\n\
WITHOUT ANY WARRANTY; without even the implied\n\
warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.  See the GNU General Public\n\
License for more details.\n\n" +
	    "You should have received a copy of the GNU General Public\n\
License along with this program; if not, write to:\n" +
	    "Free Software Foundation, Inc.\n\
59 Temple Place - Suite 330\n\
Boston, MA  02111-1307, USA.\n\
http://www.fsf.org\n\n" +
	    "* Nightshade and StratoScript are trademarks of\n   Digitalis Education Solutions, Inc.");

	licence_txtlbl->adjustSize();
	licence_txtlbl->setPos(10,10);
	licence_win = new StdBtWin(_("Information"));
	//licence_win->setOpaque(opaqueGUI);
	licence_win->reshape(core->getDisplayWidth()/2 -300, core->getDisplayHeight()/2 -270, 600, 560);
	licence_win->addComponent(licence_txtlbl);
	licence_win->setVisible(FlagInfos);

	return licence_win;
}

Component* UI::createHelpWindow(void)
{

	help_text = new HashBox(30, 0.33);
	help_text->setPos(20,20);
	help_text->setSizex(450);
	help_text->addItemList( string(" \n<b>") + string(_("Movement and Selection:")) + "</b>\n"
							+ _("Arrow Keys") + "\n" + _("Pan View") + "\n"
							+ _("Page Up") + "\n" + _("Zoom In") + "\n"
							+ _("Page Down") + "\n" + _("Zoom Out") + "\n"
							+ _("Left Click") + "\n" + _("Select Object")  + "\n"
							+ _("Right Click") + "\n" + _("Unselect")  + "\n"
							+ _("CTRL + Left Click") + "\n" + _("Unselect")  + "\n"
							+ "\\\n" + _("Zoom out (planet and satellites views)")  + "\n"
							+ "/\n" + _("Zoom to selected object")  + "\n"
							+ "SPACE\n" + _("Center on selected object") + "\n \n \n"

							+ " \n<b>" + _("Display Options:") + "</b>\n"
							+ "U\n" + _("Equatorial/altazimuthal mount") + "\n"
							+ "F11\n" + _("Toggle fullscreen if possible.") + "\n"
							+ "C\n" + _("Constellation Lines") + "\n"
							+ "V\n" + _("Constellation Labels") + "\n"
							+ "B\n" + _("Constellation Boundaries") + "\n"
							+ "R\n" + _("Constellation Art") + "\n"
							+ "E\n" + _("Equatorial Grid") + "\n"
							+ "Z\n" + _("Meridian") + "\n"
							+ "N\n" + _("Nebula Labels") + "\n"
							+ "P\n" + _("Body Labels") + "\n"
							+ "D\n" + _("Star Labels") + "\n"
							+ "S\n" + _("Stars") + "\n"
							+ "G\n" + _("Ground") + "\n"
							+ "A\n" + _("Atmosphere") + "\n"
							+ "F\n" + _("Fog") + "\n"
							+ "Q\n" + _("Cardinal Points") + "\n"
							+ "O\n" + _("Toggle Moon Scaling") + "\n"
							+ "4 ,\n" + _("Ecliptic Line") + "\n"
							+ "5 .\n" + _("Equator Line") + "\n \n \n"

							+ " \n<b>" + _("Dialogs and Other Controls:") + "</b>\n"
							+ "H\n" + _("Help") + "\n"
							+ "I\n" + _("About") + "\n"
							+ "M\n" + _("Text Menu") + "\n"
							+ "1\n" + _("Configuration") + "\n"
							+ "CTRL + F\n" + _("Search for Object") + "\n"
							+ "CTRL + G\n" + _("Go to Selected Body") + "\n"
							+ "CTRL + R\n" + _("Toggle Script Recording") + "\n"
							+ "CTRL + S\n" + _("Take Screenshot") + "\n"
							+ "CTRL + SHIFT + V\n" + _("Toggle Video Frame Recording") + "\n \n \n"

							+ " \n<b>" + _("Time and Date:") + "</b>\n"
							+ "6\n" + _("Pause Time") + "\n"
							+ "7\n" + _("Stop Time") + "\n"
							+ "8\n" + _("Return to Current Time") + "\n"
							+ "J\n" + _("Decrease Time Rate") + "\n"
							+ "K\n" + _("Real Time Rate") + "\n"
							+ "L\n" + _("Increase Time Rate") + "\n"
							+ "-\n" + _("Back 24 Hours") + "\n"
							+ "=\n" + _("Forward 24 Hours") + "\n"
							+ "[\n" + _("Back 7 Days") + "\n"
							+ "]\n" + _("Forward 7 Days") + "\n \n \n"

							+ " \n<b>" + _("After shift key '`' (backtick):") + "</b>\n"
							+ "Z\n" + _("Azimuthal Grid") + "\n"
							+ "Q\n" + _("Galactic Grid") + "\n"
							+ "P\n" + _("Body Orbits") + "\n"
							+ ",\n" + _("Body Trails") + "\n"
							+ "A\n" + _("Clouds") + "\n"
							+ "SPACE\n" + _("Go to Selected Body") + "\n"
							+ "8\n" + _("Load Default Configuration") + "\n"
							+ "K\n" + _("Replay Last Script") + "\n"
							+ "5\n" + _("Tropic Lines") + "\n"
							+ "E\n" + _("Precession Circle") + "\n \n \n"
							
							+ " \n<b>" + _("During Script Playback:") + "</b>\n"
							+ "CTRL + C \n" + _("End Script") + "\n"
							+ "6\n" + _("Pause Script") + "\n"
							+ "7\n" + _("End Script") + "\n"
							+ "K\n" + _("Resume Script") + "\n"
							+ "L\n" + _("Fast Forward Script") + "\n \n \n"
							
							+ " \n<b>" + _("Miscellaneous:") + "</b>\n"
							+ "CTRL + D \n" + _("Play demo script") + "\n"
							+ "9\n" + _("Cycle through meteor shower rates") + "\n"
							+ "CTRL + Q \n" + _("Quit") + "\n"
		);

	help_win = new StdBtWin(_("Help"));
	help_win->reshape(core->getDisplayWidth()/2 -250, core->getDisplayHeight()/2 -300, 500, 600);
	help_win->addComponent(help_text);
	help_win->setVisible(FlagHelp);
	help_win->setOnHideBtCallback(callback<void>(this, &UI::help_win_hideBtCallback));
	return help_win;
}

void UI::help_win_hideBtCallback(void)
{
	help_win->setVisible(0);
}


/*******************************************************************/
void UI::draw(void)
{

	// Lines should be 1 pixel wide only and not use sky lines width
	glLineWidth(1);

	// draw first as windows should cover these up
	// also problem after 2dfullscreen with square viewport
	if (FlagShowGravityUi) draw_gravity_ui();
	if (getFlagShowTuiMenu()) draw_tui();

	// Special cool text transparency mode
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);

	app->set2DfullscreenProjection();	// 2D coordinate
	Component::enableScissor();

	glScalef(1, -1, 1);						// invert the y axis, down is positive
	glTranslatef(0, -core->getDisplayHeight(), 0);	// move the origin from the bottom left corner to the upper left corner

	desktop->draw();

	Component::disableScissor();
	app->restoreFrom2DfullscreenProjection();	// Restore the other coordinate

}

/*******************************************************************************/
int UI::handle_move(int x, int y)
{
	// Do not allow use of mouse while script is playing
	// otherwise script can get confused
	if (app->scripts->is_playing()) return 0;

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	if (desktop->onMove(x, y)) return 1;
	if (is_dragging) {
		if ((has_dragged || sqrtf((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y))>4.)) {
			has_dragged = true;
			core->setFlagTracking(false);
			core->dragView(previous_x, previous_y, x, y);
			previous_x = x;
			previous_y = y;
			return 1;
		}
	}
	return 0;
}

/*******************************************************************************/
int UI::handle_clic(Uint16 x, Uint16 y, S_GUI_VALUE button, S_GUI_VALUE state)
{
	// Do not allow use of mouse while script is playing
	// otherwise script can get confused
	if (app->scripts->is_playing()) return 0;

	// Make sure object pointer is turned on (script may have turned off)
	core->setFlagSelectedObjectPointer(true);

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	if (desktop->onClic((int)x, (int)y, button, state)) {
		has_dragged = false;
		is_dragging = false;
		return 1;
	}

	switch (button) {
	case S_GUI_MOUSE_RIGHT :
		break;
	case S_GUI_MOUSE_LEFT :
		if (state==S_GUI_PRESSED) {
			is_dragging = true;
			has_dragged = false;
			previous_x = x;
			previous_y = y;
		} else {
			is_dragging = false;
		}
		break;
	case S_GUI_MOUSE_MIDDLE :
		break;
	case S_GUI_MOUSE_WHEELUP :
		core->zoomTo(core->getAimFov()-app->MouseZoom*core->getAimFov()/60., 0.2);
		return 1;
	case S_GUI_MOUSE_WHEELDOWN :
		core->zoomTo(core->getAimFov()+app->MouseZoom*core->getAimFov()/60., 0.2);
		return 1;
	default:
		break;
	}

	// Manage the event for the main window
	{
		//if (state==S_GUI_PRESSED) return 1;
		// Deselect the selected object
		if (button==S_GUI_MOUSE_RIGHT && state==S_GUI_RELEASED) {
			app->commander->execute_command("select");
			return 1;
		}
		if (button==S_GUI_MOUSE_MIDDLE && state==S_GUI_RELEASED) {
			if (core->getFlagHasSelected()) {
				core->gotoSelectedObject();
				core->setFlagTracking(true);
			}
		}
		if (button==S_GUI_MOUSE_LEFT && state==S_GUI_RELEASED && !has_dragged) {
			// CTRL + left clic = right clic for 1 button mouse
			if (SDL_GetModState() & KMOD_CTRL) {
				app->commander->execute_command("select");
				return 1;
			}

			// Try to select object at that position
			core->findAndSelect(x, y);

			// If an object was selected update informations
			if (core->getFlagHasSelected()) updateInfoSelectString();
		}
	}
	return 0;
}


/*******************************************************************************/

// LSS HANDLE KEYS
#ifdef LSS
// TODO replace this with flexible keymapping feature
// odd extension to prevent compilation from makefile but inclusion in make dist
#include "ui-lss.hpp"
#else

// Standard handle keys
int UI::handle_keys(SDLKey key, SDLMod mod, Uint16 unicode, S_GUI_VALUE state)
{
	int retVal = 1;

	if (desktop->onKey(unicode, state))
		return 1;

	if (state==S_GUI_PRESSED) {
//printf("handle_keys: '%c'(%d), %d, 0x%04x\n",key,(int)key,unicode,mod);
		if (unicode >= 128) {
			// the user has entered an arkane symbol which cannot
			// be a key shortcut.
			return 0;
		}
		if (unicode >= 32) {
			// the user has entered a printable ascii character
			// see SDL_keysyms.h: the keysyms are cleverly matched to ascii
			if ('A' <= unicode && unicode <='Z') unicode += ('a'-'A');
			key = (SDLKey)unicode;
			// the modifiers still contain the true modifier state
		} else {
			// improper unicode translation (like Ctrl-H)
			// or impossible unicode translation:
			// forget the unicode and use keysym instead
			retVal = 0;
		}

		// Keystrokes that are always available
		if (key == SDLK_q && (mod & COMPATIBLE_KMOD_CTRL)) {
			app->quit();
		}

		if (key == SDLK_p && (mod & COMPATIBLE_KMOD_CTRL)) {
			FlagShowFps = !FlagShowFps;
			return 1;
		}

		if (key == SDLK_x) {
			app->commander->execute_command( "flag show_tui_datetime toggle");

			// keep these in sync.  Maybe this should just be one flag.
			if (FlagShowTuiDateTime) app->commander->execute_command( "flag show_tui_short_obj_info on");
			else app->commander->execute_command( "flag show_tui_short_obj_info off");
		}


		// if script is running, only script control keys are accessible
		// to pause/resume/cancel the script
		// (otherwise script could get very confused by user interaction)
		if (app->scripts->is_playing()) {

			// here reusing time control keys to control the script playback
			if (key==SDLK_6) {
				// pause/unpause script
				app->commander->execute_command( "script action pause");

			} else if (key==SDLK_k) {
				app->commander->execute_command( "script action resume");

			} else if (key==SDLK_7 || unicode==0x0003 || (key==SDLK_m && FlagEnableTuiMenu)) { // ctrl-c
				// TODO: should double check with user here...
				app->commander->execute_command( "script action end");
				if (key==SDLK_m)
					setFlagShowTuiMenu(true);
			}
			// TODO n is bad key if ui allowed
			else if (key==SDLK_RIGHTBRACKET) {
				app->commander->execute_command( "audio volume increment");
				return 1;
			}
			// TODO d is bad key if ui allowed
			else if (key==SDLK_LEFTBRACKET) {
				app->commander->execute_command( "audio volume decrement");
				return 1;

			} else if (key==SDLK_j) {
				app->commander->execute_command( "script action slower" );

			} else if (key==SDLK_l) {
				app->commander->execute_command( "script action faster" );

			} else if (!app->scripts->get_allow_ui()) {
				//				cout << "Playing a script.  Press CTRL-C (or 7) to stop." << endl;
			}

			if (!app->scripts->get_allow_ui()) return 1; // only limited user interaction allowed with script

		} else {
			app->time_multiplier = 1;  // if no script in progress always real time

			if (!shiftModifier) {
				// normal time controls here (taken over for script control above if playing a script)
				if (key==SDLK_k) app->commander->execute_command( "timerate rate 1");
				if (key==SDLK_l) app->commander->execute_command( "timerate action increment");
				if (key==SDLK_j) app->commander->execute_command( "timerate action decrement");
				if (key==SDLK_6) app->commander->execute_command( "timerate action pause");
				if (key==SDLK_7) app->commander->execute_command( "timerate rate 0");
				if (key==SDLK_8) app->commander->execute_command( "date load preset");
			}
		}

// shifted commands
		if (shiftModifier) {
			shiftModifier = 0;
			if (key == SDLK_8) {  			 // Reload defaults from config file
				app->init();
				return 1;
			}
			if (key == SDLK_SPACE) {
				// Change home planet to selected planet!
// version 20080430
				string planet = core->getSelectedPlanetEnglishName();
				if (planet!="") app->commander->execute_command( string("set home_planet \"") 
																 + planet + string("\" duration ")
																 + Utility::doubleToString(core->getFlightDuration()) );
				return 1;
			}
			if (key == SDLK_p) {
				app->commander->execute_command("flag planet_orbits toggle");
				return 1;
			}
			if (key == SDLK_PERIOD || key == SDLK_5) {
				app->commander->execute_command("flag tropic_lines toggle");
				return 1;
			}
			if(key == SDLK_x) {
			  app->commander->execute_command( "flag show_tui_short_obj_info toggle");
			  return 1;
			}
			if (key == SDLK_c) {
				app->commander->execute_command("flag constellation_boundaries toggle");
				return 1;
			}
			if (key == SDLK_COMMA || key == SDLK_4) {
				app->commander->execute_command("flag object_trails toggle");
				//				cout << "trails toggle\n";
				return 1;
			}
			if (key == SDLK_z) {
				app->commander->execute_command("flag azimuthal_grid toggle");
				return 1;
			}
			if (key == SDLK_q) {
				app->commander->execute_command("flag galactic_grid toggle");
				return 1;
			}
			if (key == SDLK_k) {
				app->scripts->replay_last_script();
				return 1;
			}
			if (key == SDLK_ESCAPE) {
				app->commander->execute_command("body action clear"); // drop user added bodies
				app->commander->execute_command("nebula action clear"); // drop user added nebulae
				app->commander->execute_command("set milky_way_texture default"); // drop user added milky way
				return 1;
			}
			if (key == SDLK_a) {
				app->commander->execute_command("flag clouds toggle");
				return 1;
			}
            if(key == SDLK_e) {
				app->commander->execute_command( "flag precession_circle toggle");
				return 1;
			}

		}

		// app specific shift key (not standard Shift keyboard key)
		if (key == SDLK_BACKQUOTE) {
			//			cout << "Hit shift button\n";
			shiftModifier = 1;
			ShiftTimeLeft = 3*1000;
		}

		// special key commands
		if (specialModifier) {
			int digit = -1;

			#ifdef DESKTOP

			if(key == SDLK_0) digit = 0;
			else if(key == SDLK_1) digit = 1;
			else if(key == SDLK_2) digit = 2;
			else if(key == SDLK_3) digit = 3;
			else if(key == SDLK_4) digit = 4;
			else if(key == SDLK_5) digit = 5;
			else if(key == SDLK_6) digit = 6;
			else if(key == SDLK_7) digit = 7;
			else if(key == SDLK_8) digit = 8;
			else if(key == SDLK_9) digit = 9;

			#else

			if(key == SDLK_0 || key == SDLK_z) digit = 0;
			else if(key == SDLK_1 || key == SDLK_d) digit = 1;
			else if(key == SDLK_2 || key == SDLK_p) digit = 2;
			else if(key == SDLK_3 || key == SDLK_n) digit = 3;
			else if(key == SDLK_4 || key == SDLK_c) digit = 4;
			else if(key == SDLK_5 || key == SDLK_v) digit = 5;
			else if(key == SDLK_6 || key == SDLK_r) digit = 6;
			else if(key == SDLK_7 || key == SDLK_q) digit = 7;
			else if(key == SDLK_8 || key == SDLK_COMMA) digit = 8;
			else if(key == SDLK_9 || key == SDLK_PERIOD) digit = 9;

			#endif
			
			if(digit != -1) {
				if(SpecialNumber >= 0) {
					SpecialNumber = SpecialNumber*10 + digit;
					
					// cout << "Special number was: " << SpecialNumber << endl;

					app->scripts->play_script_by_number(SpecialNumber);

					specialModifier = 0;
				} else {
					SpecialNumber = digit;
				}
				
				return 1;
			}
		}

		// special script access key
		if (key == SDLK_y) {
			//			cout << "Hit shift button\n";
			specialModifier = 1;
			SpecialTimeLeft = 3*1000;
			SpecialNumber = -1;
		}


#ifdef DESKTOP
		if (key == SDLK_r && (mod & COMPATIBLE_KMOD_CTRL)) {
			if (app->scripts->is_recording()) {
				app->commander->execute_command( "script action cancelrecord");
				show_message(_("Command recording stopped."), 3000);
			} else {
				app->commander->execute_command( "script action record");

				if (app->scripts->is_recording()) {
					show_message(string( _("Recording commands to script file:\n")
					                      + app->scripts->get_record_filename() + "\n\n"
					                      + _("Hit CTRL-R again to stop.\n")), 4000);
				} else {
					show_message(_("Error: Unable to open script file to record commands."), 3000);
				}
			}
			return 1;
		}
#endif

		switch (key) {
		case SDLK_ESCAPE:
			// RFE 1310384, ESC closes dialogs
			// close search mode
			FlagSearch=false;
			search_win->setVisible(FlagSearch);

			// close config dialog
			FlagConfig = false;
			config_win->setVisible(FlagConfig);

			// close help dialog
			FlagHelp = false;
			help_win->setVisible(FlagHelp);

			// close information dialog
			FlagInfos = false;
			licence_win->setVisible(FlagInfos);
			// END RFE 1310384
			break;
		case SDLK_1:
#ifdef DESKTOP
			FlagConfig=!FlagConfig;
			config_win->setVisible(FlagConfig);
#endif
			break;
		case SDLK_4:
		case SDLK_COMMA:
			app->commander->execute_command( "flag ecliptic_line toggle");
			break;
		case SDLK_5:
		case SDLK_PERIOD:
			app->commander->execute_command( "flag equator_line toggle");
			break;
		case SDLK_9:
		{
			const int zhr = core->getMeteorsRate();
			if (zhr <= 10 ) {
				app->commander->execute_command("meteors zhr 80");  // standard Perseids rate
			} else if ( zhr <= 80 ) {
				app->commander->execute_command("meteors zhr 10000"); // exceptional Leonid rate
			} else if ( zhr <= 10000 ) {
				app->commander->execute_command("meteors zhr 144000");  // highest ever recorded ZHR (1966 Leonids)
			} else {
				app->commander->execute_command("meteors zhr 10");  // set to default base rate (10 is normal, 0 would be none)
			}
			break;
		}
		case SDLK_F1:
		case SDLK_h:
			if (mod & COMPATIBLE_KMOD_CTRL) {
//                Fabien wants to toggle
//              core->setFlipHorz(mod & KMOD_SHIFT);
				if (mod & KMOD_SHIFT) {
					core->setFlipHorz(!core->getFlipHorz());
				}
			} else {
				FlagHelp=!FlagHelp;
				help_win->setVisible(FlagHelp);
			}
			break;
		case SDLK_v:
			if (mod & COMPATIBLE_KMOD_CTRL) {
//                Fabien wants to toggle
//              core->setFlipVert(mod & KMOD_SHIFT);
				if (mod & KMOD_SHIFT) {
					core->setFlipVert(!core->getFlipVert());
				}
			} else {
				app->commander->execute_command( "flag constellation_names toggle");
			}
			break;

		case SDLK_f:
			if (mod & COMPATIBLE_KMOD_CTRL) {
				FlagSearch = !FlagSearch;
				search_win->setVisible(FlagSearch);
				return 1;
			} else {
				app->commander->execute_command( "flag fog toggle");
			}
			break;

		case SDLK_r:
			app->commander->execute_command( "flag constellation_art toggle");
			break;
		case SDLK_c:
			app->commander->execute_command( "flag constellation_drawing toggle");
			break;
		case SDLK_b:
			app->commander->execute_command( "flag constellation_boundaries toggle");
			break;
		case SDLK_d:
			if (mod & COMPATIBLE_KMOD_CTRL) {
				// Play demo script
				if(!app->scripts->play_demo_script())
					show_message(string(_("Demo script not found (demo.sts).\nDownload from:")) 
								 + "\nhttp://nightshadesoftware.org", 10000);

			} else app->commander->execute_command( "flag star_names toggle");
			break;
		case SDLK_p:
			app->commander->execute_command("flag planet_names toggle");
			break;
		case SDLK_z:
			app->commander->execute_command( "flag meridian_line toggle");
			break;
		case SDLK_e:
			app->commander->execute_command( "flag equatorial_grid toggle");
			break;
		case SDLK_n:
			app->commander->execute_command( "flag nebula_names toggle");
			break;
		case SDLK_g:
			if (!(mod & COMPATIBLE_KMOD_CTRL))
				app->commander->execute_command( "flag landscape toggle");
			else {
				// Change home planet to selected planet!
// version 20080430
				string planet = core->getSelectedPlanetEnglishName();
				if (planet!="") app->commander->execute_command( string("set home_planet \"") 
																 + planet + string("\" duration ")
																 + Utility::doubleToString(core->getFlightDuration()) );
			}

			break;
		case SDLK_q:
			app->commander->execute_command( "flag cardinal_points toggle");
			break;
		case SDLK_a:
			app->commander->execute_command( "flag atmosphere toggle");
			break;

		case SDLK_t:
			core->setFlagLockSkyPosition(!core->getFlagLockSkyPosition());
			break;
		case SDLK_s:
			if (!(mod & COMPATIBLE_KMOD_CTRL))
				app->commander->execute_command( "flag stars toggle");
			break;
		case SDLK_SPACE:
			app->commander->execute_command("flag track_object on");
			break;
		case SDLK_i:
			FlagInfos=!FlagInfos;
			licence_win->setVisible(FlagInfos);
			break;
		case SDLK_EQUALS:
			//	    cout << "Move 1 day forward.  Type of day = <" << app->DayKeyMode << ">\n";
			if (app->DayKeyMode == "sidereal") app->commander->execute_command( "date sidereal 1");
			else app->commander->execute_command( "date relative 1");
			break;
		case SDLK_MINUS:
			if (app->DayKeyMode == "sidereal") app->commander->execute_command( "date sidereal -1");
			else app->commander->execute_command( "date relative -1");
			break;
		case SDLK_m:
			if (FlagEnableTuiMenu) setFlagShowTuiMenu(true);  // not recorded
			break;
		case SDLK_o:
			app->commander->execute_command( "flag moon_scaled toggle");
			break;
		case SDLK_LEFTBRACKET:
			if (app->DayKeyMode == "sidereal") app->commander->execute_command( "date sidereal -7");
			else app->commander->execute_command( "date relative -7");
			break;
		case SDLK_RIGHTBRACKET:
			if (app->DayKeyMode == "sidereal") app->commander->execute_command( "date sidereal 7");
			else app->commander->execute_command( "date relative 7");
			break;
		case SDLK_SLASH:
			if (mod & COMPATIBLE_KMOD_CTRL) {
				app->commander->execute_command( "zoom auto out");
			} else {
				// here we help script recorders by selecting the right type of zoom option
				// based on current settings of manual or full auto zoom
				if (core->getFlagManualAutoZoom()) app->commander->execute_command( "zoom auto in manual 1");
				else app->commander->execute_command( "zoom auto in");
			}
			break;
		case SDLK_BACKSLASH:
			if (core->getFlagManualAutoZoom()) app->commander->execute_command( "zoom auto out manual 1");
			else app->commander->execute_command( "zoom auto out");
			break;
		case SDLK_u:
			core->toggleMountMode();
			break;
		default:
			break;
		}
	}
	return retVal;
}
#endif // END OF LSS (versus standard keys) 

// Update changing values
void UI::gui_update_widgets(int delta_time)
{
	updateTopBar();

	// handle mouse cursor timeout
	if (MouseCursorTimeout > 0) {
		if (MouseTimeLeft > delta_time) MouseTimeLeft -= delta_time;
		else {
			// hide cursor
			MouseTimeLeft = 0;
			SDL_ShowCursor(0);
		}
	}

	// handle shift key cursor timeout
	if (shiftModifier) {
		if (ShiftTimeLeft > delta_time) ShiftTimeLeft -= delta_time;
		else {
			// done
			//	cout << "Shift timed out\n";
			shiftModifier = 0;
		}
	}

	if (specialModifier) {
		if (SpecialTimeLeft > delta_time) SpecialTimeLeft -= delta_time;
		else {
			specialModifier = 0;
		}
	}


	// update message win
	message_win->update(delta_time);


	if (FlagShowSelectedObjectInfo && core->getFlagHasSelected()) {
		info_select_ctr->setVisible(true);
		updateInfoSelectString();
	} else
		info_select_ctr->setVisible(false);

	bt_flag_ctr->setVisible(FlagMenu);
	bt_time_control_ctr->setVisible(FlagMenu);

	bt_flag_constellation_draw->setState(core->getFlagConstellationLines());
	bt_flag_constellation_name->setState(core->getFlagConstellationNames());
	bt_flag_constellation_art->setState(core->getFlagConstellationArt());
	bt_flag_azimuth_grid->setState(core->getFlagAzimutalGrid());
	bt_flag_equator_grid->setState(core->getFlagEquatorGrid());
	bt_flag_ground->setState(core->getFlagLandscape());
	bt_flag_cardinals->setState(core->getFlagCardinalsPoints());
	bt_flag_atmosphere->setState(core->getFlagAtmosphere());
	bt_flag_nebula_name->setState(core->getFlagNebulaHints());
	bt_flag_help->setState(help_win->getVisible());
	bt_flag_equatorial_mode->setState(core->getMountMode()==Core::MOUNT_EQUATORIAL);
	bt_flag_config->setState(config_win->getVisible());
//	bt_flag_chart->setState(app->getVisionModeChart());
	bt_flag_planet->setState(core->getFlagPlanetsHints());
	bt_flag_search->setState(search_win->getVisible());
	bt_flag_goto->setState(false);
	if (bt_flip_horz) bt_flip_horz->setState(core->getFlipHorz());
	if (bt_flip_vert) bt_flip_vert->setState(core->getFlipVert());

	bt_real_time_speed->setState(fabs(core->getTimeSpeed()-JD_SECOND)<0.000001);
	bt_inc_time_speed->setState((core->getTimeSpeed()-JD_SECOND)>0.0001);
	bt_dec_time_speed->setState((core->getTimeSpeed()-JD_SECOND)<-0.0001);
	// cache last time to prevent to much slow system call
	static double lastJD = 0;
	if (fabs(lastJD-core->getJDay())>JD_SECOND/4) {
		bt_time_now->setState(fabs(core->getJDay()-NShadeDateTime::JulianFromSys())<JD_SECOND);
		lastJD = core->getJDay();
	}
	if (config_win->getVisible()) updateConfigForm();
}

// Update the infos about the selected object in the TextLabel widget
void UI::updateInfoSelectString(void)
{
	if (app->getVisionModeNight()) {
		info_select_txtlbl->setTextColor(Vec3f(1.0,0.2,0.2));
	} else {
		info_select_txtlbl->setTextColor(core->getSelectedObjectInfoColor());
	}
	info_select_txtlbl->setLabel(core->getSelectedObjectInfo());
}

void UI::setTitleObservatoryName(const string& name)
{
	if (name == "")
		top_bar_appName_lbl->setLabel(APP_NAME);
	else {
	  top_bar_appName_lbl->setLabel(string(APP_NAME) + " (" + name + ")");
	}
	top_bar_appName_lbl->setPos(core->getDisplayWidth()/2-top_bar_appName_lbl->getSizex()/2,1);
}

string UI::getTitleWithAltitude(void)
{
	return core->getObservatory()->getHomePlanetNameI18n() +
	       ", " + core->getObservatory()->get_name() +
	       " @ " + Utility::doubleToString(core->getObservatory()->get_altitude()) + "m";
}

void UI::setColorScheme(const string& skinFile, const string& section)
{
	if (!desktop) return;

	InitParser conf;
	conf.load(skinFile);

	s_color GuiBaseColor		= Utility::str_to_vec3f(conf.get_str(section, "gui_base_color", "0.3,0.4,0.7"));
	s_color GuiTextColor		= Utility::str_to_vec3f(conf.get_str(section, "gui_text_color", "0.7,0.8,0.9"));

	desktop->setColorScheme(GuiBaseColor, GuiTextColor);
}


void UI::setFlagShowTuiMenu(const bool flag)
{

	if (flag && !FlagShowTuiMenu) {
		tuiUpdateIndependentWidgets();
	}

	FlagShowTuiMenu = flag;
}
