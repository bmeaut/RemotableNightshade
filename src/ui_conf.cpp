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

#include "core.h"
#include "ui.h"
#include "utility.h"
#include "app.h"


using namespace s_gui;

static string CalculateProjectionSlValue(
    const string &projection_type,
    const string &viewport_distorter_type)
{
	if (viewport_distorter_type == "fisheye_to_spheric_mirror")
		return "spheric_mirror";
	return projection_type;
}

Component* UI::createConfigWindow(void)
{
	config_win = new StdBtWin(_("Configuration"));
	//config_win->setOpaque(opaqueGUI);
	config_win->reshape(core->getDisplayWidth()/2 -300, core->getDisplayHeight()/2 -225, 600, 450);
	config_win->setVisible(FlagConfig);

	config_tab_ctr = new TabContainer();
	config_tab_ctr->setSize(config_win->getSize());

	// The current drawing position
	int x,y;
	x=70;
	y=15;

	// Rendering options
	FilledContainer* tab_render = new FilledContainer();
	tab_render->setSize(config_tab_ctr->getSize());

	const s_texture* starp = new s_texture("bt_render_star.png", TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pstar = new Picture(starp, x-50, y+5, 32, 32);
	tab_render->addComponent(pstar);

	stars_cbx = new LabeledCheckBox(core->getFlagStars(), _("Stars"));
	stars_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(stars_cbx);
	stars_cbx->setPos(x,y);
	y+=15;

	star_names_cbx = new LabeledCheckBox(core->getFlagStarName(), _("Star Labels. Up to mag :"));
	star_names_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(star_names_cbx);
	star_names_cbx->setPos(x,y);

	max_mag_star_name = new FloatIncDec(courierFont, tex_up, tex_down, -1.5, 9, core->getMaxMagStarName(), 0.5);
	max_mag_star_name->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(max_mag_star_name);
	max_mag_star_name->setPos(x + 220,y);

	y+=15;

	star_twinkle_cbx = new LabeledCheckBox(core->getFlagStarTwinkle(), _("Star Twinkle. Amount :"));
	star_twinkle_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(star_twinkle_cbx);
	star_twinkle_cbx->setPos(x,y);

	star_twinkle_amount = new FloatIncDec(courierFont, tex_up, tex_down, 0, 0.6, core->getStarTwinkleAmount(), 0.1);
	star_twinkle_amount->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(star_twinkle_amount);
	star_twinkle_amount->setPos(x + 220,y);

	y+=30;

	const s_texture* constellp = new s_texture("bt_render_constellations.png", TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pconstell = new Picture(constellp, x-50, y+10, 32, 32);
	tab_render->addComponent(pconstell);

	constellation_cbx = new LabeledCheckBox(false, _("Constellation Lines"));
	constellation_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(constellation_cbx);
	constellation_cbx->setPos(x,y);
	y+=15;

	constellation_name_cbx = new LabeledCheckBox(false, _("Constellation Labels"));
	constellation_name_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(constellation_name_cbx);
	constellation_name_cbx->setPos(x,y);
	y+=15;

	constellation_boundaries_cbx = new LabeledCheckBox(false, _("Constellation Boundaries"));
	constellation_boundaries_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(constellation_boundaries_cbx);
	constellation_boundaries_cbx->setPos(x,y);
	y+=15;

	sel_constellation_cbx = new LabeledCheckBox(false, _("Selected Constellation Only"));
	sel_constellation_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(sel_constellation_cbx);
	sel_constellation_cbx->setPos(x,y);

	y+=30;

	const s_texture* nebp = new s_texture("bt_render_nebula.png", TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pneb = new Picture(nebp, x-50, y-13, 32, 32);
	tab_render->addComponent(pneb);

	nebulas_names_cbx = new LabeledCheckBox(false, _("Nebula Labels. Up to mag :"));
	nebulas_names_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(nebulas_names_cbx);
	nebulas_names_cbx->setPos(x,y);

	max_mag_nebula_name = new FloatIncDec(courierFont, tex_up, tex_down, 0, 12,
	                                      core->getNebulaMaxMagHints(), 0.5);
	max_mag_nebula_name->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(max_mag_nebula_name);
	max_mag_nebula_name->setPos(x + 220,y);

	y+=15;

	nebulas_no_texture_cbx = new LabeledCheckBox(false, _("Also display Nebulas without textures"));
	nebulas_no_texture_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(nebulas_no_texture_cbx);
	nebulas_no_texture_cbx->setPos(x,y);

	y+=30;

	const s_texture* planp = new s_texture(string("bt_render_planet.png"), TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pplan = new Picture(planp, x-50, y-7, 32, 32);
	tab_render->addComponent(pplan);

	planets_cbx = new LabeledCheckBox(false, _("Bodies"));
	planets_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables2));
	tab_render->addComponent(planets_cbx);
	planets_cbx->setPos(x,y);

	moon_x4_cbx = new LabeledCheckBox(false, _("Moon Scale"));
	moon_x4_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(moon_x4_cbx);
	moon_x4_cbx->setPos(x + 150,y);

	y+=15;

	planets_hints_cbx = new LabeledCheckBox(false, _("Body Labels"));
	planets_hints_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(planets_hints_cbx);
	planets_hints_cbx->setPos(x,y);


	y+=30;

	const s_texture* gridp = new s_texture("bt_render_grid.png", TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pgrid = new Picture(gridp, x-50, y-4, 32, 32);
	tab_render->addComponent(pgrid);

	equator_grid_cbx = new LabeledCheckBox(false, _("Equatorial Grid"));
	equator_grid_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(equator_grid_cbx);
	equator_grid_cbx->setPos(x,y);
	y+=15;

	azimuth_grid_cbx = new LabeledCheckBox(false, _("Azimuthal Grid"));
	azimuth_grid_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(azimuth_grid_cbx);
	azimuth_grid_cbx->setPos(x,y);
	y-=15;

	equator_cbx = new LabeledCheckBox(false, _("Equator Line"));
	equator_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(equator_cbx);
	equator_cbx->setPos(x + 150,y);
	y+=15;

	ecliptic_cbx = new LabeledCheckBox(false, _("Ecliptic Line"));
	ecliptic_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(ecliptic_cbx);
	ecliptic_cbx->setPos(x + 150,y);

	y+=30;

	const s_texture* groundp = new s_texture("bt_render_ground.png", TEX_LOAD_TYPE_PNG_ALPHA);
	Picture* pground = new Picture(groundp, x-50, y-4, 32, 32);
	tab_render->addComponent(pground);

	ground_cbx = new LabeledCheckBox(false, _("Ground"));
	ground_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(ground_cbx);
	ground_cbx->setPos(x,y);

	cardinal_cbx = new LabeledCheckBox(false, _("Cardinal Points"));
	cardinal_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(cardinal_cbx);
	cardinal_cbx->setPos(x + 150,y);
	y+=15;

	atmosphere_cbx = new LabeledCheckBox(false, _("Atmosphere"));
	atmosphere_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(atmosphere_cbx);
	atmosphere_cbx->setPos(x,y);

	fog_cbx = new LabeledCheckBox(false, _("Fog"));
	fog_cbx->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(fog_cbx);
	fog_cbx->setPos(x + 150,y);

	y+=30;

	meteorlbl = new Label("-");
	meteorlbl->setPos(x,y);
	tab_render->addComponent(meteorlbl);

	y+=20;

	meteor_rate_10 = new LabeledCheckBox(false, "10");
	meteor_rate_10->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(meteor_rate_10);
	meteor_rate_10->setPos(x,y);
	meteor_rate_80 = new LabeledCheckBox(false, "80");
	meteor_rate_80->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(meteor_rate_80);
	meteor_rate_80->setPos(x+40,y);
	meteor_rate_10000 = new LabeledCheckBox(false, "10000");
	meteor_rate_10000->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(meteor_rate_10000);
	meteor_rate_10000->setPos(x+80,y);
	meteor_rate_144000 = new LabeledCheckBox(false, "144000");
	meteor_rate_144000->setOnPressCallback(callback<void>(this, &UI::updateConfigVariables));
	tab_render->addComponent(meteor_rate_144000);
	meteor_rate_144000->setPos(x+144,y);

	LabeledButton* render_save_bt = new LabeledButton(_("Save as default"));
	render_save_bt->setOnPressCallback(callback<void>(this, &UI::saveRenderOptions));
	tab_render->addComponent(render_save_bt);
	render_save_bt->setPos(tab_render->getSizex()/2-render_save_bt->getSizex()/2,tab_render->getSizey()-70);
	y+=20;

	// Date & Time options
	FilledContainer* tab_time = new FilledContainer();
	tab_time->setSize(config_tab_ctr->getSize());

	x=10;
	y=10;

	Label* tclbl = new Label(string("\u2022 ")+_("Current Time :"));
	tclbl->setPos(x,y);
	y+=20;
	tab_time->addComponent(tclbl);

	time_current = new Time_item(courierFont, tex_up, tex_down);
	time_current->setOnChangeTimeCallback(callback<void>(this, &UI::setCurrentTimeFromConfig));
	tab_time->addComponent(time_current);
	time_current->setPos(50,y);
	y+=80;

	Label* tzbl = new Label(string("\u2022 ")+_("Time Zone :"));
	tzbl->setPos(x,y);
	y+=20;
	tab_time->addComponent(tzbl);

	Label* system_tz_lbl = new Label(string("\u2022 ")+_("Using System Default Time Zone"));
	tab_time->addComponent(system_tz_lbl);
	system_tz_lbl->setPos(50 ,y);
	y+=20;
	string tmpl("(ERROR)");
	system_tz_lbl2 = new Label(tmpl);
	tab_time->addComponent(system_tz_lbl2);

	system_tz_lbl2->setPos(70 ,y);
	y+=30;

	Label* time_speed_lbl = new Label(string("\u2022 ")+_("Time speed : "));
	tab_time->addComponent(time_speed_lbl);
	time_speed_lbl->setPos(x ,y);
	y+=20;

	time_speed_lbl2 = new Label();
	tab_time->addComponent(time_speed_lbl2);
	time_speed_lbl2->setPos(50 ,y);
	y+=30;

	TextLabel* ts_lbl = new TextLabel(_("Use key J and L to decrease and increase\n   time speed.\nUse key K to return to real time speed."));
	tab_time->addComponent(ts_lbl);
	ts_lbl->setPos(50 ,y);
	y+=30;

	// Location options
	FilledContainer* tab_location = new FilledContainer();
	tab_location->setSize(config_tab_ctr->getSize());

	x=5;
	y=5;
	lastHomePlanet = core->getObservatory()->getHomePlanet()->getEnglishName();
	earth = new s_texture(
	    *(core->getObservatory()->getHomePlanet()
	      ->getMapTexture()));
	const s_texture *pointertex = new s_texture("pointeur1.png");
	const s_texture *citytex = new s_texture("city.png");
	earth_map = new MapPicture(earth, pointertex, citytex, x,y,tab_location->getSizex()-10, 250);
	earth_map->setOnPressCallback(callback<void>(this, &UI::setObserverPositionFromMap));
	earth_map->setOnNearestCityCallback(callback<void>(this, &UI::setCityFromMap));
	tab_location->addComponent(earth_map);
	y+=earth_map->getSizey();
	earth_map->set_font(9.5f, BaseFontName);
	load_cities(core->getDataDir() + "cities.fab");

	y += 5;
	Label * lblcursor = new Label(_("Cursor : "));
	lblcursor->setPos(20, y+1);
	lblMapLocation = new Label();
	lblMapLocation->setPos(130, y+1);

	Label * lblloc = new Label(_("Selected : "));
	lblloc->setPos(20, y+21);
	lblMapPointer = new Label("ERROR");
	lblMapPointer->setPos(130, y+21);

	Label * lbllong = new Label(_("Longitude : "));
	lbllong->setPos(20, y+41);
	long_incdec	= new FloatIncDec(courierFont, tex_up, tex_down, -180, 180, 0, 1.f/60);
	long_incdec->setSizex(135);
	long_incdec->setFormat(FORMAT_LONGITUDE);
	long_incdec->setOnPressCallback(callback<void>(this, &UI::setObserverPositionFromIncDec));
	long_incdec->setPos(130,y+40);

	Label * lbllat = new Label(_("Latitude : "));
	lbllat->setPos(20, y+61);
	lat_incdec	= new FloatIncDec(courierFont, tex_up, tex_down, -90, 90, 0, 1.f/60);
	lat_incdec->setFormat(FORMAT_LATITUDE);
	lat_incdec->setSizex(135);
	lat_incdec->setOnPressCallback(callback<void>(this, &UI::setObserverPositionFromIncDec));
	lat_incdec->setPos(130,y+60);

	Label * lblalt = new Label(_("Altitude : "));
	lblalt->setPos(20, y+81);
	alt_incdec	= new IntIncDec(courierFont, tex_up, tex_down, 0, 2000, 0, 10);
	alt_incdec->setSizex(135);
	alt_incdec->setOnPressCallback(callback<void>(this, &UI::setObserverPositionFromIncDec));
	alt_incdec->setPos(130,y+80);

	LabeledButton* location_save_bt = new LabeledButton(_("Save location"));
	location_save_bt->setOnPressCallback(callback<void>(this, &UI::saveObserverPosition));
	location_save_bt->setPos(280,y+70);
	//location_save_bt->setSize(120,25);

	tab_location->addComponent(lblcursor);
	tab_location->addComponent(lblloc);
	tab_location->addComponent(lblMapLocation);
	tab_location->addComponent(lblMapPointer);
	tab_location->addComponent(lbllong);
	tab_location->addComponent(lbllat);
	tab_location->addComponent(lblalt);
	tab_location->addComponent(long_incdec);
	tab_location->addComponent(lat_incdec);
	tab_location->addComponent(alt_incdec);
	tab_location->addComponent(location_save_bt);

	// Video Options
	FilledContainer* tab_video = new FilledContainer();
	tab_video->setSize(config_tab_ctr->getSize());

	x=30;
	y=10;
	Label * lblvideo1 = new Label(string("\u2022 ")+_("Projection :"));
	lblvideo1->setPos(x, y);
	tab_video->addComponent(lblvideo1);

	y+=20;

	projection_sl = new StringList();
//	projection_sl->addItem("perspective");
	projection_sl->addItem("fisheye");
//	projection_sl->addItem("stereographic");
	projection_sl->addItem("spheric_mirror");
	projection_sl->adjustSize();
	projection_sl->setValue(CalculateProjectionSlValue(
	                            core->getProjectionType(),
	                            app->getViewPortDistorterType()));
	projection_sl->setOnPressCallback(callback<void>(this, &UI::updateVideoVariables));
	tab_video->addComponent(projection_sl);
	projection_sl->setPos(x+20,y);
	y+=85;

	disk_viewport_cbx = new LabeledCheckBox(false, _("Disk Viewport"));
	disk_viewport_cbx->setOnPressCallback(callback<void>(this, &UI::updateVideoVariables));
	tab_video->addComponent(disk_viewport_cbx);
	disk_viewport_cbx->setPos(x,y);
	y+=35;

	x=220;
	y=10;
	Label * lblvideo2 = new Label(string("\u2022 ")+_("Screen Resolution :"));
	lblvideo2->setPos(x+10, y);
	tab_video->addComponent(lblvideo2);
	y+=24;

	Label * lblvideo3 = new Label(_("Restart program for"));
	Label * lblvideo4 = new Label(_("change to apply."));
	lblvideo3->setPos(x+10, y+145);
	lblvideo4->setPos(x+10, y+160);
	tab_video->addComponent(lblvideo3);
	tab_video->addComponent(lblvideo4);

	screen_size_sl = new ListBox(6);
	screen_size_sl->setPos(x+20,y);
	screen_size_sl->setSizex(200);
	screen_size_sl->addItemList(app->getVideoModeList());
	char vs[1000];
	sprintf(vs, "%dx%d", core->getViewportWidth(), core->getViewportHeight());
	screen_size_sl->setCurrent(vs);
	tab_video->addComponent(screen_size_sl);

	snprintf(vs, 999, "%sconfig.ini", AppSettings::Instance()->getConfigDir().c_str());
	Label * lblvideo5 = new Label(_("For unlisted screen resolution, edit the file :"));
	Label * lblvideo6 = new Label(string(vs));
	lblvideo5->setPos(30, tab_video->getSizey()-125);
	lblvideo6->setPos(30, tab_video->getSizey()-110);
	tab_video->addComponent(lblvideo5);
	tab_video->addComponent(lblvideo6);

	LabeledButton* video_save_bt = new LabeledButton(_("Save as default"));
	video_save_bt->setOnPressCallback(callback<void>(this, &UI::setVideoOption));
	tab_video->addComponent(video_save_bt);
	video_save_bt->setPos(tab_video->getSizex()/2-video_save_bt->getSizex()/2,tab_video->getSizey()-70);

	// Landscapes option
	FilledContainer* tab_landscapes = new FilledContainer();
	tab_landscapes->setSize(config_tab_ctr->getSize());

	x=10;
	y=10;
	Label * lbllandscapes1 = new Label(string("\u2022 ")+_("Choose landscapes:"));
	lbllandscapes1->setPos(x, y);
	tab_landscapes->addComponent(lbllandscapes1);

	x=30;
	y+=24;

	landscape_sl = new ListBox(18);

	landscape_sl->setPos(x,y);
	landscape_sl->setSizex(150);
	landscape_sl->addItemList(Landscape::getLandscapeNames(core->getDataDir() + "landscapes.ini"));
	landscape_sl->setOnChangeCallback(callback<void>(this, &UI::setLandscape));
	landscape_sl->setCurrent(core->getLandscapeName());
	tab_landscapes->addComponent(landscape_sl);

	landscape_authorlb = new Label(_("Author: ") + core->getLandscapeAuthorName());
	landscape_authorlb->setPos(x+landscape_sl->getSizex()+20, y);
	landscape_authorlb->adjustSize();
	tab_landscapes->addComponent(landscape_authorlb);

	landscape_descriptionlb = new TextLabel(core->getLandscapeDescription());
	landscape_descriptionlb->setPos(x+landscape_sl->getSizex()+20, y+25);
	landscape_descriptionlb->adjustSize();
	tab_landscapes->addComponent(landscape_descriptionlb);

	y+=150;

	LabeledButton* landscape_save_bt = new LabeledButton(_("Save as default"));
	landscape_save_bt->setOnPressCallback(callback<void>(this, &UI::saveLandscapeOptions));
	tab_landscapes->addComponent(landscape_save_bt);
	landscape_save_bt->setPos(tab_landscapes->getSizex()/2-landscape_save_bt->getSizex()/2,tab_landscapes->getSizey()-70);


	// Language options
	FilledContainer* tab_language = new FilledContainer();
	tab_language->setSize(config_tab_ctr->getSize());

	x=10;
	y=10;
	Label * lbllanguage = new Label(string("\u2022 ")+_("Program Language: "));
	lbllanguage->setPos(x, y);
	tab_language->addComponent(lbllanguage);

	y+=25;

	language_lb = new ListBox(6);
	language_lb->setPos(x+10,y);
	language_lb->setSizex(200);
	language_lb->addItemList(Translator::getAvailableLanguagesCodes(core->getLocaleDir()));
	language_lb->setOnChangeCallback(callback<void>(this, &UI::setAppLanguage));
	language_lb->setCurrent(app->getAppLanguage());
	tab_language->addComponent(language_lb);

	x=260;
	y=10;

	Label * lbllanguage2 = new Label(string("\u2022 ")+_("Sky Language: "));
	lbllanguage2->setPos(x, y);
	tab_language->addComponent(lbllanguage2);

	y+=25;

	languageSky_lb = new ListBox(6);
	languageSky_lb->setPos(x+10,y);
	languageSky_lb->setSizex(200);
	languageSky_lb->addItemList(Translator::getAvailableLanguagesCodes(core->getLocaleDir()));
	languageSky_lb->setOnChangeCallback(callback<void>(this, &UI::setSkyLanguage));
	languageSky_lb->setCurrent(core->getSkyLanguage());
	tab_language->addComponent(languageSky_lb);

	x=150;
	y+=languageSky_lb->getSizey();
	y+=30;

	Label * lbllanguage3 = new Label(string("\u2022 ")+_("Sky Culture: "));
	lbllanguage3->setPos(x, y);
	tab_language->addComponent(lbllanguage3);

	y+=25;

	skyculture_lb = new ListBox(5);
	skyculture_lb->setSizex(200);
	skyculture_lb->setPos(x,y);
	skyculture_lb->addItemList(core->getSkyCultureListI18());
	skyculture_lb->setOnChangeCallback(callback<void>(this, &UI::setSkyCulture));
	skyculture_lb->setCurrent(core->getSkyCulture());
	tab_language->addComponent(skyculture_lb);

	LabeledButton* language_save_bt = new LabeledButton(_("Save as default"));
	language_save_bt->setOnPressCallback(callback<void>(this, &UI::saveLanguageOptions));
	tab_language->addComponent(language_save_bt);
	language_save_bt->setPos(tab_language->getSizex()/2-language_save_bt->getSizex()/2,tab_language->getSizey()-70);

	// Global window
	config_tab_ctr->setTexture(flipBaseTex);
	config_tab_ctr->addTab(tab_language, _("Language"));
	config_tab_ctr->addTab(tab_time, _("Date & Time"));
	config_tab_ctr->addTab(tab_location, _("Location"));
	config_tab_ctr->addTab(tab_landscapes, _("Landscapes"));
	config_tab_ctr->addTab(tab_video, _("Video"));
	config_tab_ctr->addTab(tab_render, _("Rendering"));
	config_win->addComponent(config_tab_ctr);
	config_win->setOnHideBtCallback(callback<void>(this, &UI::config_win_hideBtCallback));

	return config_win;
}

void UI::dialogCallback(void)
{
	string lastID = dialog_win->getLastID();
	int lastButton = dialog_win->getLastButton();
	string lastInput = dialog_win->getLastInput();
	int lastType = dialog_win->getLastType();

	if (lastID == "observatory name") {
		if (lastButton != BT_OK || lastInput == "")
			lastInput = UNKNOWN_OBSERVATORY;
		doSaveObserverPosition(lastInput);
		setCityFromMap();
	} else if (lastID != "") {
		string msg = lastID;
		msg += " returned btn: ";

		if (lastButton == BT_OK) msg += "BT_OK";
		else if (lastButton == BT_YES) msg += "BT_YES";
		else if (lastButton == BT_NO) msg += "BT_NO";
		else if (lastButton == BT_CANCEL) msg += "BT_CANCE";

		if (lastType == STDDLGWIN_MSG) dialog_win->MessageBox(string(APP_NAME),msg, BT_OK);
		else if (lastType == STDDLGWIN_INPUT) dialog_win->MessageBox(string(APP_NAME),string(msg) + " inp: " + lastInput, BT_OK);
	}
}

void UI::setSkyLanguage(void)
{
	core->setSkyLanguage(languageSky_lb->getCurrent());
}

void UI::setAppLanguage(void)
{
	app->setAppLanguage(language_lb->getCurrent());
}

void UI::setSkyCulture(void)
{
	core->setSkyCulture(skyculture_lb->getCurrent());
}

void UI::load_cities(const string & fileName)
{
	float time;

	char linetmp[200];
	char cname[50],cstate[50],ccountry[50],clat[20],clon[20],ctime[20];
	int showatzoom;
	int alt;
	char tmpstr[2000];
	int total = 0;

	cout << "Loading Cities data...";
	FILE *fic = fopen(fileName.c_str(), "r");
	if (!fic) {
		cerr << "Can't open " << fileName << endl;
		return; // no art, but still loaded constellation data
	}

	// determine total number to be loaded for percent complete display
	while (fgets(tmpstr, 2000, fic)) {
		++total;
	}
	rewind(fic);

	int line = 0;
	while (!feof(fic)) {
		fgets(linetmp, 100, fic);
		if (linetmp[0] != '#') {
			if (sscanf(linetmp, "%s %s %s %s %s %d %s %d\n", cname, cstate, ccountry, clat,
			           clon, &alt, ctime, &showatzoom) != 8) {
				if (feof(fic)) {
					// Empty city file
					fclose(fic);
					return;
				}
				cerr << "ERROR while loading city data in line " << line << endl;
				assert(0);
			}

			string name, state, country;
			name = cname;
			for (string::size_type i=0; i<name.length(); ++i) {
				if (name[i]=='_') name[i]=' ';
			}
			state = cstate;
			for (string::size_type i=0; i<state.length(); ++i) {
				if (state[i]=='_') state[i]=' ';
			}
			country = ccountry;
			for (string::size_type i=0; i<country.length(); ++i) {
				if (country[i]=='_') country[i]=' ';
			}
			if (ctime[0] == 'x')
				time = 0;
			else
				sscanf(ctime,"%f",&time);
			earth_map->addCity(name,
			                   state,
			                   country,
			                   get_dec_angle(clon),
			                   get_dec_angle(clat), time, showatzoom, alt);
		}
		line++;
	}
	fclose(fic);
	cout << "(" << line << " cities loaded)" << endl;
}

// Create Search window widgets
Component* UI::createSearchWindow(void)
{
	int x=10;
	int y=10;

	// TODO: Escape should close search dialog

	// Bring up dialog
	search_win = new StdBtWin(_("Object Search"));
	search_win->reshape(core->getDisplayWidth()/2 -200, 2*core->getDisplayHeight()/3 -50, 400, 100);
	search_win->setVisible(FlagSearch);

	lblSearchMessage = new Label("");
	lblSearchMessage->setPos(15, search_win->getSizey()-25);

	Label * lblstars1 = new Label(_("Search for (eg. Saturn, Polaris, HP6218, Orion, M31):"));
	lblstars1->setPos(x, y);
	search_win->addComponent(lblstars1);

	y+=30;

	const s_texture* searchp = new s_texture("bt_search.png");
	Picture* psearch = new Picture(searchp, x, y+1, 24, 24);
	search_win->addComponent(psearch);

	star_edit = new EditBox();
	star_edit->setOnReturnKeyCallback(callback<void>(this, &UI::gotoSearchedObject));
	star_edit->setOnKeyCallback(callback<void>(this, &UI::autoCompleteSearchedObject));
	search_win->addComponent(star_edit);
	star_edit->setPos(x+30,y);
	star_edit->setSize(230,25);

	LabeledButton* gobutton = new LabeledButton(_("GO"));
	gobutton->setPos(300, y-2);
	gobutton->setJustification(JUSTIFY_CENTER);
	gobutton->setOnPressCallback(callback<void>(this, &UI::gotoSearchedObject));

	search_win->addComponent(gobutton);
	search_win->addComponent(lblSearchMessage);
	search_win->setOnHideBtCallback(callback<void>(this, &UI::search_win_hideBtCallback));

	return search_win;
}


void UI::autoCompleteSearchedObject(void)
{
	string objectName = star_edit->getText();
	star_edit->setAutoCompleteOptions(core->listMatchingObjectsI18n(objectName, 5));
	lblSearchMessage->setLabel(star_edit->getAutoCompleteOptions());
}

void UI::gotoSearchedObject(void)
{
	if (core->findAndSelectI18n(star_edit->getText())) {
		star_edit->clearText();
		core->gotoSelectedObject();
		core->setFlagTracking(true);
		lblSearchMessage->setLabel("");
	} else {
		lblSearchMessage->setLabel(star_edit->getText()+" is unknown!");
	}
}

void UI::updateConfigVariables(void)
{
	app->commander->execute_command("flag stars ", stars_cbx->getState());
	app->commander->execute_command("flag star_names ", star_names_cbx->getState());
	app->commander->execute_command("set max_mag_star_name ", max_mag_star_name->getValue());
	app->commander->execute_command("flag star_twinkle ", star_twinkle_cbx->getState());
	app->commander->execute_command("set star_twinkle_amount ", star_twinkle_amount->getValue());
	app->commander->execute_command("flag constellation_drawing ", constellation_cbx->getState());
	app->commander->execute_command("flag constellation_names ", constellation_name_cbx->getState());
	app->commander->execute_command("flag constellation_boundaries ", constellation_boundaries_cbx->getState());
	app->commander->execute_command("flag constellation_pick ", sel_constellation_cbx->getState());
	app->commander->execute_command("flag nebula_names ", nebulas_names_cbx->getState());
	app->commander->execute_command("set max_mag_nebula_name ", max_mag_nebula_name->getValue());
	core->setFlagNebulaDisplayNoTexture(nebulas_no_texture_cbx->getState());
	app->commander->execute_command("flag planet_names ", planets_hints_cbx->getState());
	app->commander->execute_command("flag moon_scaled ", moon_x4_cbx->getState());
	app->commander->execute_command("flag equatorial_grid ", equator_grid_cbx->getState());
	app->commander->execute_command("flag azimuthal_grid ", azimuth_grid_cbx->getState());
	app->commander->execute_command("flag equator_line ", equator_cbx->getState());
	app->commander->execute_command("flag ecliptic_line ", ecliptic_cbx->getState());
	app->commander->execute_command("flag landscape ", ground_cbx->getState());
	app->commander->execute_command("flag cardinal_points ", cardinal_cbx->getState());
	app->commander->execute_command("flag atmosphere ", atmosphere_cbx->getState());
	app->commander->execute_command("flag fog ", fog_cbx->getState());
	if (meteor_rate_10->getState() && core->getMeteorsRate()!=10)
		app->commander->execute_command("meteors zhr 10");
	else if (meteor_rate_80->getState() && core->getMeteorsRate()!=80)
		app->commander->execute_command("meteors zhr 80");
	else if (meteor_rate_10000->getState() && core->getMeteorsRate()!=10000)
		app->commander->execute_command("meteors zhr 10000");
	else if (meteor_rate_144000->getState() && core->getMeteorsRate()!=144000)
		app->commander->execute_command("meteors zhr 144000");
}

void UI::updateConfigVariables2(void)
{
	app->commander->execute_command("flag planets ", planets_cbx->getState());
}

void UI::setCurrentTimeFromConfig(void)
{
	//	core->navigation->set_JDay(time_current->getJDay() - core->observatory->get_GMT_shift()*JD_HOUR);
	app->commander->execute_command(string("date local " + time_current->getDateString()));
}

void UI::setObserverPositionFromMap(void)
{
	std::ostringstream oss;
	oss << "moveto lat " << earth_map->getPointerLatitude() << " lon " << earth_map->getPointerLongitude()
	<< " alt " << earth_map->getPointerAltitude();
	app->commander->execute_command(oss.str());
}

void UI::setCityFromMap(void)
{
	waitOnLocation = false;
	lblMapLocation->setLabel(earth_map->getCursorString());
	lblMapPointer->setLabel(earth_map->getPositionString());
}

void UI::setObserverPositionFromIncDec(void)
{
	std::ostringstream oss;
	oss.setf(ios::fixed);
	oss.precision(10);
	oss << "moveto lat " << lat_incdec->getValue() << " lon " << long_incdec->getValue()
	<< " alt " << alt_incdec->getValue();
	app->commander->execute_command(oss.str());
}

void UI::doSaveObserverPosition(const string& name)
{
	string location = name;
	for (string::size_type i=0; i<location.length(); ++i) {
		if (location[i]==' ') location[i]='_';
	}

	std::ostringstream oss;
	oss << "moveto lat " << lat_incdec->getValue() << " lon " << long_incdec->getValue()
	<< " name " << location;
	app->commander->execute_command(oss.str());

	core->getObservatory()->save(AppSettings::Instance()->getConfigFile(), "init_location");
	app->ui->setTitleObservatoryName(app->ui->getTitleWithAltitude());
}

void UI::saveObserverPosition(void)
{
	string location = earth_map->getPositionString();

	if (location == UNKNOWN_OBSERVATORY)
		dialog_win->InputBox(string(APP_NAME),_("Enter observatory name"), "observatory name");
	else
		doSaveObserverPosition(location);

}

void UI::saveLandscapeOptions(void)
{
	cout << "Saving landscape name in file " << AppSettings::Instance()->getConfigFile() << endl;
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());
	conf.set_str("init_location:landscape_name", core->getObservatory()->get_landscape_name());
	conf.save(AppSettings::Instance()->getConfigFile());
}

void UI::saveLanguageOptions(void)
{
	cout << "Saving language in file " << AppSettings::Instance()->getConfigFile() << endl;
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());
	conf.set_str("localization:sky_locale", core->getSkyLanguage());
	conf.set_str("localization:app_locale", app->getAppLanguage());
	conf.set_str("localization:sky_culture", core->getSkyCultureDir());
	conf.save(AppSettings::Instance()->getConfigFile());
}

void UI::saveRenderOptions(void)
{
	cout << "Saving rendering options in file " << AppSettings::Instance()->getConfigFile() << endl;

	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());

	conf.set_boolean("astro:flag_stars", core->getFlagStars());
	conf.set_boolean("astro:flag_star_name", core->getFlagStarName());
	conf.set_double("stars:max_mag_star_name", core->getMaxMagStarName());
	conf.set_boolean("stars:flag_star_twinkle", core->getFlagStarTwinkle());
	conf.set_double("stars:star_twinkle_amount", core->getStarTwinkleAmount());
	conf.set_boolean("viewing:flag_constellation_drawing", core->getFlagConstellationLines());
	conf.set_boolean("viewing:flag_constellation_name", core->getFlagConstellationNames());
	conf.set_boolean("viewing:flag_constellation_boundaries", core->getFlagConstellationBoundaries());
	conf.set_boolean("viewing:flag_constellation_pick", core->getFlagConstellationIsolateSelected());
	conf.set_boolean("astro:flag_nebula", core->getFlagNebula());
	conf.set_boolean("astro:flag_nebula_name", core->getFlagNebulaHints());
	conf.set_double("astro:max_mag_nebula_name", core->getNebulaMaxMagHints());
	conf.set_boolean("astro:flag_nebula_display_no_texture", core->getFlagNebulaDisplayNoTexture());
	conf.set_boolean("astro:flag_planets", core->getFlagPlanets());
	conf.set_boolean("astro:flag_planets_hints", core->getFlagPlanetsHints());
	conf.set_double("viewing:moon_scale", core->getMoonScale());
	conf.set_boolean("viewing:flag_moon_scaled", core->getFlagMoonScaled());
	conf.set_boolean("viewing:flag_chart", app->getVisionModeChart());
	conf.set_boolean("viewing:flag_night", app->getVisionModeNight());
	conf.set_boolean("viewing:flag_equatorial_grid", core->getFlagEquatorGrid());
	conf.set_boolean("viewing:flag_azimutal_grid", core->getFlagAzimutalGrid());
	conf.set_boolean("viewing:flag_equator_line", core->getFlagEquatorLine());
	conf.set_boolean("viewing:flag_ecliptic_line", core->getFlagEclipticLine());
	conf.set_boolean("landscape:flag_landscape", core->getFlagLandscape());
	conf.set_boolean("viewing:flag_cardinal_points", core->getFlagCardinalsPoints());
	conf.set_boolean("landscape:flag_atmosphere", core->getFlagAtmosphere());
	conf.set_boolean("landscape:flag_fog", core->getFlagFog());
	conf.set_int("astro:meteor_rate", core->getMeteorsRate());
	conf.save(AppSettings::Instance()->getConfigFile());
}

