/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2004-2006 Robert Spearman
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
#include <fstream>
#include <dirent.h>
#include "sky_localizer.h"
#include "translator.h"
#include "init_parser.h"
#include <cassert>
#include "shared_data.h"

REGISTER(CultureRecord);

SkyLocalizer::SkyLocalizer(const string& cultureDir)
{
	struct dirent *entryp;
	DIR *dp;

	if ((dp = opendir(cultureDir.c_str())) == NULL) {
		cerr << "Unable to find culture directory:" << cultureDir << endl;
		assert(0);
	}

	while ((entryp = readdir(dp)) != NULL) {
		string tmp = entryp->d_name;
		string tmpfic = cultureDir+"/"+tmp+"/info.ini";
		FILE* fic = fopen(tmpfic.c_str(), "r");
		if (fic) {
			InitParser conf;
			conf.load(tmpfic);
			dirToNameEnglish[tmp] = conf.get_str("info:name");
			if( SharedData::Instance()->DB() ) {
				CultureRecord rec( dirToNameEnglish[tmp].c_str(), tmp.c_str() );
				insert(rec);
			}
			// cout << tmp << " : " << dirToNameEnglish[tmp] << endl;
			fclose(fic);
		}
	}

	if( SharedData::Instance()->DB() ) {
		SharedData::Instance()->DB()->commit();
	}
	closedir(dp);
}

SkyLocalizer::~SkyLocalizer(void) {
	if( SharedData::Instance()->DB() ) {
		dbCursor<CultureRecord> cursor(dbCursorForUpdate);
		if( cursor.select() )
			cursor.removeAll();
		SharedData::Instance()->DB()->commit();
	}
}

//! returns newline delimited list of human readable culture names in english
string SkyLocalizer::getSkyCultureListEnglish(void)
{
	string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		cultures += iter->second + "\n";
	}
	return cultures;
}

//! returns newline delimited list of human readable culture names translated to current locale
string SkyLocalizer::getSkyCultureListI18(void)
{
	string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		if (iter != dirToNameEnglish.begin()) cultures += "\n";
		cultures += _(iter->second);
	}
	//wcout << cultures << endl;
	return cultures;
}

//! returns newline delimited hash of human readable culture names translated to current locale
//! and the directory names
string SkyLocalizer::getSkyCultureHash(void)
{
	string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {

		// weed out invalid hash entries from invalid culture lookups in hash
		// TODO how to keep hash clean in the first place
		if (iter->second == "") continue;

		cultures += _(iter->second);
		cultures += string("\n") + iter->first + "\n";
	}
	// wcout << cultures << endl;
	return cultures;
}


string SkyLocalizer::directoryToSkyCultureEnglish(const string& directory)
{
	return dirToNameEnglish[directory];
}

string SkyLocalizer::directoryToSkyCultureI18(const string& directory)
{
	return _(dirToNameEnglish[directory]);
}

string SkyLocalizer::skyCultureToDirectory(const string& cultureName)
{
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		if (_(iter->second) == cultureName) return iter->first;
	}
	return "";
}
