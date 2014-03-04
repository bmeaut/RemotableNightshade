/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2009 Lionel Ruiz
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

// Handle Keys for LSS planetarium system

int UI::handle_keys(SDLKey key, SDLMod mod, Uint16 unicode, S_GUI_VALUE state)
{
	int retVal = 1;
	std::ostringstream oss;
	double latimem;
	
	string homeDir = getenv("HOME");
	string CDIR2 = homeDir + "/." + APP_LOWER_NAME + "/";

	if (desktop->onKey(unicode, state))
		return 1;

	if (state==S_GUI_PRESSED)
	{
		//printf("handle_keys: '%c'(%d), %d, 0x%04x\n",key,(int)key,unicode,mod);
		//if (unicode >= 128) {
		  // the user has entered an arkane symbol which cannot
		  // be a key shortcut.
		//  return 1;
		//}
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

		if (key == SDLK_q && (mod & COMPATIBLE_KMOD_CTRL))
		{
			app->quit();
		}

		// if script is running, only script control keys are accessible
		// to pause/resume/cancel the script
		// (otherwise script could get very confused by user interaction)
		if(app->scripts->is_playing())
		{

			// here reusing time control keys to control the script playback
			if((key==SDLK_h)||(key==SDLK_p)) 
			{
				// pause/unpause script
				app->commander->execute_command( "script action pause");
				app->time_multiplier = 1;  // don't allow resumption of ffwd this way (confusing for audio)
			}
			else if(key==SDLK_k || key==SDLK_SPACE || key==SDLK_TAB  || key==SDLK_y)
			{
				app->commander->execute_command( "script action resume");
				app->time_multiplier = 1;
			}
			else if(key==SDLK_g || unicode==0x0003 || (key==SDLK_m && FlagEnableTuiMenu))
			{  // ctrl-c
				// TODO: should double check with user here...
				app->commander->execute_command( "script action end");
				if(key==SDLK_m) setFlagShowTuiMenu(true);
			}
			// TODO n is bad key if ui allowed
			// DIGITALIS
			else if(key==SDLK_RIGHTBRACKET)
			{
				app->commander->execute_command( "audio volume increment");
				return 1;
			}
			// TODO d is bad key if ui allowed
			// DIGITALIS
			else if(key==SDLK_LEFTBRACKET)
			{
				app->commander->execute_command( "audio volume decrement");
				return 1;

			}
			else if(key==SDLK_SEMICOLON)
			{
				if(app->time_multiplier==2)
				{
					app->time_multiplier = 1;

					// restart audio in correct place
					app->commander->enableAudio();
				}
				else if(app->time_multiplier > 1 )
				{
					app->time_multiplier /= 2;
				}

			}
			else if(key==SDLK_l)
			{
				// stop audio since won't play at higher speeds
				app->commander->disableAudio();
				app->time_multiplier *= 2;
				if(app->time_multiplier>8) app->time_multiplier = 8;
			}
			else if(!app->scripts->get_allow_ui())
			{
				// DIGITALIS
				//				cout << "Playing a script.  Press CTRL-C (or 7) to stop." << endl;
			}

			//if(!app->scripts->get_allow_ui()) return 0;  // only limited user interaction allowed with script

		}
		else
		{
			app->time_multiplier = 1;  // if no script in progress always real time

			// normal time controls here (taken over for script control above if playing a script)
			if(key==SDLK_k || key==SDLK_SPACE) app->commander->execute_command( "timerate rate 1");
			if(key==SDLK_l) {
				if (!(mod & COMPATIBLE_KMOD_CTRL)) {
					app->commander->execute_command( "timerate action increment");
	    			} else {
            				app->commander->execute_command("date relative 146095.9992");
				}
			}
			if(key==SDLK_j) {
				if (!(mod & COMPATIBLE_KMOD_CTRL)) {
					app->commander->execute_command( "timerate action decrement");
	    			} else {
            				app->commander->execute_command("date relative -146095.9992");
				}
			}
			if(key==SDLK_h) app->commander->execute_command( "timerate action pause");
			if(key==SDLK_g) app->commander->execute_command( "timerate rate 0");
			if(key==SDLK_DOLLAR) app->commander->execute_command( "date load preset");

		}
 
		// DIGITALIS shifted commands
		if(shiftModifier) {
			shiftModifier = 0;
	//std::string action = "/home/planetarium/main.sh &";
	//system(action.c_str());			
			if(key == SDLK_DOLLAR) {  // DIGITALIS 200703
			  // Reload defaults from config file
			  app->init();
			  app->commander->execute_command("body action clear");
			  app->scripts->play_script(CDIR2+"fscripts/M00.sts", CDIR2+"fscripts/");
			  return 1;
			}			
			if(key == SDLK_CARET) {
			  // Change home planet to selected planet!
			  // DIGITALIS version 20080430
			  string planet = core->getSelectedPlanetEnglishName();
			  if(planet!="") app->commander->execute_command( string("set home_planet ") + planet + string(""));
			  return 1;
			}

			if(key == SDLK_WORLD_89) {
			  app->commander->execute_command( "nebula action clear");
			  app->commander->execute_command( "set sky_culture western-color");
			  app->commander->execute_command( "set sky_locale fr");
			  app->commander->execute_command( "external_viewer action stop");
			  app->commander->execute_command( "body action clear");
			return 1;
			}

			if(key == SDLK_UNDERSCORE) {
				app->scripts->play_script(CDIR2+"fscripts/nebulae_drawings.sts", CDIR2+"fscripts/");
			return 1;
			}

			if(key == SDLK_q) {
				app->scripts->play_script(CDIR2+"fscripts/windrose.sts", CDIR2+"fscripts/");
			return 1;
			}

			if(key == SDLK_d) {
			  app->commander->execute_command( "flag tropic_lines toggle");
			  return 1;
			}

			if(key == SDLK_c) {
			  app->commander->execute_command( "flag circumpolar_circle toggle");
			  return 1;
			}

			if(key == SDLK_s) {
			  app->commander->execute_command( "flag precession_circle toggle");
			  return 1;
			}

			if(key == SDLK_v) {
			  app->commander->execute_command( "flag show_tui_short_obj_info toggle");
			  return 1;
			}

			if(key == SDLK_b) {
				if (core->getMeteorsRate()<=10000) app->commander->execute_command("meteors zhr 150000"); else app->commander->execute_command("meteors zhr 10");
			  return 1;
			}
			if(key == SDLK_COLON) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/05.mp3 action play");
			  return 1;
			}
			if(key == SDLK_SEMICOLON) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/06.mp3 action play");
			  return 1;
			}
			if(key == SDLK_COMMA) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/07.mp3 action play");
			  return 1;
			}
			if(key == SDLK_EXCLAIM) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/08.mp3 action play");
			  return 1;
			}
			if(key == SDLK_0) {
				app->scripts->play_script(CDIR2+"fscripts/M10.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_1) {
				app->scripts->play_script(CDIR2+"fscripts/M11.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_2) {
				app->scripts->play_script(CDIR2+"fscripts/M12.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_3) {
				app->scripts->play_script(CDIR2+"fscripts/M13.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_4) {
				app->scripts->play_script(CDIR2+"fscripts/M14.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_5) {
				app->scripts->play_script(CDIR2+"fscripts/M15.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_6) {
				app->scripts->play_script(CDIR2+"fscripts/M16.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_7) {
				app->scripts->play_script(CDIR2+"fscripts/M17.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_8) {
				app->scripts->play_script(CDIR2+"fscripts/M18.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_9) {
				app->scripts->play_script(CDIR2+"fscripts/M19.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_PERIOD) {
				app->scripts->play_script(CDIR2+"fscripts/M21.sts", CDIR2+"fscripts/");
			return 1;
			}
			if(key == SDLK_WORLD_73) {
				app->commander->execute_command("flag planet_orbits toggle");
				return 1;
			}
			if(key == SDLK_a) {
				//app->commander->execute_command("flag constellation_boundaries toggle");
 				//app->commander->execute_command("deselect");     
				app->commander->execute_command("set sky_culture western-mod");
				return 1;
			}
			if(key == SDLK_y) {
			  std::string action = CDIR2+"ftp/pub/masterput.sh &";
			  system(action.c_str());			
				return 1;
			}
			if(key == SDLK_z) {
				app->commander->execute_command("set sky_locale en");
				return 1;
			}
			if(key == SDLK_j) {
				app->commander->execute_command("date sidereal -365");
				return 1;
			}
			if(key == SDLK_l) {
				app->commander->execute_command("date sidereal 365");
				return 1;
			}
			if(key == SDLK_MINUS) {
				app->commander->execute_command("deselect");
				return 1;
			}
			if(key == SDLK_r) {
				app->commander->execute_command("deselect");
				return 1;
			}
			if(key == SDLK_e) {
				//app->commander->execute_command("set sky_culture hevelius");
				app->commander->execute_command("deselect");
				app->commander->execute_command("select constellation Ari pointer off");
				app->commander->execute_command("select constellation Tau pointer off");
				app->commander->execute_command("select constellation Gem pointer off");
				app->commander->execute_command("select constellation Cnc pointer off");
				app->commander->execute_command("select constellation Leo pointer off");
				app->commander->execute_command("select constellation Vir pointer off");
				app->commander->execute_command("select constellation Lib pointer off");
				app->commander->execute_command("select constellation Sco pointer off");
				app->commander->execute_command("select constellation Sgr pointer off");
				app->commander->execute_command("select constellation Cap pointer off");
				app->commander->execute_command("select constellation Aqr pointer off");
				app->commander->execute_command("select constellation Psc pointer off");
				return 1;
			}
			if(key == SDLK_t) {
				app->commander->execute_command("flag object_trails toggle");
				//				cout << "trails toggle\n";
				return 1;
			}
 			if(key == SDLK_WORLD_71) {
				app->commander->execute_command("moveto lat -90 duration 5");
				//latimem = core->getobservatory().get_latitude();
				//latimem = latimem - 30;
				//oss << "moveto lat " << latimem << " duration 5";
				//app->commander->execute_command(oss.str());
				return 1;
			}
		  	if(key == SDLK_WORLD_64) {
				app->commander->execute_command("moveto lat 90 duration 5");
				//latimem = core->getobservatory().get_latitude();
				//latimem = latimem + 30;
				//oss << "moveto lat " << latimem << " duration 5";
				//app->commander->execute_command(oss.str());
				return 1;
			}
		  	if(key == SDLK_F1) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/13.avi action play");
				return 1;
			}
		  	if(key == SDLK_F2) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/14.avi action play");
				return 1;
			}
		  	if(key == SDLK_F3) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/15.avi action play");
				return 1;
			}
		  	if(key == SDLK_F4) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/16.avi action play");
				return 1;
			}
		  	if(key == SDLK_F5) {
				app->SelectedScriptDirectory = core->getDataDir() + "scripts/";
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/17.avi action play");
				return 1;
			}
		  	if(key == SDLK_F6) {
				app->SelectedScriptDirectory = core->getDataDir() + "scripts/";
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/18.avi action play");
				return 1;
			}
		  	if(key == SDLK_F7) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/19.avi action play");
				return 1;
			}
		  	if(key == SDLK_F8) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/20.avi action play");
				return 1;
			}
		  	if(key == SDLK_F9) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/21.avi action play");
				return 1;
			}
		  	if(key == SDLK_F10) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/22.avi action play");
				return 1;
			}
		  	if(key == SDLK_F11) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/23.avi action play");
				return 1;
			}
		  	if(key == SDLK_F12) {
				app->commander->execute_command("external_viewer filename ~/.nightshade/videos/24.avi action play");
				return 1;
			}
			if(key == SDLK_x) {
				app->commander->execute_command("flag azimuthal_grid toggle");
				return 1;
			}
			if(key == SDLK_AMPERSAND) {
		            if (mod & COMPATIBLE_KMOD_CTRL) {
				app->scripts->play_script(CDIR2+"fscripts/13.sts", CDIR2+"fscripts/");
			    } else {
				app->commander->execute_command("flag stars toggle");
				return 1;
			    }
			}
			if(key == SDLK_RIGHTPAREN) {
			      	app->commander->execute_command( "zoom auto in");
				app->commander->execute_command( "zoom fov 360 duration 5");
				return 1;
			}
			if(key == SDLK_EQUALS) {
			      	app->commander->execute_command( "zoom auto in");
				app->commander->execute_command( "zoom fov 10 duration 5");
				return 1;
			}
		}

		// DIGITALIS shift key (²)
		if(key == SDLK_WORLD_18) {
			//			cout << "Hit shift button\n";
			shiftModifier = 1;
			ShiftTimeLeft = 3*1000;

	//std::string action = "/home/planetarium/second.sh &";
	//system(action.c_str());			
			
		}