void UI::setVideoOption(void)
{
	string s = screen_size_sl->getCurrent();
	int i = s.find("x");
	int w = atoi(s.substr(0,i).c_str());
	int h = atoi(s.substr(i+1,s.size()).c_str());

	cout << "Saving video size " << w << "x" << h << " in file " << AppSettings::Instance()->getConfigFile() << endl;

	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());

	conf.set_str("projection:type", core->getProjectionType());
	conf.set_str("video:distorter", app->getViewPortDistorterType());

	if (core->getViewportMaskDisk()) conf.set_str("projection:viewport", "disk");
	else conf.set_str("projection:viewport", "maximized");

	conf.set_int("video:screen_w", w);
	conf.set_int("video:screen_h", h);
	conf.save(AppSettings::Instance()->getConfigFile());
}

void UI::setLandscape(void)
{
	core->setLandscape(Landscape::nameToKey(core->getDataDir() + "landscapes.ini", landscape_sl->getCurrent()));
	landscape_authorlb->setLabel(_("Author: ") + core->getLandscapeAuthorName());
	landscape_descriptionlb->setLabel(_("Info: ") + core->getLandscapeDescription());
}

void UI::updateVideoVariables(void)
{
	if (projection_sl->getValue() == "spheric_mirror") {
		core->setProjectionType("fisheye");
		app->setViewPortDistorterType("fisheye_to_spheric_mirror");
	} else {
		core->setProjectionType(projection_sl->getValue());
		app->setViewPortDistorterType("none");
	}

	if (disk_viewport_cbx->getState() && !core->getViewportMaskDisk()) {
		core->setViewportMaskDisk();
	}
	if (!disk_viewport_cbx->getState() && core->getViewportMaskDisk()) {
		core->setViewportMaskNone();
	}
}

