/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2005-2006 Robert Spearman
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

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#include "app_command_interface.h"
#include "core.h"
#include "image.h"
#include "stellastro.h"
#include "nightshade.h"
#include "named_sockets.h"

using namespace std;


AppCommandInterface::AppCommandInterface(Core * core, App * app)
{
	stcore = core;
	stapp = app;
	audio = NULL;
	audioDisabled = 0; // on by default
	ExtViewer = NULL;
}

AppCommandInterface::~AppCommandInterface()
{
}

void AppCommandInterface::disableAudio()
{
	audioDisabled = 1;
	if (audio && audio->from_script()) audio->disable();
}


void AppCommandInterface::enableAudio()
{
	audioDisabled = 0;
	if (audio && audio->from_script()) audio->enable();
}


int AppCommandInterface::execute_command(string commandline )
{
	unsigned long int delay;
	return execute_command(commandline, delay, 1);  // Assumed to be trusted!
	// delay is ignored, as not needed by the ui callers
}

// for easy calling of simple commands with a double as last argument value
int AppCommandInterface::execute_command(string command, double arg)
{
	unsigned long int delay;

	std::ostringstream commandline;
	commandline << command << arg;

	return execute_command(commandline.str(), delay, 1);  // Assumed to be trusted!
	// delay is ignored, as not needed by the ui callers
}

// for easy calling of simple commands with an int as last argument value
int AppCommandInterface::execute_command(string command, int arg)
{
	unsigned long int delay;

	std::ostringstream commandline;
	commandline << command << arg;

	return execute_command(commandline.str(), delay, 1);  // Assumed to be trusted!
	// delay is ignored, as not needed by the ui callers
}


