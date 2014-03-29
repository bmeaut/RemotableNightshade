/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
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

#include <string>
#include <iostream>
#include <cstdlib>

#include "SDL.h"

#include "nightshade.h"
#include "app.h"
#include "utility.h"
#include "app_settings.h"
#include "sdl_facade.h"
#include "signals.h"

#ifndef WIN32
#include <X11/Xlib.h>
#include <sys/stat.h>
#endif

#ifdef WIN32
#include <Shlobj.h>
#endif


#include "mongoose-cpp/mwebapi.h"

using namespace std;

string CDIR;	// Config Directory
string LDIR;	// Locale dir for PO translation files
string DATA_ROOT;	// Data Root Directory

static bool firstRun = false;

// Print a beautiful console logo !!
void drawIntro(void)
{
	cout << "\n[ "<< APP_NAME << " - " << EDITION << " Edition\n";
	cout << "[ Copyright (C) 2003-2011 Digitalis Education Solutions, Inc. et al.\n";
	cout << "[ Copyright (C) 2000-2008 Fabien Chereau et al.\n";
	cout << "[ http://NightshadeSoftware.org\n\n";
};


// Display usage in the console
void usage(char **argv)
{
	cout << _("Usage: %s [OPTION] ...\n -v, --version          Output version information and exit.\n -h, --help             Display this help and exit.\n");
}



// Check command line arguments
void check_command_line(int argc, char **argv)
{
	if (argc == 2) {
		if (!(strcmp(argv[1],"--version") && strcmp(argv[1],"-v"))) {
			cout << APP_NAME << endl;
			exit(0);
		}
		if (!(strcmp(argv[1],"--help") && strcmp(argv[1],"-h"))) {
			usage(argv);
			exit(0);
		}
	}

	if (argc > 1) {
		cout << _("%s: Bad command line argument(s)\n");
		cout << _("Try `%s --help' for more information.\n");
		exit(1);
	}
}


// Set the data, textures, and config directories in core.global : test the default
// installation dir and try to find the files somewhere else if not found there
// This enable to launch from the local directory without installing it
void setDirectories(const char* executableName)
{
	// The variable CONFIG_DATA_DIR must have been set by the configure script
	// Its value is the dataRoot directory, ie the one containing data/ and textures/
	FILE * tempFile = NULL;

	// Check the presence of a file in possible data directories and set the
	// dataRoot string if the directory was found.
	tempFile = fopen("./data/ssystem.ini","r");
	if (tempFile) {
		DATA_ROOT = ".";
	} else {
		tempFile = fopen((string(CONFIG_DATA_DIR) + "/data/ssystem.ini").c_str(),"r");
		if (tempFile) {
			DATA_ROOT = string(CONFIG_DATA_DIR);
		} else {
			tempFile = fopen("../data/ssystem.ini","r");
			if (tempFile) {
				DATA_ROOT = "..";
			} else {
				// Failure....
				cerr << "ERROR : I can't find the datas directories in :\n"  << CONFIG_DATA_DIR <<
				     "/ nor in ./ nor in ../\nYou may fully install the software (type \"make install\" on POSIX systems)\n" <<
				     "or launch the application from the " << APP_LOWER_NAME << " package directory." << endl;
				exit(-1);
			}
		}
	}
	fclose(tempFile);
	tempFile = NULL;

	// We now have a valid dataRoot directory, we can then set the data and textures dir
	LDIR = LOCALEDIR;

	// If the system is windows then place config files (files the user needs runtime write 
	// access to) in the user's application data dir. Otherwise startup or conf file updates
	// will fail as non-elevated user on Vista and Win7
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	char szPath[MAX_PATH]; 
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath);

	CDIR = string(szPath) + "\\nightshade\\";
	LDIR = DATA_ROOT + "\\data\\locale";

	if ((tempFile = fopen((CDIR + "config.ini").c_str(),"r"))) {
		fclose(tempFile);
	} else {
		// First launch for that user : set default options by copying the default files
		cerr << "Trying to create configuration file as none was found.\n";
		cerr << "Creating in " << CDIR << ".\n";
		firstRun = true;
		system( (string("mkdir \"") + CDIR + "\"").c_str() );
		copy_file(DATA_ROOT + "\\data\\default_config.ini", CDIR + "config.ini", true);
	}