// DIGITALIS
#ifdef DESKTOP
		if (key == SDLK_HASH && (mod & COMPATIBLE_KMOD_CTRL))
		{
			if(app->scripts->is_recording())
			{
				app->commander->execute_command( "script action cancelrecord");
				show_message(_("Command recording stopped."), 3000);
			}
			else
			{
				app->commander->execute_command( "script action record");

				if(app->scripts->is_recording())
				{
					show_message(wstring( _("Recording commands to script file:\n")
					                      + Utility::stringToWstring(app->scripts->get_record_filename()) + L"\n\n"
					                      + _("Hit CTRL-R again to stop.\n")), 4000);
				}
				else
				{
					show_message(_("Error: Unable to open script file to record commands."), 3000);
				}
			}
            return 0;
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
		  case SDLK_LEFTPAREN:
		            if (mod & COMPATIBLE_KMOD_CTRL) {
				app->scripts->play_script(CDIR2+"fscripts/17.sts", CDIR2+"fscripts/");
			    } else {
		app->commander->execute_command( "flag planets toggle");
			    }
            break;
		  case SDLK_UNDERSCORE:
				app->commander->execute_command("flag nebulae toggle");
            break;
		  case SDLK_t:
				app->commander->execute_command("flag object_trails toggle");
            break;
		  case SDLK_WORLD_71:
	            if (mod & COMPATIBLE_KMOD_CTRL) {
		    } else {
			latimem = core->getObservatory()->get_latitude();
			latimem = latimem - 45;
			oss << "moveto lat " << latimem << " duration 7";
			app->commander->execute_command(oss.str());
		    }
            break;
		  case SDLK_WORLD_64:
	            if (mod & COMPATIBLE_KMOD_CTRL) {
			latimem = core->getObservatory()->get_latitude();
			latimem = latimem + 30;
			oss << "moveto lat " << latimem << " duration 4";
			app->commander->execute_command(oss.str());
		    } else {
			latimem = core->getObservatory()->get_latitude();
			latimem = latimem + 45;
			oss << "moveto lat " << latimem << " duration 7";
			app->commander->execute_command(oss.str());
		    }
            break;
	    case SDLK_s:  // DIGITALIS
		app->commander->execute_command( "flag ecliptic_line toggle");
            break;
            case SDLK_d:
	    {
            	app->commander->execute_command( "flag equator_line toggle");
	    }
            break;
	    case SDLK_0:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K0.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M00.sts", CDIR2+"fscripts/");
	    }
	    break;
	    case SDLK_1:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K1.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M01.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_2:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K2.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M02.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_3:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K3.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M03.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_4:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K4.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M04.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_5:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K5.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M05.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_6:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K6.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M06.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_7:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K7.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M07.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_8:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K8.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M08.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_9:
            if (mod & KMOD_SHIFT) {
		app->scripts->play_script(CDIR2+"fscripts/K9.sts", CDIR2+"fscripts/");
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M09.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_PERIOD:
            if (mod & KMOD_SHIFT) {
	    } else {
		app->scripts->play_script(CDIR2+"fscripts/M20.sts", CDIR2+"fscripts/");
	    }
            break;
	    case SDLK_b:
		if (core->getMeteorsRate()==10) app->commander->execute_command("meteors zhr 10000"); else app->commander->execute_command("meteors zhr 10");
              /*const int zhr = core->getMeteorsRate();
              if (zhr <= 10 ) {
                app->commander->execute_command("meteors zhr 80");  // standard Perseids rate
              } else if( zhr <= 80 ) {
                app->commander->execute_command("meteors zhr 10000"); // exceptional Leonid rate
              } else if( zhr <= 10000 ) {
                app->commander->execute_command("meteors zhr 144000");  // highest ever recorded ZHR (1966 Leonids)
              } else {
                app->commander->execute_command("meteors zhr 10");  // set to default base rate (10 is normal, 0 would be none)
              }*/
            break;

          case SDLK_y:
           if (mod & COMPATIBLE_KMOD_CTRL) {
//			latimem = core->getObservatory().get_longitude();
//			latimem = latimem - 90;
//			oss << "moveto lon " << latimem << " duration 7";
//			app->commander->execute_command(oss.str());
		app->scripts->play_script(CDIR2+"ftp/pub/viewport.sts", CDIR2+"ftp/pub/");
//                core->setFlipVert(!core->getFlipVert());
	   } else {
                //core->setFlipHorz(!core->getFlipHorz());
		app->scripts->play_script(CDIR2+"ftp/pub/script.sts", CDIR2+"ftp/pub/");
	   }
            break;
          case SDLK_z:
              app->commander->execute_command( "flag constellation_names toggle");
            break;
          case SDLK_WORLD_72:
              app->commander->execute_command( "flag milky_way toggle");
              //core->setMilkyWay("milkyway_chart.png");
            break;
          case SDLK_QUOTE:
            if (mod & COMPATIBLE_KMOD_CTRL) {
				app->scripts->play_script(CDIR2+"fscripts/16.sts", CDIR2+"fscripts/");
            } else {
              app->commander->execute_command( "flag fog toggle");
            }
          break;

          case SDLK_e:
            app->commander->execute_command( "flag constellation_art toggle");
            break;
          case SDLK_a:
            app->commander->execute_command( "flag constellation_drawing toggle");
            break;
          case SDLK_r:
            app->commander->execute_command( "flag constellation_boundaries toggle");
            break;
          case SDLK_AMPERSAND:
            if (mod & COMPATIBLE_KMOD_CTRL) {
	      // Reload defaults from config file
	      // app->init();
		app->scripts->play_script(CDIR2+"fscripts/13.sts", CDIR2+"fscripts/");
	    } else app->commander->execute_command( "flag star_names toggle");
            break;
		case SDLK_WORLD_73:  // DIGITALIS
		            if (mod & COMPATIBLE_KMOD_CTRL) {
				app->scripts->play_script(CDIR2+"fscripts/14.sts", CDIR2+"fscripts/");
			    } else {
			app->commander->execute_command("flag planet_names toggle");
			    }
            break;
		case SDLK_x: // DIGITALIS
              app->commander->execute_command( "flag meridian_line toggle");
            break;
          case SDLK_c:
            app->commander->execute_command( "flag equatorial_grid toggle");
            break;
          case SDLK_QUOTEDBL:
            if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/15.sts", CDIR2+"fscripts/");
            } else {
            	app->commander->execute_command( "flag nebula_names toggle");
            }
            break;
          case SDLK_ASTERISK:
            app->commander->execute_command( "set sky_culture western-color");
            app->commander->execute_command( "set sky_locale fr");
            app->commander->execute_command( "external_viewer action stop");
            app->commander->execute_command( "body action clear");
            break;
          case SDLK_LESS:
            if (!(mod & COMPATIBLE_KMOD_CTRL))
	      app->commander->execute_command( "flag landscape toggle");
	    else {
	    }
	      
            break;
          case SDLK_q:
            app->commander->execute_command( "flag cardinal_points toggle");
            break;
          case SDLK_w:
            app->commander->execute_command( "flag atmosphere toggle");
            break;

          case SDLK_WORLD_89:
            core->setFlagLockSkyPosition(!core->getFlagLockSkyPosition());
            break;
          case SDLK_MINUS:
            if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/18.sts", CDIR2+"fscripts/");
	    } else {
              app->commander->execute_command( "flag stars toggle");
	    }
            break;
          case SDLK_CARET:
            if (!(mod & COMPATIBLE_KMOD_CTRL)) {
            	app->commander->execute_command("flag track_object on");
	    } else {
	      string planet = core->getSelectedPlanetEnglishName();
	      if(planet!="") app->commander->execute_command( string("set home_planet ") + planet + string(""));
	    }
            break;
          case SDLK_PLUS:
            break;
          case SDLK_SLASH:
            break;
          case SDLK_n:
     app->commander->execute_command( "external_viewer action stop");
            break;
          case SDLK_WORLD_0:
            FlagInfos=!FlagInfos;
            licence_win->setVisible(FlagInfos);
            break;
          case SDLK_m:
            if (FlagEnableTuiMenu) setFlagShowTuiMenu(true);  // not recorded
            break;
          case SDLK_f:
            app->commander->execute_command( "flag moon_scaled toggle");
            break;
          case SDLK_u:
	    // DIGITALIS - 20070123 add to svn
	    //if(app->DayKeyMode != "sidereal")
	    //if(app->DayKeyMode != "sidereal")
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
		app->commander->execute_command( "date sidereal -7");
	    } else {
	        app->commander->execute_command( "date relative -7");
	    }
            break;
          case SDLK_i:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
		app->commander->execute_command( "date sidereal -1");
	    } else {
	        app->commander->execute_command( "date relative -1");
	    }
            break;
          case SDLK_o:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
		app->commander->execute_command( "date sidereal 1");
	    } else {
	        app->commander->execute_command( "date relative 1");
	    }
            break;
          case SDLK_p:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
		app->commander->execute_command( "date sidereal 7");
	    } else {
	        app->commander->execute_command( "date relative 7");
	    }
            break;
          case SDLK_COLON:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/03.mp3 action play");
	    } else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/11.mp3 action play");
	    }
            break;
          case SDLK_SEMICOLON:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/02.mp3 action play");
	    } else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/10.mp3 action play");
	    }
            break;
          case SDLK_COMMA:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/01.mp3 action play");
	    } else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/09.mp3 action play");
	    }
            break;
          case SDLK_EXCLAIM:
             if (!(mod & COMPATIBLE_KMOD_CTRL)) {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/04.mp3 action play");
	    } else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/audio/12.mp3 action play");
	    }
            break;
          case SDLK_EQUALS:
            if (mod & COMPATIBLE_KMOD_CTRL) {
	      app->commander->execute_command( "zoom auto in");
	      app->commander->execute_command( "zoom fov 0.5 duration 5");
            } else {
              // here we help script recorders by selecting the right type of zoom option
              // based on current settings of manual or full auto zoom
              if(core->getFlagManualAutoZoom()) app->commander->execute_command( "zoom auto in manual 1");
              else { 
		app->commander->execute_command( "zoom auto in");
	        //app->commander->execute_command( "zoom fov 2 duration 5");
	      }
            }
            break;
          case SDLK_RIGHTPAREN:
            if (mod & COMPATIBLE_KMOD_CTRL) {
	      app->commander->execute_command( "zoom auto in");
	      app->commander->execute_command( "zoom fov 60 duration 5");
            } else {
              if(core->getFlagManualAutoZoom()) app->commander->execute_command( "zoom auto out manual 1");
			  else app->commander->execute_command( "zoom auto initial");
	    }
	    break;
          case SDLK_v:
            app->commander->execute_command( "flag show_tui_datetime toggle");
              // keep these in sync.  Maybe this should just be one flag.
              //if(FlagShowTuiDateTime) app->commander->execute_command( "flag show_tui_short_obj_info on");
              //else app->commander->execute_command( "flag show_tui_short_obj_info off");
            break;
          case SDLK_RETURN:
            if (mod & COMPATIBLE_KMOD_CTRL) {
            core->toggleMountMode();
	    }
            break;
          case SDLK_F1:
		if (mod & COMPATIBLE_KMOD_CTRL) {
     //app->commander->execute_command( "script filename ~/.nightshade/fscripts/01.sts action play");
		app->scripts->play_script(CDIR2+"fscripts/01.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/01.avi action play");
		}
          break;
          case SDLK_F2:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/02.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/02.avi action play");
		}
          break;
          case SDLK_F3:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/03.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/03.avi action play");
		}
            break;
          case SDLK_F4:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/04.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/04.avi action play");
		}
            break;
          case SDLK_F5:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/05.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/05.avi action play");
		}
            break;
          case SDLK_F6:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/06.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/06.avi action play");
		}
            break;
          case SDLK_F7:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/07.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/07.avi action play");
		}
            break;
          case SDLK_F8:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/08.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/08.avi action play");
		}
            break;
          case SDLK_F9:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/09.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/09.avi action play");
		}
            break;
          case SDLK_F10:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/10.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/10.avi action play");
		}
            break;
          case SDLK_F11:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/11.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/11.avi action play");
		}
            break;
          case SDLK_F12:
		if (mod & COMPATIBLE_KMOD_CTRL) {
		app->scripts->play_script(CDIR2+"fscripts/12.sts", CDIR2+"fscripts/");
		} else {
     app->commander->execute_command( "external_viewer filename ~/.nightshade/videos/12.avi action play");
		}
            break;
          default:
            break;
        }
	}
	return retVal;
}