// called by script executors
// certain key settings can't be modified by scripts unless
// they are "trusted" - TODO details TBD when needed
int AppCommandInterface::execute_command(string commandline, unsigned long int &wait, bool trusted)
{
	string command;
	stringHash_t args;
	int status = 0;  // true if command was understood
	int recordable = 1;  // true if command should be recorded (if recording)

	wait = 0;  // default, no wait between commands

	status = parse_command(commandline, command, args);

	// If command is empty then don't bother checking all these cases
	if( command.length() < 1 )
		return 0;

	// application specific logic to run each command
	if (command == "flag") {

		// could loop if want to allow that syntax
		if (args.begin() != args.end()) {

			bool val;
			status = set_flag( args.begin()->first, args.begin()->second, val, trusted);

			// rewrite command for recording so that actual state is known (rather than "toggle")
			if (args.begin()->second == "toggle") {
				std::ostringstream oss;
				oss << command << " " << args.begin()->first << " " << val;
				commandline = oss.str();
			}
		} else status = 0;

	}  else if (command == "wait") {

		if ( args["until"]!="") {
			float fseconds = str_time_to_seconds(args["until"]);
			if (fseconds > 0) {
				fseconds -= stapp->scripts->get_script_elapsed_seconds();
				if(fseconds > 0 ) wait = (unsigned long int)(fseconds*1000);
				else wait = 0;
			}
		} else if ( args["duration"]!="") {
			float fdelay = str_to_double(args["duration"]);
			if (fdelay > 0) wait = (int)(fdelay*1000);
		}

		// Allow timer to be reset
		if ( args["action"]=="reset_timer") stapp->scripts->reset_timer();

	} else if (command == "set") {
		// set core variables

		// TODO: some bounds/error checking here

		if (args["atmosphere_fade_duration"]!="") stcore->setAtmosphereFadeDuration(str_to_double(args["atmosphere_fade_duration"]));
		else if (args["auto_move_duration"]!="") stcore->setAutomoveDuration( str_to_double(args["auto_move_duration"]));
		else if (args["constellation_art_fade_duration"]!="") stcore->setConstellationArtFadeDuration(str_to_double(args["constellation_art_fade_duration"]));
		else if (args["constellation_art_intensity"]!="") stcore->setConstellationArtIntensity(str_to_double(args["constellation_art_intensity"]));
		else if (args["light_pollution_limiting_magnitude"]!="") stcore->setLightPollutionLimitingMagnitude(str_to_double(args["light_pollution_limiting_magnitude"]));
		else if (args["flight_duration"]!="") stcore->setFlightDuration( str_to_double(args["flight_duration"]));
		else if (args["heading"]!="") {

			float fdelay = str_to_double(args["duration"]);
			if (fdelay <= 0) fdelay = 0;

			stcore->getNavigation()->change_heading(str_to_double(args["heading"]), (int)(fdelay*1000));

		} else if (args["home_planet"]!="") {
			float duration = 0;
			if( args["duration"] == "default" )
				duration = stcore->getFlightDuration();
			else
				duration = str_to_double(args["duration"], 0);
			stcore->setHomePlanet(args["home_planet"], duration);
		} else if (args["landscape_name"]!="") stcore->setLandscape(args["landscape_name"]);
		else if (args["line_width"]!="") stcore->setLineWidth(str_to_double(args["line_width"]));
		else if (args["max_mag_nebula_name"]!="") stcore->setNebulaMaxMagHints(str_to_double(args["max_mag_nebula_name"]));
		else if (args["max_mag_star_name"]!="") stcore->setMaxMagStarName(str_to_double(args["max_mag_star_name"]));
		else if (args["moon_scale"]!="") {
			stcore->setMoonScale(str_to_double(args["moon_scale"]));
		} else if (args["milky_way_texture"]!="") {
			if(args["milky_way_texture"]=="default") stcore->milkyswap(args["milky_way_texture"]);
			else stcore->milkyswap(stapp->scripts->get_script_path() + args["milky_way_texture"]);
		} else if (args["sky_culture"]!="") stcore->setSkyCultureDir(args["sky_culture"]);
		else if (args["sky_locale"]!="") stcore->setSkyLanguage(args["sky_locale"]);
		else if (args["ui_locale"]!="") stapp->setAppLanguage(args["ui_locale"]);
		else if (args["star_mag_scale"]!="") stcore->setStarMagScale(str_to_double(args["star_mag_scale"]));
		else if (args["star_size_limit"]!="") stcore->setStarSizeLimit(str_to_double(args["star_size_limit"]));
		else if (args["planet_size_limit"]!="") stcore->setPlanetsSizeLimit(str_to_double(args["planet_size_limit"]));
		else if (args["star_scale"]!="") {
			float scale = str_to_double(args["star_scale"]);
			stcore->setStarScale(scale);
			stcore->setPlanetsScale(scale);
		} else if (args["nebula_scale"]!="") {
			float scale = str_to_double(args["nebula_scale"]);
			stcore->setNebulaCircleScale(scale);
		} else if (args["star_twinkle_amount"]!="") stcore->setStarTwinkleAmount(str_to_double(args["star_twinkle_amount"]));
		else if (args["star_limiting_mag"]!="") stcore->setStarLimitingMag(str_to_double(args["star_limiting_mag"]));
		else if (args["time_zone"]!="") stapp->setCustomTimezone(args["time_zone"]);
		else if (args["milky_way_intensity"]!="") {
			stcore->setMilkyWayIntensity(str_to_double(args["milky_way_intensity"]));
			// safety feature to be able to turn back on
			if (stcore->getMilkyWayIntensity()) stcore->setFlagMilkyWay(true);
		} else if (args["zoom_offset"]!="") {
			stcore->setViewOffset(str_to_double(args["zoom_offset"]));

		} else status = 0;


		if (trusted) {

			// trusted commands disabled due to code reorg

			//    else if(args["base_font_size"]!="") stcore->BaseFontSize = str_to_double(args["base_font_size"]);
			//	else if(args["bbp_mode"]!="") stcore->BbpMode = str_to_double(args["bbp_mode"]);
			//    else if(args["date_display_format"]!="") stcore->DateDisplayFormat = args["date_display_format"];
			//	else if(args["fullscreen"]!="") stcore->Fullscreen = args["fullscreen"];
			//	else if(args["horizontal_offset"]!="") stcore->HorizontalOffset = str_to_double(args["horizontal_offset"]);
			//	else if(args["init_fov"]!="") stcore->InitFov = str_to_double(args["init_fov"]);
			//	else if(args["preset_sky_time"]!="") stapp->PresetSkyTime = str_to_double(args["preset_sky_time"]);
			//	else if(args["screen_h"]!="") stcore->ScreenH = str_to_double(args["screen_h"]);
			//	else if(args["screen_w"]!="") stcore->ScreenW = str_to_double(args["screen_w"]);
			//    else if(args["startup_time_mode"]!="") stapp->StartupTimeMode = args["startup_time_mode"];
			// else if(args["time_display_format"]!="") stcore->TimeDisplayFormat = args["time_display_format"];
			//else if(args["type"]!="") stcore->Type = args["type"];
			//else if(args["version"]!="") stcore->Version = str_to_double(args["version"]);
			//      else if(args["vertical_offset"]!="") stcore->VerticalOffset = str_to_double(args["vertical_offset"]);
			//else if(args["viewing_mode"]!="") stcore->ViewingMode = args["viewing_mode"];
			//else if(args["viewport"]!="") stcore->Viewport = args["viewport"];

		}

	} else if (command == "select") {

		// default is to deselect current object
		stcore->unSelect();


		string select_type, identifier;

		if (args["hp"]!="") {
			select_type = "hp";
			identifier = args["hp"];
		} else if (args["star"]!="") {
			select_type = "star";
			identifier = args["star"];
		} else if (args["planet"]!="") {
			select_type = "planet";
			identifier = args["planet"];

			if (args["planet"] == "home_planet")
				identifier = stcore->getNavigation()->getHomePlanet()->getEnglishName();
		} else if (args["nebula"]!="") {
			select_type = "nebula";
			identifier = args["nebula"];
		} else if (args["constellation"]!="") {
			select_type = "constellation";
			identifier = args["constellation"];
		} else if (args["constellation_star"]!="") {
			select_type = "constellation_star";
			identifier = args["constellation_star"];
		} else {
			select_type = "";
		}

		if (select_type != "" ) stcore->selectObject(select_type, identifier);

		// determine if selected object pointer should be displayed
		if (args["pointer"]=="off" || args["pointer"]=="0") stcore->setFlagSelectedObjectPointer(false);
		else stcore->setFlagSelectedObjectPointer(true);


	} else if (command == "deselect") {
		if (args["constellation"] != "") {
			stcore->unsetSelectedConstellation(args["constellation"]);
		} else {
			stcore->deselect();
		}
	} else if (command == "look") { // change direction of view
		//	  double duration = str_to_pos_double(args["duration"]);

		if (args["delta_az"]!="" || args["delta_alt"]!="") {
			// immediately change viewing direction
			stcore->panView(str_to_double(args["delta_az"]),
			                str_to_double(args["delta_alt"]));
		}	else status = 0;

		// TODO absolute settings (see RFE 1311031)




	} else if (command == "zoom") {
		double duration = str_to_pos_double(args["duration"]);

		if (args["auto"]!="") {
			// auto zoom using specified or default duration
			if (args["duration"]=="") duration = stcore->getAutoMoveDuration();

			if (args["auto"]=="out") {
				if (args["manual"]=="1") stcore->autoZoomOut(duration, 0, 1);
				else stcore->autoZoomOut(duration, 0, 0);
			} else if (args["auto"]=="initial") stcore->autoZoomOut(duration, 1, 0);
			else if (args["manual"]=="1") {
				stcore->autoZoomIn(duration, 1);  // have to explicity allow possible manual zoom
			} else stcore->autoZoomIn(duration, 0);

		} else if (args["fov"]!="") {
			// zoom to specific field of view
			stcore->zoomTo( str_to_double(args["fov"]), str_to_double(args["duration"]));

		} else if (args["delta_fov"]!="") stcore->setFov(stcore->getFov() + str_to_double(args["delta_fov"]));
		// should we record absolute fov instead of delta? isn't usually smooth playback
		else status = 0;

	} else if (command == "timerate") {  // NOTE: accuracy issue related to frame rate

		if (args["rate"]!="") {
			stcore->setTimeSpeed(str_to_double(args["rate"])*JD_SECOND);
			stapp->temp_time_velocity = stcore->getTimeSpeed();
			stapp->FlagTimePause = 0;

		} else if (args["action"]=="pause") {
			// TODO why is this in stelapp?  should be in stelcore - Rob
			stapp->FlagTimePause = !stapp->FlagTimePause;
			if (stapp->FlagTimePause) {
				// TODO pause should be all handled in core methods
				stapp->temp_time_velocity = stcore->getTimeSpeed();
				stcore->setTimeSpeed(0);
			} else {
				stcore->setTimeSpeed(stapp->temp_time_velocity);
			}

		} else if (args["action"]=="resume") {
			stapp->FlagTimePause = 0;
			stcore->setTimeSpeed(stapp->temp_time_velocity);

		} else if (args["action"]=="increment") {
			// speed up time rate
			stapp->FlagTimePause = 0;
			double s = stcore->getTimeSpeed();
			double sstep = 10.;

			if( !args["step"].empty() )
				sstep = str_to_double(args["step"]);

			if (s>=JD_SECOND) s*=sstep;
			else if (s<-JD_SECOND) s/=sstep;
			else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
			else if (s>=-JD_SECOND && s<0.) s=0.;
			stcore->setTimeSpeed(s);
			stapp->temp_time_velocity = stcore->getTimeSpeed();
			// for safest script replay, record as absolute amount
			commandline = "timerate rate " + double_to_str(s/JD_SECOND);

		} else if (args["action"]=="decrement") {
			stapp->FlagTimePause = 0;
			double s = stcore->getTimeSpeed();
			double sstep = 10.;

			if( !args["step"].empty() )
				sstep = str_to_double(args["step"]);

			if (s>JD_SECOND) s/=sstep;
			else if (s<=-JD_SECOND) s*=sstep;
			else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
			else if (s>0. && s<=JD_SECOND) s=0.;
			stcore->setTimeSpeed(s);
			stapp->temp_time_velocity = stcore->getTimeSpeed();
			// for safest script replay, record as absolute amount
			commandline = "timerate rate " + double_to_str(s/JD_SECOND);
		} else status=0;

	} else if (command == "multiplier") {  // script rate multiplier
		if (args["rate"]!="") {
			stapp->setTimeMultiplier(str_to_double(args["rate"]));
			if (!stapp->FlagTimePause)
				stapp->temp_time_velocity = stcore->getTimeSpeed();

		} else if (args["action"]=="increment") {
			// speed up script rate
			double s = stapp->getTimeMultiplier();
			double sstep = 10.0;

			if( !args["step"].empty() )
				sstep = str_to_double(args["step"]);

			stapp->setTimeMultiplier(s*sstep);
			if (!stapp->FlagTimePause)
				stapp->temp_time_velocity = stcore->getTimeSpeed();

			// for safest script replay, record as absolute amount
			commandline = "multiplier rate " + double_to_str(s*sstep);

		} else if (args["action"]=="decrement") {
			// slow rate
			double s = stapp->getTimeMultiplier();
			double sstep = 10.0;

			if( !args["step"].empty() )
				sstep = str_to_double(args["step"]);

			if (!stapp->FlagTimePause)
				stapp->temp_time_velocity = stcore->getTimeSpeed();
			stapp->setTimeMultiplier(s/sstep);

			// for safest script replay, record as absolute amount
			commandline = "multiplier rate " + double_to_str(s/sstep);
		} else status=0;

	} else if (command == "date") {
		if (args["jday"] != "") {
			stcore->setJDay( str_to_double(args["jday"]) );
		}
		else if (args["local"]!="") {
			// ISO 8601-like format [[+/-]YYYY-MM-DD]Thh:mm:ss (no timzone offset, T is literal)
			double jd;
			string new_date;

			if (args["local"][0] == 'T') {
				// set time only (don't change day)
				string sky_date = stapp->get_ISO8601_time_local(stcore->getJDay());
				new_date = sky_date.substr(0,10) + args["local"];
			} else new_date = args["local"];

			if (NShadeDateTime::StringToJday( new_date, jd )) {
				stcore->setJDay(jd - (stapp->get_GMT_shift(jd) * JD_HOUR));
			} else {
				debug_message = _("Error parsing date.");
				status = 0;
			}

		} else if (args["utc"]!="") {
			double jd;
			if (NShadeDateTime::StringToJday( args["utc"], jd ) ) {
				stcore->setJDay(jd);
			} else {
				debug_message = _("Error parsing date.");
				status = 0;
			}

		} else if (args["relative"]!="") { // value is a float number of days
			double days = str_to_double(args["relative"]);
			stcore->setJDay(stcore->getJDay() + days );

		} else if (args["sidereal"]!="") { // value is a float number of sidereal days
			double days = str_to_double(args["sidereal"]);

			const Planet* home = stcore->getObservatory()->getHomePlanet();
			if (home->getEnglishName() != "Solar System Observer")
				days *= home->getSiderealDay();
			stcore->getNavigation()->set_JDay(stcore->getNavigation()->get_JDay() + days );

		} else if (args["load"]=="current") {
			// set date to current date
			stcore->setJDay(NShadeDateTime::JulianFromSys());
		} else if (args["load"]=="preset") {
			// set date to preset (or current) date, based on user setup
			// TODO: should this record as the actual date used?
			if (stapp->StartupTimeMode=="preset" || stapp->StartupTimeMode=="Preset")
				stcore->setJDay(stapp->PresetSkyTime -
				                stapp->get_GMT_shift(stapp->PresetSkyTime) * JD_HOUR);
			else stcore->setJDay(NShadeDateTime::JulianFromSys());

		} else status=0;

	} else if (command == "body") {  // add to svn when reimplement scripting

		if (args["action"] == "load" ) {
			// Load a new solar system object

			// textures relative to script
			args["path"] = stapp->scripts->get_script_path();

			string error_string = stcore->addSolarSystemBody(args);
			if (error_string != "" ) {
				debug_message = error_string;
				status = 0;
			}

		} else if (args["action"] == "drop" && args["name"] != "") {

			//	    if(args["parent"] == "" ) args["parent"] = "none";

			// Delete an existing object, but only if was added by a script!
			string error_string = stcore->removeSolarSystemBody( args["name"] );
			if (error_string != "" ) {
				debug_message = error_string;
				status = 0;
			}

		} else if (args["action"] == "clear") {

			// drop all bodies that are not in the original config file
			string error_string = stcore->removeSupplementalSolarSystemBodies();
			if (error_string != "" ) {
				debug_message = error_string;
				status = 0;
			}

		} else {
			status = 0;
		}

	} else if (command == "moveto") {

		if(args["lat"]=="") args["lat"] = args["latitude"];
		if(args["lon"]=="") args["lon"] = args["longitude"];
		if(args["alt"]=="") args["alt"] = args["altitude"];

		if (args["lat"]!="" || args["lon"]!="" || args["alt"]!="" || args["heading"]!="") {

			Observer *observatory = stcore->getObservatory();

			double lat = observatory->get_latitude();
			double lon = observatory->get_realLongitude();
			double alt = observatory->get_altitude();
			double heading = stcore->getHeading();
			string name;
			int delay;

			if (args["name"]!="") name = args["name"];
			if (args["lat"]!="") {
				if (args["lat"]=="default") lat = observatory->getDefaultLatitude();
				else lat = str_to_double(args["lat"]);
			}
			if (args["lon"]!="") {
				if (args["lon"]=="default") lon = observatory->getDefaultLongitude();
				else lon = str_to_double(args["lon"]);
			}
			if (args["alt"]!="") {
				if (args["alt"]=="default") alt = observatory->getDefaultAltitude();
				else alt = str_to_double(args["alt"]);
			}
			if (args["heading"]!="") {
				if (args["heading"]=="default") heading = stcore->getNavigation()->get_defaultHeading();
				else heading = str_to_double(args["heading"]);
			}

			delay = (int)(1000.*str_to_double(args["duration"]));

			stcore->moveObserver(lat,lon,alt,delay,name);
			stcore->getNavigation()->change_heading(heading, delay);
		} else status = 0;

	} else if (command=="image") {

		ImageMgr& script_images = ImageMgr::getImageMgr("script");

		//cout << "Check " << args["name"] << " " << args["alpha"] << endl;

		if (args["name"]=="") {
			debug_message = _("Image name required.");
			status = 0;
		} else if (args["action"]=="drop") {
			script_images.drop_image(args["name"]);
		} else {
			if (args["action"]=="load" && args["filename"]!="") {

				Image::IMAGE_POSITIONING img_pos = Image::POS_VIEWPORT;
				if (args["coordinate_system"] == "horizontal") img_pos = Image::POS_HORIZONTAL;
				else if (args["coordinate_system"] == "equatorial") img_pos = Image::POS_EQUATORIAL;
				else if (args["coordinate_system"] == "j2000") img_pos = Image::POS_J2000;
				else if (args["coordinate_system"] == "dome") img_pos = Image::POS_DOME;

				string image_filename;
				if (!trusted)
					image_filename = stapp->scripts->get_script_path() + args["filename"];
				else
					image_filename = stcore->getDataRoot() + "/" + args["filename"];

				bool mipmap = 0; // Default off for historical reasons
				if (args["mipmap"] == "on" || args["mipmap"] == "1") mipmap = 1;

				status = script_images.load_image(image_filename, args["name"], img_pos, mipmap);

				if (status==0) debug_message = _("Unable to open file: ") + image_filename;
			}

			if ( status ) {
				Image * img = script_images.get_image(args["name"]);

				if (img != NULL) {
					if (args["alpha"]!="") img->set_alpha(str_to_double(args["alpha"]),
						                                      str_to_double(args["duration"]));
					if (args["scale"]!="") img->set_scale(str_to_double(args["scale"]),
						                                      str_to_double(args["duration"]));
					if (args["rotation"]!="") img->set_rotation(str_to_double(args["rotation"]),
						        str_to_double(args["duration"]));
					if (args["xpos"]!="" || args["ypos"]!="")
						img->set_location(str_to_double(args["xpos"]), args["xpos"]!="",
						                  str_to_double(args["ypos"]), args["ypos"]!="",
						                  str_to_double(args["duration"]));
					// for more human readable scripts, as long as someone doesn't do both...
					if (args["altitude"]!="" || args["azimuth"]!="")
						img->set_location(str_to_double(args["altitude"]), args["altitude"]!="",
						                  str_to_double(args["azimuth"]), args["azimuth"]!="",
						                  str_to_double(args["duration"]));
				} else {
					debug_message = _("Unable to find image: ") + args["name"];
					status=0;
				}
			}
		}
	} else if (command=="audio") {

#ifndef HAVE_SDL_MIXER_H
		debug_message = _("This executable was compiled without audio support.");
		status = 0;
#else

		if (args["action"]=="drop") {
			bool fromscript = true;
			if(args["from_script"] == "false")
				fromscript = false;

			if (audio && audio->drop(fromscript) ) {
				audio = NULL;
			}

		} else if (args["action"]=="sync") {
			if (audio) audio->sync();

		} else if (args["action"]=="pause") {
			if (audio) audio->pause();

		} else if ( args["action"]=="play" && args["filename"]!="") {
			// only one track at a time allowed.
			if (audio) delete audio;

			// if from script, local to that path
			string path;
			if (!trusted) path = stapp->scripts->get_script_path();
			else path = "";

			bool fromscript = true;
			if(args["from_script"] == "false")
				fromscript = false;

			audio = new Audio(path + args["filename"], "default track", str_to_long(args["output_rate"]), fromscript);
			audio->play(args["loop"]=="on");

			if (audioDisabled) audio->disable();
			else audio->enable();

			// if fast forwarding mute (pause) audio
	//		if(stapp->getTimeMultiplier()!=1) audio->pause();

		} else if ( args["action"]=="play" || args["action"]=="resume") {
// resume paused track
			if (audio) audio->resume();

		} else if (args["volume"]!="") {

			recordable = 0;
			if (args["volume"] == "increment") {
				AudioPlayer::Instance().increment_volume();
			} else if (args["volume"] == "decrement") {
				AudioPlayer::Instance().decrement_volume();
			} else AudioPlayer::Instance().set_volume( str_to_double(args["volume"]) );

		} else status = 0;
#endif
	} else if (command == "script") {

		ImageMgr& script_images = ImageMgr::getImageMgr("script");

		if (args["action"]=="end") {
			// stop script, audio, and unload any loaded images
			if (audio && audio->from_script()) {
				delete audio;
				audio = NULL;
			}
			if (ExtViewer) {
				delete ExtViewer;
				ExtViewer = NULL;
				// throttle up application
				stapp->set_minfps(10000);
			}
			stapp->scripts->cancel_script();
			script_images.drop_all_images();
			enableAudio(); // make sure will work next time

		} else if (args["action"]=="play" && args["filename"]!="") {
			string script_path = stapp->scripts->get_script_path();
			
			if (stapp->scripts->is_playing()) {

				// stop script, audio, and unload any loaded images
				if (audio) {
					delete audio;
					audio = NULL;
				}
				if (ExtViewer) {
					delete ExtViewer;
					ExtViewer = NULL;
					// throttle up application
					stapp->set_minfps(10000);
				}
				stapp->scripts->cancel_script();
				script_images.drop_all_images();

				// keep same script path
				if( !stapp->scripts->play_script(script_path + args["filename"], script_path) ) {
					debug_message = string(_("Unable to execute script")) + ": " + script_path + args["filename"];
					status = 0;
				}

			} else {
				// Absolute path is only allowed if trusted caller (application, not script or nscontrol)
				if(args["path"]=="" || !trusted) {
					// Default to local script directory or force relative path
					script_path += args["path"];
					if( !stapp->scripts->play_script(script_path + args["filename"], script_path) ) {
						debug_message = string(_("Unable to execute script")) + ": " + script_path + args["filename"];
						status = 0;
					}
				} else {
					if( !stapp->scripts->play_script(args["path"] + args["filename"], args["path"]) ) {
						debug_message = string(_("Unable to execute script")) + ": " + script_path + args["filename"];
						status = 0;
					}
				}
			}

		} else if (args["action"]=="record") {
			stapp->scripts->record_script(args["filename"]);
			recordable = 0;  // don't record this command!
		} else if (args["action"]=="cancelrecord") {
			stapp->scripts->cancel_record_script();
			recordable = 0;  // don't record this command!
		} else if (args["action"]=="pause" && !stapp->scripts->is_paused()) {
			// n.b. action=pause TOGGLES pause
			disableAudio();

			if (ExtViewer)
				ExtViewer->pause();

			stapp->scripts->pause_script();
		} else if (args["action"]=="pause" || args["action"]=="resume") {
			stapp->scripts->resume_script();

			if( !stapp->scripts->is_faster() )
				enableAudio();

			if (ExtViewer) ExtViewer->resume();
		} else if (args["action"]=="faster") {
			disableAudio();
			stapp->scripts->faster_script();
		} else if (args["action"]=="slower") {
			stapp->scripts->slower_script();

			if (!stapp->scripts->is_faster())
				enableAudio();
		} else status =0;

	} else if (command=="sky_culture") {

		// NEW 201005
		if (args["path"]!="" && args["action"]=="load") {
			string path;
			if(trusted) path = args["path"];
			else path = stapp->scripts->get_script_path() + args["path"];

			status = stcore->loadSkyCulture(path);
			debug_message = "Error loading sky culture from path specified.";
		}

	} else if (command=="nebula") {

		// NEW 201005
		if (args["action"]=="load") {

			string path = stapp->scripts->get_script_path() + args["path"];
			if (args["path"]!="" && trusted) path = args["path"];

			status = stcore->loadNebula(str_to_double(args["ra"]), str_to_double(args["de"]), str_to_double(args["magnitude"]),
										str_to_double(args["angular_size"]), str_to_double(args["rotation"]), args["name"],
										path + args["filename"], args["credit"], str_to_double(args["texture_luminance_adjust"], 1),
										str_to_double(args["distance"], -1));

			debug_message = "Error loading nebula.";

		} else if (args["action"] == "drop" && args["name"] != "") {

			// Delete an existing nebulae, but only if was added by a script!
			string error_string = stcore->removeNebula( args["name"] );
			if (error_string != "" ) {
				debug_message = error_string;
				status = 0;
			}

		} else if (args["action"] == "clear") {

			// drop all nebulae that are not in the original config file
			string error_string = stcore->removeSupplementalNebulae();
			if (error_string != "" ) {
				debug_message = error_string;
				status = 0;
			}

		} else {
			status = 0;
		}

	} else if (command=="clear") {

		// TODO move to stelcore

		// set sky to known, standard states (used by scripts for simplicity)
		execute_command("set home_planet Earth");

		if (args["state"] == "natural") {
			execute_command("flag atmosphere on");
			execute_command("flag landscape on");
		} else {
			execute_command("flag atmosphere off");
			execute_command("flag landscape off");
		}

		// turn off all labels
		execute_command("flag azimuthal_grid off");
		execute_command("flag galactic_grid off");
		execute_command("flag meridian_line off");
		execute_command("flag cardinal_points off");
		execute_command("flag constellation_art off");
		execute_command("flag constellation_drawing off");
		execute_command("flag constellation_names off");
		execute_command("flag constellation_boundaries off");
		execute_command("flag ecliptic_line off");
		execute_command("flag equatorial_grid off");
		execute_command("flag equator_line off");
		execute_command("flag tropic_lines off");
		execute_command("flag circumpolar_circle off");
		execute_command("flag precession_circle off");
		execute_command("flag fog off");
		execute_command("flag nebula_names off");
		execute_command("flag object_trails off");
		execute_command("flag planet_names off");
		execute_command("flag planet_orbits off");
		execute_command("flag show_tui_datetime off");
		execute_command("flag star_names off");
		execute_command("flag show_tui_short_obj_info off");

		// make sure planets, stars, etc. are turned on!
		// milkyway is left to user, for those without 3d cards
		execute_command("flag stars on");
		execute_command("flag planets on");
		execute_command("flag nebulae on");

		// also deselect everything, set to default fov and real time rate
		execute_command("deselect");
		execute_command("timerate rate 1");
		execute_command("zoom auto initial");

	} else if (command=="landscape" && args["action"] == "load") {

		// textures are relative to script
		args["path"] = stapp->scripts->get_script_path();
		stcore->loadLandscape(args);

	} else if (command=="meteors") {
		if (args["zhr"]!="") {
			stcore->setMeteorsRate(str_to_int(args["zhr"]));
		} else status =0;

	} else if (command=="external_viewer") {
		if (args["action"]=="play" && args["filename"]!="") {
			if (ExtViewer) {
				delete(ExtViewer);
				ExtViewer = NULL;
			}

			string script_path = stapp->scripts->get_script_path();
			if(trusted) script_path = "";  // Allow absolute filename

			ExtViewer = new ExternalViewer(script_path + args["filename"],
			                               stcore->getDataDir(), args["coordinate_system"]);
			// throttle down application
			int rate = 24; // fps
			if (args["background_framerate"]!="") {
				rate = str_to_int(args["background_framerate"]);
				if (rate < 10 ) rate = 10;
			}
			stapp->set_minfps(rate);

#ifdef DESKTOP
			stapp->ui->show_message("Started external viewer:\n\n" +
			                        args["filename"], 5000);
#endif
		} else if ((args["action"]=="play" || args["action"]=="resume") && ExtViewer) {
			ExtViewer->resume();
		} else if (args["action"]=="stop" && ExtViewer) {
			delete(ExtViewer);
			ExtViewer = NULL;
			// throttle up application
			stapp->set_minfps(10000);
		} else if (args["action"]=="pause" && ExtViewer) {
			ExtViewer->pause();
		}

		if (ExtViewer) {

			if (args["alpha"]!="") ExtViewer->set_alpha(str_to_double(args["alpha"]),
				        str_to_double(args["duration"]));
			if (args["scale"]!="") ExtViewer->set_scale(str_to_double(args["scale"]),
				        str_to_double(args["duration"]));
			if (args["rotation"]!="") ExtViewer->set_rotation(str_to_double(args["rotation"]),
				        str_to_double(args["duration"]), str_to_bool(args["shortPath"]));
			if (args["altitude"]!="" || args["azimuth"]!="")
				ExtViewer->set_location(str_to_double(args["altitude"]), args["altitude"]!="",
				                        stcore->getHeading() + str_to_double(args["azimuth"]), args["azimuth"]!="",
				                        str_to_double(args["duration"]));
			if (args["clone"]!="") ExtViewer->set_clone(str_to_int(args["clone"]));
			if (args["coordinate_system"]!="") ExtViewer->set_viewport(args["coordinate_system"]);
		}

	} else if (command=="configuration") {

		if(args["action"]=="load") {

			stapp->init();

			#ifndef DESKTOP

				system( ( stcore->getDataDir() + "script_load_config_after " ).c_str() );

			#endif

		} else if(args["action"]=="save"){

			if(!trusted) {
				status = 0;
				debug_message = "Must be trusted to save config.";
			} else {

			#ifndef DESKTOP

				system( ( stcore->getDataDir() + "script_save_config_before " ).c_str() );

			#endif

				stapp->saveCurrentConfig(AppSettings::Instance()->getConfigFile());

			#ifndef DESKTOP

				system( ( stcore->getDataDir() + "script_save_config_after " ).c_str() );

			#endif

			}

		} else {
			status = 0;
		}
	} else if(command=="shutdown") {
		if(trusted) {
			if( AppSettings::Instance()->Digitarium() )
				::system( ( stcore->getDataDir() + "script_shutdown" ).c_str() );
			stapp->quit();
		}
		else {
			status = 0;
			debug_message = "Must be trusted to shutdown.";
		}
	} else if(command=="cove_lights") {
		if(args["protocol"] == "bowen") {
			string cmd("");
			if(args["function"] == "all") {
				RangeMap<float> rmap(1.0, 0.0, 100, 0);
				cmd = "covelights_all_x:x_";
				if( !args["r"].empty() )
					cmd += double_to_str(floor(rmap.Map(str_to_double(args["r"])))) + "_";
				else
					cmd += "-1_";
				if( !args["g"].empty() )
					cmd += double_to_str(floor(rmap.Map(str_to_double(args["g"])))) + "_";
				else
					cmd += "-1_";
				if( !args["b"].empty() )
					cmd += double_to_str(floor(rmap.Map(str_to_double(args["b"])))) + "_";
				else
					cmd += "-1_";

				if( !args["duration"].empty() )
					cmd += args["duration"];
				else
					cmd += "0";
			}
			else if(args["function"] == "preset") {
				cmd = "covelights_preset_";
				cmd += args["number"];
			}
			else if(args["function"] == "connect") {
				cmd.clear();
				if( !NamedSockets::Instance().Connected("Bowen") )
					NamedSockets::Instance().CreateOnCurrentSubnet( "Bowen", "245", 6005 );
			}

			if( !cmd.empty() ) {
				cmd += "*";
				if( NamedSockets::Instance().Connected("Bowen") )
					NamedSockets::Instance().Send( "Bowen", cmd );
			}
		}
		else {
			status = 0;
			debug_message = "Unsupported cove-light protocol.";
		}
	} else if(command=="color") {
		if( !args["property"].empty() && !args["r"].empty() && !args["g"].empty() && !args["b"].empty()) {
			float r = str_to_double(args["r"]), g = str_to_double(args["g"]), b = str_to_double(args["b"]);

			if(args["property"] == "circumpolar_circle")
				stcore->setColorCircumpolarCircle( Vec3f(r, g, b) );
			else if(args["property"] == "constellation_lines")
				stcore->setColorConstellationLine( Vec3f(r, g, b) );
			else if(args["property"] == "constellation_names")
				stcore->setColorConstellationNames( Vec3f(r, g, b) );
			else if(args["property"] == "constellation_art")
				stcore->setColorConstellationArt( Vec3f(r, g, b) );
			else if(args["property"] == "constellation_boundaries")
				stcore->setColorConstellationBoundaries( Vec3f(r, g, b) );
			else if(args["property"] == "cardinal_points")
				stcore->setColorCardinalPoints( Vec3f(r, g, b) );
			else if(args["property"] == "planet_orbits")
				stcore->setColorPlanetsOrbits( Vec3f(r, g, b) );
			else if(args["property"] == "satellite_orbits")
				stcore->setColorSatelliteOrbits( Vec3f(r, g, b) );
			else if(args["property"] == "planet_names")
				stcore->setColorPlanetsNames( Vec3f(r, g, b) );
			else if(args["property"] == "planet_trails")
				stcore->setColorPlanetsTrails( Vec3f(r, g, b) );
			else if(args["property"] == "azimuthal_grid")
				stcore->setColorAzimutalGrid( Vec3f(r, g, b) );
			else if(args["property"] == "galactic_grid")
				stcore->setColorGalacticGrid( Vec3f(r, g, b) );
			else if(args["property"] == "equator_grid")
				stcore->setColorEquatorGrid( Vec3f(r, g, b) );
			else if(args["property"] == "equator_line")
				stcore->setColorEquatorLine( Vec3f(r, g, b) );
			else if(args["property"] == "ecliptic_line")
				stcore->setColorEclipticLine( Vec3f(r, g, b) );
			else if(args["property"] == "meridian_line")
				stcore->setColorMeridianLine( Vec3f(r, g, b) );
			else if(args["property"] == "nebula_names")
				stcore->setColorNebulaLabels( Vec3f(r, g, b) );
			else if(args["property"] == "nebula_circle")
				stcore->setColorNebulaCircle( Vec3f(r, g, b) );
			else if(args["property"] == "precession_circle")
				stcore->setColorPrecessionCircle( Vec3f(r, g, b) );
			else
				debug_message = _("Command 'color': bad value for property argument.");
		}
		else
			debug_message = _("Command 'color': missing expected argument 'property', 'r', 'g' or 'b'.");
	} else {
		debug_message = _("Unrecognized or malformed command name.");
		status = 0;
	}

	if (status ) {

		// if recording commands, do that now
		if (recordable) stapp->scripts->record_command(commandline);

		//    cout << commandline << endl;

	} else {

		// Show gui error window only if script asked for gui debugging
		if (stapp->scripts->is_playing() && stapp->scripts->get_gui_debug())
			stapp->ui->show_message(_("Could not execute command:") + string("\n\"") +
			                        commandline + string("\"\n\n") + debug_message, 7000);

		cerr << "Could not execute: " << commandline << endl << debug_message << endl;
	}

	return(status);

}

