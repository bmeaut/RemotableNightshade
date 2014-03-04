/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Author: Trystan Larey-Williams
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

#include "app_settings.h"
#include "nightshade.h"
#include "utility.h"

// Stub implementations of functions that don't exist on windows and visa versa
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	time_t timegm(struct tm*) {
		time_t t;
		memset( &t, 0, sizeof(t) );
		return t;
	}
#else
	int GetTimeZoneInformation( TIME_ZONE_INFORMATION* info ) {
		return 0;
	}
#endif 

AppSettings* AppSettings::m_instance = NULL;

AppSettings::AppSettings( string configDir, string dataRoot, string lDir ) : m_configDir(configDir),
																			 m_dataRoot(dataRoot),
																			 m_lDir(lDir) {
}

void AppSettings::Init( string configDir, string dataRoot, string lDir ) {
	if( m_instance )
		delete m_instance;
	
	m_instance = new AppSettings( configDir, dataRoot, lDir );
}

AppSettings* AppSettings::Instance(){
	if( !m_instance )
		return new AppSettings( "", "", "" );
	else
		return m_instance;
}

const bool AppSettings::OSX(void) const {
#if defined(MACOSX)
	return true;
#else
	return false;
#endif
}

const bool AppSettings::Windows(void) const {
	#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
		return true;
	#else
		return false;
	#endif
}

const bool AppSettings::Unix(void) const {
	#if !defined(MACOSX) && !defined(WIN32) && !defined(CYGWIN) && !defined(__MINGW32__) && !defined(MINGW32)
		return true;
	#else
		return false;
	#endif
}

const bool AppSettings::Digitarium(void) const {
	#if !defined(DESKTOP)
		return true;
	#else
		return false;
	#endif
}

void AppSettings::loadAppSettings( InitParser* const conf ) const {

	conf->load(m_configDir + "config.ini");

	// Main section
	string version = conf->get_str("main:version");

	if (version!=string(VERSION)) {

		std::istringstream istr(version);
		char tmp;
		int v1 =0;
		int v2 =0;
		istr >> v1 >> tmp >> v2;

		// Config versions less than 0.6.0 are not supported, otherwise we will try to use it
		if ( v1 == 0 && v2 < 6 ) {

			// The config file is too old to try an importation
			printf("The current config file is from a version too old for parameters to be imported (%s).\nIt will be replaced by the default config file.\n", version.empty() ? "<0.6.0" : version.c_str());
			copy_file(m_dataRoot + "/data/default_config.ini", getConfigFile(), true);
			conf->load(m_configDir + "config.ini");  // Read new config!
		} else {

			cout << "Attempting to use an existing older config file." << endl;
		}
	}
}

const string AppSettings::getConfigDir(void) const {
	return m_configDir;
}

const string AppSettings::getConfigFile(void) const {
	return getConfigDir() + "config.ini";
}

const string AppSettings::getlDir(void) const {
	return m_lDir;
}

const string AppSettings::getDataRoot(void) const {
	return m_dataRoot;
}
