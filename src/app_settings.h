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

/*
 * Provides a single point of access to common application settings such as data
 * and configuration directories. The singleton is initialized in main.cpp and
 * is henceforth available from anywhere via the Instance method.
 *
 */

#include <string>
#include "init_parser.h"
#include <time.h>
#include <nshade_shared_memory.h>

#pragma once

// Stub implementations of functions that don't exist on windows
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	time_t timegm(struct tm*);
#else
	struct TIME_ZONE_INFORMATION {
		long Bias;
		long DaylightBias;
	};
	int GetTimeZoneInformation( TIME_ZONE_INFORMATION* );
#endif

class AppSettings {

public:
	static AppSettings* Instance();
	static void Init(string, string, string);

	// Obtains config.ini settings. Caller must allocate InitParser.
	void loadAppSettings( InitParser* const ) const;

	// Runtime environment queries
	const string getConfigFile(void) const;
	const string getConfigDir(void) const;
	const string getDataRoot(void) const;
	const string getlDir(void) const;

	// Platform query functions. These should be preferred over sprinkling
	// preprocessor statements throughout the code.
	const bool OSX(void) const;
	const bool Unix(void) const;
	const bool Windows(void) const;
	const bool Digitarium( void ) const;

private:
	AppSettings();
	AppSettings( string, string, string );
	static AppSettings* m_instance;
	const string m_configDir;
	const string m_dataRoot;
	const string m_lDir;

};