// set flags
// if caller is not trusted, some flags can't be changed
// newval is new value of flag changed

int AppCommandInterface::set_flag(string name, string value, bool &newval, bool trusted)
{

	bool status = 1;

	// value can be "on", "off", or "toggle"
	if (value == "toggle") {

		if (trusted) {

			/* disabled due to code rework

			// normal scripts shouldn't be able to change these user settings
			if(name=="enable_zoom_keys") {
				newval = !stcore->getFlagEnableZoomKeys();
				stcore->setFlagEnableZoomKeys(newval); }
			else if(name=="enable_move_keys") {
				newval = !stcore->getFlagEnableMoveKeys();
				stcore->setFlagEnableMoveKeys(newval); }
			else if(name=="enable_move_mouse") newval = (stapp->FlagEnableMoveMouse = !stapp->FlagEnableMoveMouse);
			else if(name=="menu") newval = (stapp->ui->FlagMenu = !stapp->ui->FlagMenu);
			else if(name=="help") newval = (stapp->ui->FlagHelp = !stapp->ui->FlagHelp);
			else if(name=="infos") newval = (stapp->ui->FlagInfos = !stapp->ui->FlagInfos);
			else if(name=="show_topbar") newval = (stapp->ui->FlagShowTopBar = !stapp->ui->FlagShowTopBar);
			else if(name=="show_time") newval = (stapp->ui->FlagShowTime = !stapp->ui->FlagShowTime);
			else if(name=="show_date") newval = (stapp->ui->FlagShowDate = !stapp->ui->FlagShowDate);
			else if(name=="show_appname") newval = (stapp->ui->FlagShowAppName = !stapp->ui->FlagShowAppName);
			else if(name=="show_fps") newval = (stapp->ui->FlagShowFps = !stapp->ui->FlagShowFps);
			else if(name=="show_fov") newval = (stapp->ui->FlagShowFov = !stapp->ui->FlagShowFov);
			else if(name=="enable_tui_menu") newval = (stapp->ui->FlagEnableTuiMenu = !stapp->ui->FlagEnableTuiMenu);
			else if(name=="show_gravity_ui") newval = (stapp->ui->FlagShowGravityUi = !stapp->ui->FlagShowGravityUi);
			else if(name=="gravity_labels") {
				newval = !stcore->getFlagGravityLabels();
				stcore->setFlagGravityLabels(newval);
				}
			else status = 0;  // no match here, anyway
			*/
			status = 0;

		} else status = 0;


 		if (name=="antialias_lines") {
			newval = !stcore->getFlagAntialiasLines();
			stcore->setFlagAntialiasLines(newval);
		} else if (name=="constellation_drawing") {
			newval = !stcore->getFlagConstellationLines();
			stcore->setFlagConstellationLines(newval);
		} else if (name=="constellation_names") {
			newval = !stcore->getFlagConstellationNames();
			stcore->setFlagConstellationNames(newval);
		} else if (name=="constellation_art") {
			newval = !stcore->getFlagConstellationArt();
			stcore->setFlagConstellationArt(newval);
		} else if (name=="constellation_boundaries") {
			newval = !stcore->getFlagConstellationBoundaries();
			stcore->setFlagConstellationBoundaries(newval);
		} else if (name=="constellation_pick") {
			newval = !stcore->getFlagConstellationIsolateSelected();
			stcore->setFlagConstellationIsolateSelected(newval);
		} else if (name=="star_twinkle") {
			newval = !stcore->getFlagStarTwinkle();
			stcore->setFlagStarTwinkle(newval);
		} else if (name=="point_star") {
			newval = !stcore->getFlagPointStar();
			stcore->setFlagPointStar(newval);
		} else if (name=="show_selected_object_info") newval = (stapp->ui->FlagShowSelectedObjectInfo = !stapp->ui->FlagShowSelectedObjectInfo);
		else if (name=="show_tui_datetime") {
			newval = (stapp->ui->FlagShowTuiDateTime = !stapp->ui->FlagShowTuiDateTime);
			ReferenceState state;
			state.show_tui_date_time = newval;
			SharedData::Instance()->References( state );
		}
		else if (name=="show_tui_short_obj_info") {
			newval = (stapp->ui->FlagShowTuiShortObjInfo = !stapp->ui->FlagShowTuiShortObjInfo);
			ReferenceState state;
			state.show_tui_short_obj_info = newval;
			SharedData::Instance()->References( state );
		}
		else if (name=="manual_zoom") {
			newval = !stcore->getFlagManualAutoZoom();
			stcore->setFlagManualAutoZoom(newval);
		} else if (name=="light_travel_time") {
			newval = !stcore->getFlagLightTravelTime();
			stcore->setFlagLightTravelTime(newval);
		} else if (name=="show_script_bar") newval = (stapp->ui->FlagShowScriptBar = !stapp->ui->FlagShowScriptBar);
		else if (name=="fog") {
			newval = !stcore->getFlagFog();
			stcore->setFlagFog(newval);
		} else if (name=="atmosphere") {
			newval = !stcore->getFlagAtmosphere();
			stcore->setFlagAtmosphere(newval);
			if (!newval) stcore->setFlagFog(false); // turn off fog with atmosphere
		}
		/*		else if(name=="chart") {
			newval = !stapp->getVisionModeChart();
			if (newval) stapp->setVisionModeChart();
		}
		else if(name=="night") {
			newval = !stapp->getVisionModeNight();
			if (newval) stapp->setVisionModeNight();
		}
		*/
		//else if(name=="use_common_names") newval = (stcore->FlagUseCommonNames = !stcore->FlagUseCommonNames);
		else if (name=="azimuthal_grid") {
			newval = !stcore->getFlagAzimutalGrid();
			stcore->setFlagAzimutalGrid(newval);
		} else if (name=="galactic_grid") {
			newval = !stcore->getFlagGalacticGrid();
			stcore->setFlagGalacticGrid(newval);
		} else if (name=="equatorial_grid") {
			newval = !stcore->getFlagEquatorGrid();
			stcore->setFlagEquatorGrid(newval);
		} else if (name=="equator_line") {
			newval = !stcore->getFlagEquatorLine();
			stcore->setFlagEquatorLine(newval);
		} else if (name=="ecliptic_line") {
			newval = !stcore->getFlagEclipticLine();
			stcore->setFlagEclipticLine(newval);
		} else if (name=="precession_circle") {
			newval = !stcore->getFlagPrecessionCircle();
			stcore->setFlagPrecessionCircle(newval);
		} else if (name=="tropic_lines") {
			newval = !stcore->getFlagTropicLines();
			stcore->setFlagTropicLines(newval);
		} else if (name=="circumpolar_circle") {
			newval = !stcore->getFlagCircumpolarCircle();
			stcore->setFlagCircumpolarCircle(newval);
		} else if (name=="meridian_line") {
			newval = !stcore->getFlagMeridianLine();
			stcore->setFlagMeridianLine(newval);
		} else if (name=="cardinal_points") {
			newval = !stcore->getFlagCardinalsPoints();
			stcore->setFlagCardinalsPoints(newval);
		} else if (name=="clouds") {
			newval = !stcore->getFlagClouds();
			stcore->setFlagClouds(newval);
		} else if (name=="moon_scaled") {
			newval = !stcore->getFlagMoonScaled();
			stcore->setFlagMoonScaled(newval);
		} else if (name=="landscape") {
			newval = !stcore->getFlagLandscape();
			stcore->setFlagLandscape(newval);
		} else if (name=="stars") {
			newval = !stcore->getFlagStars();
			stcore->setFlagStars(newval);
		} else if (name=="star_names") {
			newval = !stcore->getFlagStarName();
			stcore->setFlagStarName(newval);
		} else if (name=="planets") {
			newval = !stcore->getFlagPlanets();
			stcore->setFlagPlanets(newval);
		} else if (name=="planet_names") {
			newval = !stcore->getFlagPlanetsHints();
			stcore->setFlagPlanetsHints(newval);
			if (stcore->getFlagPlanetsHints()) stcore->setFlagPlanets(true); // for safety if script turns planets off
		} else if (name=="planet_orbits") {
			newval = !stcore->getFlagPlanetsOrbits();
			stcore->setFlagPlanetsOrbits(newval);
		} else if (name=="nebulae") {
			newval = !stcore->getFlagNebula();
			stcore->setFlagNebula(newval);
		} else if (name=="nebula_names") {
			newval = !stcore->getFlagNebulaHints();
			if (newval) stcore->setFlagNebula(true); // make sure visible
			stcore->setFlagNebulaHints(newval);
		} else if (name=="milky_way") {
			newval = !stcore->getFlagMilkyWay();
			stcore->setFlagMilkyWay(newval);
		} else if (name=="bright_nebulae") {
			newval = !stcore->getFlagBrightNebulae();
			stcore->setFlagBrightNebulae(newval);
		} else if (name=="object_trails") {
			newval = !stcore->getFlagPlanetsTrails();
			stcore->setFlagPlanetsTrails(newval);
		} else if (name=="track_object") {
			newval = !stcore->getFlagTracking();
			stcore->setFlagTracking(newval);
		} else if (name=="script_gui_debug") {  // Not written to config - script specific
			newval = !stapp->scripts->get_gui_debug();
			stapp->scripts->set_gui_debug(newval);
		} else return(status); // no matching flag found untrusted, but maybe trusted matched

	} else {

		newval = (value == "on" || value == "1");

		if (trusted) {

			/* disabled due to code rework
			// normal scripts shouldn't be able to change these user settings
			if(name=="enable_zoom_keys") stcore->setFlagEnableZoomKeys(newval);
			else if(name=="enable_move_keys") stcore->setFlagEnableMoveKeys(newval);
			else if(name=="enable_move_mouse") stapp->FlagEnableMoveMouse = newval;
			else if(name=="menu") stapp->ui->FlagMenu = newval;
			else if(name=="help") stapp->ui->FlagHelp = newval;
			else if(name=="infos") stapp->ui->FlagInfos = newval;
			else if(name=="show_topbar") stapp->ui->FlagShowTopBar = newval;
			else if(name=="show_time") stapp->ui->FlagShowTime = newval;
			else if(name=="show_date") stapp->ui->FlagShowDate = newval;
			else if(name=="show_appname") stapp->ui->FlagShowAppName = newval;
			else if(name=="show_fps") stapp->ui->FlagShowFps = newval;
			else if(name=="show_fov") stapp->ui->FlagShowFov = newval;
			else if(name=="enable_tui_menu") stapp->ui->FlagEnableTuiMenu = newval;
			else if(name=="show_gravity_ui") stapp->ui->FlagShowGravityUi = newval;
			else if(name=="gravity_labels") stcore->setFlagGravityLabels(newval);
			else status = 0;

			*/
			status = 0;

		} else status = 0;

 		if (name=="antialias_lines") stcore->setFlagAntialiasLines(newval);
		else if (name=="constellation_drawing") stcore->setFlagConstellationLines(newval);
		else if (name=="constellation_names") stcore->setFlagConstellationNames(newval);
		else if (name=="constellation_art") stcore->setFlagConstellationArt(newval);
		else if (name=="constellation_boundaries") stcore->setFlagConstellationBoundaries(newval);
		else if (name=="constellation_pick") stcore->setFlagConstellationIsolateSelected(newval);
		else if (name=="star_twinkle") stcore->setFlagStarTwinkle(newval);
		else if (name=="point_star") stcore->setFlagPointStar(newval);
		else if (name=="show_selected_object_info") stapp->ui->FlagShowSelectedObjectInfo = newval;
		else if (name=="show_tui_datetime") {
			stapp->ui->FlagShowTuiDateTime = newval;
			ReferenceState state;
			state.show_tui_date_time = newval;
			SharedData::Instance()->References( state );
		}
		else if (name=="show_tui_short_obj_info") {
			stapp->ui->FlagShowTuiShortObjInfo = newval;
			ReferenceState state;
			state.show_tui_short_obj_info = newval;
			SharedData::Instance()->References( state );
		}
		else if (name=="manual_zoom") stcore->setFlagManualAutoZoom(newval);
		else if (name=="light_travel_time") stcore->setFlagLightTravelTime(newval);
		else if (name=="show_script_bar") stapp->ui->FlagShowScriptBar = newval;
		else if (name=="fog") stcore->setFlagFog(newval);
		else if (name=="atmosphere") {
			stcore->setFlagAtmosphere ( newval);
			if (!newval) stcore->setFlagFog(false); // turn off fog with atmosphere
		}
		/*		else if(name=="chart") {
			if (newval) stapp->setVisionModeChart();
		}
		else if(name=="night") {
			if (newval) stapp->setVisionModeNight();
		}
		*/
		else if (name=="azimuthal_grid") stcore->setFlagAzimutalGrid(newval);
		else if (name=="galactic_grid") stcore->setFlagGalacticGrid(newval);
		else if (name=="equatorial_grid") stcore->setFlagEquatorGrid(newval);
		else if (name=="equator_line") stcore->setFlagEquatorLine(newval);
		else if (name=="ecliptic_line") stcore->setFlagEclipticLine(newval);
		else if (name=="precession_circle") stcore->setFlagPrecessionCircle(newval);
		else if (name=="circumpolar_circle") stcore->setFlagCircumpolarCircle(newval);
		else if (name=="tropic_lines") stcore->setFlagTropicLines(newval);
		else if (name=="meridian_line") stcore->setFlagMeridianLine(newval);
		else if (name=="cardinal_points") stcore->setFlagCardinalsPoints(newval);
		else if (name=="clouds") stcore->setFlagClouds(newval);
		else if (name=="moon_scaled") stcore->setFlagMoonScaled(newval);
		else if (name=="landscape") stcore->setFlagLandscape(newval);
		else if (name=="stars") stcore->setFlagStars(newval);
		else if (name=="star_names") stcore->setFlagStarName(newval);
		else if (name=="planets") {
			stcore->setFlagPlanets(newval);
			if (!stcore->getFlagPlanets()) stcore->setFlagPlanetsHints(false);
		} else if (name=="planet_names") {
			stcore->setFlagPlanetsHints(newval);
			if (stcore->getFlagPlanetsHints()) stcore->setFlagPlanets(true); // for safety if script turns planets off
		} else if (name=="planet_orbits") stcore->setFlagPlanetsOrbits(newval);
		else if (name=="nebulae") stcore->setFlagNebula(newval);
		else if (name=="nebula_names") {
			stcore->setFlagNebula(true);  // make sure visible
			stcore->setFlagNebulaHints(newval);
		} else if (name=="milky_way") stcore->setFlagMilkyWay(newval);
		else if (name=="bright_nebulae") stcore->setFlagBrightNebulae(newval);
		else if (name=="object_trails") stcore->setFlagPlanetsTrails(newval);
		else if (name=="track_object") stcore->setFlagTracking(newval);
		else if (name=="script_gui_debug") stapp->scripts->set_gui_debug(newval); // Not written to config - script specific
		else return(status);

	}


	return(1);  // flag was found and updated

}

string AppCommandInterface::getErrorString( void ) {
	return( debug_message );
}

void AppCommandInterface::update(int delta_time)
{
	if (audio) audio->update(delta_time);
}