#else

	// confirm locale directory or fall back to default
	// for easier testing

	tempFile = fopen((string(LDIR) + "/es/LC_MESSAGES/" + APP_LOWER_NAME + ".mo").c_str(),"r");
	if (!tempFile) {
		// Fallback I
		LDIR = "/usr/share/locale";
	        tempFile = fopen((string(LDIR) + "/es/LC_MESSAGES/" + APP_LOWER_NAME + ".mo").c_str(),"r");

		if (!tempFile) {
		  // Fallback II
		  LDIR = "/usr/local/share/locale";
		  tempFile = fopen((string(LDIR) + "/es/LC_MESSAGES/" + APP_LOWER_NAME + ".mo").c_str(),"r");
		}
	}

	if(tempFile) {
	  fclose(tempFile);
	  tempFile = NULL;
	  cout << "LOCALE DIRECTORY: " << LDIR << endl;
	} else {
	  cout << "WARNING: No locale directory found." << endl;
	}


	// Just an indication if we are on unix/linux that we use local data files
	if (DATA_ROOT != string(CONFIG_DATA_DIR))
		printf("> Found data files in %s : local version.\n", DATA_ROOT.c_str());

	// The problem is more complexe in the case of a unix/linux system
	// The config files are in the HOME/.APP_LOWER_NAME/ directory and this directory
	// has to be created if it doesn't exist

	// Get the user home directory
	string homeDir = getenv("HOME");
	CDIR = homeDir + "/." + APP_LOWER_NAME + "/";

	// If unix system, check if the file $HOME/.APP_LOWER_NAME/config.ini exists,
	// if not, try to create it.
	if ((tempFile = fopen((CDIR + "config.ini").c_str(),"r"))) {
		fclose(tempFile);
	} else {
		firstRun = true;
		printf("Will create config files in %s\n", CDIR.c_str());
		if ((tempFile = fopen((CDIR + "config.ini").c_str(),"w"))) {
			fclose(tempFile);
		} else {
			// Maybe the directory is not created so try to create it
			printf("Try to create directory %s\n",CDIR.c_str());
			if (mkdir(CDIR.c_str(), 0777) < 0) {
				cerr << "Couldn't create directory " << CDIR << "." << endl;
			}

			if ((tempFile = fopen((CDIR + "config.ini").c_str(),"w"))) {
				fclose(tempFile);
			} else {
				cerr << "Can't create the file "<< CDIR << "config.ini\nIf the directory " <<
				     CDIR << " is missing please create it by hand.\nIf not, check that you have access to " <<
				     CDIR << endl;
				exit(-1);
			}
		}

		// First launch for that user : set default options by copying the default files
		copy_file(DATA_ROOT + "/data/default_config.ini", CDIR + "config.ini", true);
	}
#endif	// Unix system

	// hack
	Utility::setDataRoot( string(DATA_ROOT) );
}


// Main procedure
int main(int argc, char **argv)
{


	// Used for getting system date formatting
	setlocale(LC_TIME, "");

	// Check the command line
	check_command_line(argc, argv);

	// Print the console logo..
	drawIntro();

	// Find what are the main Data, Textures and Config directories
	setDirectories(argv[0]);
	
	printf( "CONFIG DIR: %s\n", CDIR.c_str() );
	AppSettings::Init(CDIR, DATA_ROOT, LDIR);
	InitParser conf;
	AppSettings* ini = AppSettings::Instance();
	ini->loadAppSettings( &conf );
	
	SDLFacade* sdl = new SDLFacade();
	sdl->initSDL();

	Uint16 curW, curH;
	bool fullscreen;

	// Always force windowed mode and current resolution on digitarium systems.
	if( ini->Digitarium() ) {
		sdl->getCurrentRes( &curW, &curH );
		fullscreen = false;
	}
	else {
		// Trystan: If we're running for the first time, or config.ini DNE, then update config.ini with current fullscreen resolution.
		// Always force windowed mode on digitarium systems.
		if( firstRun ) {
			sdl->getCurrentRes( &curW, &curH );
			conf.set_int("video:screen_w", curW);
			conf.set_int("video:screen_h", curH);
			conf.save( ini->getConfigFile() );
		}
		curW = conf.get_int("video:screen_w");
		curH = conf.get_int("video:screen_h");
		fullscreen = conf.get_boolean("video:fullscreen");
	}

	// Trystan: SDL surface 'must' be created prior to any OpenGL calls or will crash on OSX
	sdl->createSurface(curW, curH, conf.get_int("video:bbp_mode"), fullscreen, DATA_ROOT + "/data/icon.bmp");

	App* app = new App( sdl );

	// Initialize the Webapi & start it.
	MWebapi ms(*app);
	ms.start();
	cout << ms.isRunning() << endl;

	// Register custom suspend and term signal handers
	ISignals* signalObj = ISignals::Create(app);
	signalObj->Register( SIGTSTP, ISignals::NSSigTSTP );
	signalObj->Register( SIGTERM, ISignals::NSSigTERM );
	signalObj->Register( SIGINT, ISignals::NSSigTERM );
	signalObj->Register( SIGQUIT, ISignals::NSSigTERM );

	app->init();

	app->startMainLoop();

	ms.debug();

	// Clean memory
	ms.stop();
	delete app;
	delete sdl;
	delete signalObj;
	return 0;
}