void UI::updateConfigForm(void)
{
	stars_cbx->setState(core->getFlagStars());
	star_names_cbx->setState(core->getFlagStarName());
	max_mag_star_name->setValue(core->getMaxMagStarName());
	star_twinkle_cbx->setState(core->getFlagStarTwinkle());
	star_twinkle_amount->setValue(core->getStarTwinkleAmount());
	constellation_cbx->setState(core->getFlagConstellationLines());
	constellation_name_cbx->setState(core->getFlagConstellationNames());
	constellation_boundaries_cbx->setState(core->getFlagConstellationBoundaries());
	sel_constellation_cbx->setState(core->getFlagConstellationIsolateSelected());
	nebulas_names_cbx->setState(core->getFlagNebulaHints());
	max_mag_nebula_name->setValue(core->getNebulaMaxMagHints());
	nebulas_no_texture_cbx->setState(core->getFlagNebulaDisplayNoTexture());
	planets_cbx->setState(core->getFlagPlanets());
	planets_hints_cbx->setState(core->getFlagPlanetsHints());
	moon_x4_cbx->setState(core->getFlagMoonScaled());
	equator_grid_cbx->setState(core->getFlagEquatorGrid());
	azimuth_grid_cbx->setState(core->getFlagAzimutalGrid());
	equator_cbx->setState(core->getFlagEquatorLine());
	ecliptic_cbx->setState(core->getFlagEclipticLine());
	ground_cbx->setState(core->getFlagLandscape());
	cardinal_cbx->setState(core->getFlagCardinalsPoints());
	atmosphere_cbx->setState(core->getFlagAtmosphere());
	fog_cbx->setState(core->getFlagFog());

	string meteorRate;
	if (core->getMeteorsRate()==10) {
		meteorRate = _(": Normal rate");
		meteor_rate_10->setState(true);
	} else
		meteor_rate_10->setState(false);
	if (core->getMeteorsRate()==80) {
		meteorRate = _(": Standard Perseids rate");
		meteor_rate_80->setState(true);
	} else
		meteor_rate_80->setState(false);
	if (core->getMeteorsRate()==10000) {
		meteorRate = _(": Exceptional Leonid rate");
		meteor_rate_10000->setState(true);
	} else
		meteor_rate_10000->setState(false);
	if (core->getMeteorsRate()==144000) {
		meteorRate = _(": Highest rate ever (1966 Leonids)");
		meteor_rate_144000->setState(true);
	} else
		meteor_rate_144000->setState(false);
	meteorlbl->setLabel(_("Meteor Rate per minute")+meteorRate);

	// Update map texture if new home planet
	if( lastHomePlanet != core->getObservatory()->getHomePlanet()->getEnglishName()) {
		delete earth;
		earth = new s_texture( *(core->getObservatory()->getHomePlanet()->getMapTexture()));
		earth_map->setTexture(earth);
		lastHomePlanet = core->getObservatory()->getHomePlanet()->getEnglishName();
	}
	earth_map->setPointerLongitude(core->getObservatory()->get_longitude());
	earth_map->setPointerLatitude(core->getObservatory()->get_latitude());
	long_incdec->setValue(core->getObservatory()->get_longitude());
	lat_incdec->setValue(core->getObservatory()->get_latitude());
	alt_incdec->setValue(core->getObservatory()->get_altitude());

	if( lastHomePlanet == "Earth" ) {
		earth_map->setShowCities( true );
		lblMapLocation->setLabel(earth_map->getCursorString());
	} else {
		earth_map->setShowCities( false );
		lblMapLocation->setLabel("");
	}

	if (!waitOnLocation)
		lblMapPointer->setLabel(earth_map->getPositionString());
	else {
		earth_map->findPosition(core->getObservatory()->get_longitude(),core->getObservatory()->get_latitude());
		lblMapPointer->setLabel(earth_map->getPositionString());
		waitOnLocation = false;
	}
	

	time_current->setJDay(core->getJDay() + app->get_GMT_shift(core->getJDay())*JD_HOUR);
	system_tz_lbl2->setLabel(string("(") +
			NShadeDateTime::TimeZoneNameFromSystem(core->getJDay()) + ")");

	time_speed_lbl2->setLabel(string("\u2022 ")+_("Current Time Speed is x") + Utility::doubleToString(core->getTimeSpeed()/JD_SECOND));

	projection_sl->setValue(CalculateProjectionSlValue(
	                            core->getProjectionType(),
	                            app->getViewPortDistorterType()));
	disk_viewport_cbx->setState(core->getViewportMaskDisk());
}

void UI::config_win_hideBtCallback(void)
{
	FlagConfig = false;
	config_win->setVisible(false);
	// for MapPicture - when the dialog appears, this tells the system
	// not to show the city until MapPicture has located the name
	// from the lat and long.
	waitOnLocation = true;
	bt_flag_config->setState(0);
}

void UI::search_win_hideBtCallback(void)
{
	FlagSearch = false;
	search_win->setVisible(false);
	bt_flag_search->setState(0);
}
