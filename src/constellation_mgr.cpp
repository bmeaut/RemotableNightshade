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

// Class used to manage group of constellation

#include <iostream>
#include <fstream>
#include <vector>

#include "constellation_mgr.h"
#include "constellation.h"
#include "hip_star_mgr.h"
#include "hip_star.h"
#include "utility.h"
#include <nshade_state.h>

// constructor which loads all data from appropriate files
ConstellationMgr::ConstellationMgr(HipStarMgr *_hip_stars) :
		asterFont(NULL),
		hipStarMgr(_hip_stars),
		flagNames(0),
		flagLines(0),
		flagArt(0),
		flagBoundaries(0)
{
	assert(hipStarMgr);
	isolateSelected = false;
}

ConstellationMgr::~ConstellationMgr()
{
	vector<Constellation *>::iterator iter;
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		for (iter = asterisms.begin(); iter != asterisms.end(); iter++) {
			dbQuery q;
			q = "englishName=",(*iter)->englishName.c_str(),"and type=",ObjectRecord::OBJECT_CONSTELLATION;
			if( cursor.select(q) ) {
				cursor.remove();
			}
			delete(*iter);
		}
		SharedData::Instance()->DB()->commit();
	}
	else {
		for (iter = asterisms.begin(); iter != asterisms.end(); iter++)
			delete(*iter);
	}

	if (asterFont) delete asterFont;
	asterFont = NULL;

	vector<vector<Vec3f> *>::iterator iter1;
	for (iter1 = allBoundarySegments.begin(); iter1 != allBoundarySegments.end(); ++iter1) {
		delete (*iter1);
	}
	allBoundarySegments.clear();
}

void ConstellationMgr::setFlagGravityLabel(bool g)
{
	Constellation::gravityLabel = g;
}

void ConstellationMgr::setLineColor(const Vec3f& c)
{
	Constellation::lineColor = c;
	SettingsState state;
	state.m_state.const_lines[0] = c[0];
	state.m_state.const_lines[1] = c[1];
	state.m_state.const_lines[2] = c[2];
	SharedData::Instance()->Settings( state );
}
Vec3f ConstellationMgr::getLineColor() const
{
	return Constellation::lineColor;
}

void ConstellationMgr::setBoundaryColor(const Vec3f& c)
{
	Constellation::boundaryColor = c;
	SettingsState state;
	state.m_state.const_bounds[0] = c[0];
	state.m_state.const_bounds[1] = c[1];
	state.m_state.const_bounds[2] = c[2];
	SharedData::Instance()->Settings( state );
}
Vec3f ConstellationMgr::getBoundaryColor() const
{
	return Constellation::boundaryColor;
}

void ConstellationMgr::setLabelColor(const Vec3f& c)
{
	Constellation::labelColor = c;
	SettingsState state;
	state.m_state.const_names[0] = c[0];
	state.m_state.const_names[1] = c[1];
	state.m_state.const_names[2] = c[2];
	SharedData::Instance()->Settings( state );

}
Vec3f ConstellationMgr::getLabelColor() const
{
	return Constellation::labelColor;
}

void ConstellationMgr::setArtColor(const Vec3f& c)
{
	Constellation::artColor = c;
	SettingsState state;
	state.m_state.const_art[0] = c[0];
	state.m_state.const_art[1] = c[1];
	state.m_state.const_art[2] = c[2];
	SharedData::Instance()->Settings( state );
}

Vec3f ConstellationMgr::getArtColor() const
{
	return Constellation::artColor;
}

void ConstellationMgr::setFont(float font_size, const string& ttfFileName)
{
	if (asterFont) delete asterFont;
	asterFont = new s_font(font_size, ttfFileName);
	assert(asterFont);
}

// Load line and art data from files
int ConstellationMgr::loadLinesAndArt(const string &skyCultureDir, LoadingBar& lb)
{

	string fileName = skyCultureDir + "/constellationship.fab";
	string artfileName = skyCultureDir + "/constellationsart.fab";
	string boundaryfileName = skyCultureDir + "/boundaries.dat";

	std::ifstream inf(fileName.c_str());

	if (!inf.is_open()) {
		printf("Can't open constellation data file %s\n", fileName.c_str());
		return -1;
	}

	// delete existing data, if any
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "type=",ObjectRecord::OBJECT_CONSTELLATION;
		if( cursor.select(q) )
			cursor.removeAllSelected();
		SharedData::Instance()->DB()->commit();
	}

	vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		delete(*iter);
	}
	asterisms.clear();
	selected.clear();

	Constellation *cons = NULL;

	string record;
	int line=0;
	while (!inf.eof() && std::getline(inf, record)) {
		line++;
		if (record.size()!=0 && record[0]=='#')
			continue;
		cons = new Constellation;

		if (cons->read(record, hipStarMgr)) {
			asterisms.push_back(cons);
		} else {
			cerr << "ERROR on line " << line << " of " << fileName.c_str() << endl;
			delete cons;
		}
	}
	inf.close();

	// Set current states
	setFlagArt(flagArt);
	setFlagLines(flagLines);
	setFlagNames(flagNames);
	setFlagBoundaries(flagBoundaries);


	FILE *fic = fopen(artfileName.c_str(), "r");
	if (!fic) {
//		cerr << "Can't open " << artfileName.c_str() << endl;
		return 0; // no art, but still loaded constellation data
	}
	fclose(fic);

	// Read the constellation art file with the following format :
	// ShortName texture_file x1 y1 hp1 x2 y2 hp2
	// Where :
	// shortname is the international short name (i.e "Lep" for Lepus)
	// texture_file is the graphic file of the art texture
	// x1 y1 are the x and y texture coordinates in pixels of the star of hipparcos number hp1
	// x2 y2 are the x and y texture coordinates in pixels of the star of hipparcos number hp2
	// The coordinate are taken with (0,0) at the top left corner of the image file
	string shortname;
	string texfile;
	unsigned int x1, y1, x2, y2, x3, y3, hp1, hp2, hp3;
	float fx1, fy1, fx2, fy2, fx3, fy3; // read floats to allow proportional image points to allow image sizes to vary as needed
	int texW, texH; // art texture dimensions

	// Read in constellation art information
	// Note: Stellarium 0.10.3 allows more than 3 alignment points, here only first 3 are used.

	ifstream artFile(artfileName.c_str());
	if (!artFile.is_open()) {
		cerr << "Can't open file" << artFile << endl;
		return 0;
	}

	while (!artFile.eof() && std::getline(artFile, record)) {

		if ( record != "") {
			stringstream in(record);
			in >> shortname >> texfile;

			if(in.fail()) {
				cerr << "Error parsing constellation art record:\n" << record << endl;
				continue;
			}

			// TODO add better error checking
			if(shortname!="" && shortname[0]!='#') {
				in >> fx1 >> fy1 >> hp1;
				in >> fx2 >> fy2 >> hp2;
				in >> fx3 >> fy3 >> hp3;
			} else {
				continue;
			}

			cons = NULL;
			cons = findFromAbbreviation(shortname);
			if (!cons) {
				// save on common error delay
				cerr << "ERROR : Can't find constellation called : " << shortname << endl;
			} else {

				// Try local sky culture directory for texture images first
				string localFile = skyCultureDir + "/" + texfile;
				FILE *ftest = fopen(localFile.c_str(), "r");
				if (!ftest) {
					// Load from application texture directory
					cons->art_tex = new s_texture(texfile, TEX_LOAD_TYPE_PNG_BLEND1, true);  // use mipmaps
				} else {
					// Load from local directory
					fclose(ftest);
					cons->art_tex = new s_texture(true, localFile, TEX_LOAD_TYPE_PNG_BLEND1, true);  // use mipmaps
				}

				if(cons->art_tex->getID() == 0) continue;  // otherwise no texture

				cons->art_tex->getDimensions(texW, texH);

				// support absolute and proportional image coordinates
				(fx1>1) ? x1=(unsigned int)fx1 : x1=(unsigned int)(texW*fx1);
				(fy1>1) ? y1=(unsigned int)fy1 : y1=(unsigned int)(texH*fy1);

				(fx2>1) ? x2=(unsigned int)fx2 : x2=(unsigned int)(texW*fx2);
				(fy2>1) ? y2=(unsigned int)fy2 : y2=(unsigned int)(texH*fy2);

				(fx3>1) ? x3=(unsigned int)fx3 : x3=(unsigned int)(texW*fx3);
				(fy3>1) ? y3=(unsigned int)fy3 : y3=(unsigned int)(texH*fy3);

				Vec3f s1 = hipStarMgr->searchHP(hp1)->getObsJ2000Pos(0);
				Vec3f s2 = hipStarMgr->searchHP(hp2)->getObsJ2000Pos(0);
				Vec3f s3 = hipStarMgr->searchHP(hp3)->getObsJ2000Pos(0);

				// To transform from texture coordinate to 2d coordinate we need to find X with XA = B
				// A formed of 4 points in texture coordinate, B formed with 4 points in 3d coordinate
				// We need 3 stars and the 4th point is deduced from the other to get an normal base
				// X = B inv(A)
				Vec3f s4 = s1 + ((s2 - s1) ^ (s3 - s1));
				Mat4f B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
				Mat4f A(x1, texH - y1, 0.f, 1.f, x2, texH - y2, 0.f, 1.f, x3, texH - y3, 0.f, 1.f, x1, texH - y1, texW, 1.f);
				Mat4f X = B * A.inverse();

				cons->art_vertex[0] = Vec3f(X * Vec3f(0, 0, 0));
				cons->art_vertex[1] = Vec3f(X * Vec3f(texW / 2, 0, 0));
				cons->art_vertex[2] = Vec3f(X * Vec3f(texW / 2, texH / 2, 0));
				cons->art_vertex[3] = Vec3f(X * Vec3f(0, texH / 2, 0));
				cons->art_vertex[4] = Vec3f(X * Vec3f(texW, 0, 0));
				cons->art_vertex[5] = Vec3f(X * Vec3f(texW, texH / 2, 0));
				cons->art_vertex[6] = Vec3f(X * Vec3f(texW, texH, 0));
				cons->art_vertex[7] = Vec3f(X * Vec3f(texW / 2 + 0, texH, 0));
				cons->art_vertex[8] = Vec3f(X * Vec3f(0, texH, 0));

			}
		}
	}
	artFile.close();

	loadBoundaries(boundaryfileName);

	return 0;
}


void ConstellationMgr::draw(Projector * prj, Navigator * nav) const
{
	prj->set_orthographic_projection();
	draw_lines(prj);
	draw_names(prj);
	draw_art(prj, nav);
	drawBoundaries(prj);
	prj->reset_perspective_projection();
}

// Draw constellations art textures
void ConstellationMgr::draw_art(Projector * prj, Navigator * nav) const
{
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->draw_art_optim(prj, nav);
	}

	glDisable(GL_CULL_FACE);
}

// Draw constellations lines
void ConstellationMgr::draw_lines(Projector * prj) const
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->draw_optim(prj);
	}
}

// Draw the names of all the constellations
void ConstellationMgr::draw_names(Projector * prj) const
{
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glBlendFunc(GL_ONE, GL_ONE);
	// if (draw_mode == DM_NORMAL) glBlendFunc(GL_ONE, GL_ONE);
	// else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // charting

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); iter++) {
		// Check if in the field of view
		if (prj->project_j2000_check((*iter)->XYZname, (*iter)->XYname))
			(*iter)->draw_name(asterFont, prj);
	}
}

Constellation *ConstellationMgr::is_star_in(const Object &s) const
{
	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		// Check if the star is in one of the constellation
		if ((*iter)->is_star_in(s))
			return (*iter);
	}
	return NULL;
}

Constellation *ConstellationMgr::findFromAbbreviation(const string & abbreviation) const
{
	// search in uppercase only
	string tname = abbreviation;
	transform(tname.begin(), tname.end(), tname.begin(),::toupper);

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		if ((*iter)->abbreviation == tname)
			return (*iter);
	}
	return NULL;
}


/**
 * @brief Read constellation names from the given file
 * @param namesFile Name of the file containing the constellation names in english
 */
void ConstellationMgr::loadNames(const string& namesFile)
{
	// Constellation not loaded yet
	if (asterisms.empty()) return;

	vector < Constellation * >::const_iterator iter;
	// clear previous names
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
			dbQuery q;
			q = "englishName=",(*iter)->englishName.c_str(),"and type=",ObjectRecord::OBJECT_CONSTELLATION;
			if( cursor.select(q) )
				cursor.remove();

			(*iter)->englishName.clear();
		}
		SharedData::Instance()->DB()->commit();
	}
	else {
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->englishName.clear();
	}

	// read in translated common names from file
	ifstream commonNameFile(namesFile.c_str());
	if (!commonNameFile.is_open()) {
		cerr << "Can't open file" << namesFile << endl;
		return;
	}

	// find matching constellation and update name
	string record, tmp, tmp2, ename;
	bool quotes;
	bool done=false;
	string tmpShortName;
	Constellation *aster;
	while (!commonNameFile.eof() && std::getline(commonNameFile, record)) {

		if ( record != "") {
			stringstream in(record);
			in >> tmpShortName;

			//	cout << "working on short name " << tmpShortName << endl;

			aster = findFromAbbreviation(tmpShortName);
			if (aster != NULL) {
				// Read the names in english
				// Whitespace delimited with optional quotes to be mostly compatible with Stellarium 0.10.3 format
				in >> tmp;

				done = false;
				if(tmp[0]=='"') {
					quotes=true;

					if(tmp[tmp.length()-1]=='"') {
						ename = tmp.substr(1,tmp.length()-2);
						done=true;
					} else ename = tmp.substr(1,tmp.length()-1);
				} else {
					quotes=false;
					ename = tmp;
				}

				while(!done && in >> tmp) {

					if(quotes && tmp[tmp.length()-1]=='"') {
						tmp2 = tmp.substr(0, tmp.length()-2);
						ename += " " + tmp2;
						done = true;
					} else {
						ename += " " + tmp;
					}
				}

				aster->englishName = ename;
				//aster->englishName =  record.substr(tmpShortName.length()+1,record.length()).c_str();
				if( SharedData::Instance()->DB() ) {
					ObjectRecord rec( ename.c_str(), ename.c_str(), tmpShortName.c_str(), ObjectRecord::OBJECT_CONSTELLATION );
					insert(rec);
				}
			}
		}
	}

	if( SharedData::Instance()->DB() ) {
		SharedData::Instance()->DB()->commit();
	}

	commonNameFile.close();

}

//! @brief Update i18 names from english names according to current locale
//! The translation is done using gettext with translated strings defined in translations.h
void ConstellationMgr::translateNames(Translator& trans)
{
	vector < Constellation * >::const_iterator iter;
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
			(*iter)->nameI18 = trans.translateUTF8((*iter)->englishName.c_str());

			dbQuery q;
			q = "englishName=",(*iter)->englishName.c_str(),"and type=",ObjectRecord::OBJECT_CONSTELLATION;
			if( cursor.select(q) ) {
				cursor->nameI18 = (*iter)->nameI18.c_str();
				cursor.update();
			}
		}

		SharedData::Instance()->DB()->commit();
	}
	else {
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->nameI18 = trans.translateUTF8((*iter)->englishName.c_str());
	}

	if(asterFont) asterFont->clearCache();  // remove cached strings
}

// update faders
void ConstellationMgr::update(int delta_time)
{
	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->update(delta_time);
	}
}


void ConstellationMgr::setArtIntensity(float _max)
{
	artMaxIntensity = _max;
	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->art_fader.set_max_value(_max);
}

void ConstellationMgr::setArtFadeDuration(float duration)
{
	artFadeDuration = duration;
	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->art_fader.set_duration((int) (duration * 1000.f));
}


void ConstellationMgr::setFlagLines(bool b)
{
	flagLines = b;
	ObjectsState state;
	state.m_state.const_lines = b;
	SharedData::Instance()->Objects(state);

	if (selected.begin() != selected.end()  && isolateSelected) {
		vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagLines(b);
	} else {
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagLines(b);
	}
}

void ConstellationMgr::setFlagBoundaries(bool b)
{
	flagBoundaries = b;
	ObjectsState state;
	state.m_state.const_boundary = b;
	SharedData::Instance()->Objects(state);

	if (selected.begin() != selected.end() && isolateSelected) {
		vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagBoundaries(b);
	} else {
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagBoundaries(b);
	}
}

void ConstellationMgr::setFlagArt(bool b)
{
	flagArt = b;
	ObjectsState state;
	state.m_state.const_art = b;
	SharedData::Instance()->Objects(state);

	if (selected.begin() != selected.end() && isolateSelected) {
		vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagArt(b);
	} else {
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagArt(b);
	}
}


void ConstellationMgr::setFlagNames(bool b)
{
	flagNames = b;
	ObjectsState state;
	state.m_state.const_labels = b;
	SharedData::Instance()->Objects(state);

	if (selected.begin() != selected.end() && isolateSelected) {
		vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagName(b);
	} else {
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagName(b);
	}
}

Object ConstellationMgr::getSelected(void) const
{
	return *selected.begin();  // TODO return all or just remove this method
}

//! Define which constellation is selected from its abbreviation
void ConstellationMgr::setSelected(const string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != NULL) setSelectedConst(c);

}

//! Define which constellation to unselect from its abbreviation
void ConstellationMgr::unsetSelected(const string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != NULL) unsetSelectedConst(c);
}


//! Define which constellation is selected and return brightest star
ObjectBaseP ConstellationMgr::setSelectedStar(const string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != NULL) {
		setSelectedConst(c);
		return c->getBrightestStarInConstellation();
	}
	return NULL;
}



void ConstellationMgr::setSelectedConst(Constellation * c)
{

	// update states for other constellations to fade them out
	if (c != NULL) {

		selected.push_back(c);

		// Propagate current settings to newly selected constellation
		c->setFlagLines(getFlagLines());
		c->setFlagName(getFlagNames());
		c->setFlagArt(getFlagArt());
		c->setFlagBoundaries(getFlagBoundaries());

		if (isolateSelected) {
			vector < Constellation * >::const_iterator iter;
			for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {

				bool match = 0;
				vector < Constellation * >::const_iterator s_iter;
				for (s_iter = selected.begin(); s_iter != selected.end(); ++s_iter)
					if ( (*iter)==(*s_iter) ) {
						match=true; // this is a selected constellation
						break;
					}

				if (!match) {
					// Not selected constellation
					(*iter)->setFlagLines(false);
					(*iter)->setFlagName(false);
					(*iter)->setFlagArt(false);
					(*iter)->setFlagBoundaries(false);
				}
			}
			Constellation::singleSelected = true;  // For boundaries
		} else {
			Constellation::singleSelected = false; // For boundaries
		}
	} else {
		if (selected.begin() == selected.end()) return;

		// Otherwise apply standard flags to all constellations
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
			(*iter)->setFlagLines(getFlagLines());
			(*iter)->setFlagName(getFlagNames());
			(*iter)->setFlagArt(getFlagArt());
			(*iter)->setFlagBoundaries(getFlagBoundaries());
		}

		// And remove all selections
		selected.clear();


	}
}

// - add to svn 20070112
//! Remove constellation from selected list
void ConstellationMgr::unsetSelectedConst(Constellation * c)
{
	vector < Constellation * >::iterator iter;
	for (iter = selected.begin(); iter != selected.end(); ++iter) {
		if (c == (*iter)) {
			//	  cout << "matched constellation to remove from selected list\n";
			selected.erase(iter);
			// stay at same location next time through
			iter--;
		}
	}

	// if nothing is selected now, send out current settings to all constellations
	if (selected.begin() == selected.end()) {

		// Otherwise apply standard flags to all constellations
		vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
			(*iter)->setFlagLines(getFlagLines());
			(*iter)->setFlagName(getFlagNames());
			(*iter)->setFlagArt(getFlagArt());
			(*iter)->setFlagBoundaries(getFlagBoundaries());
		}

	} else {

		// just this one constellation was unselected so reset flags
		c->setFlagLines(false);
		c->setFlagName(false);
		c->setFlagArt(false);
		c->setFlagBoundaries(false);
	}

}


// Load from file
bool ConstellationMgr::loadBoundaries(const string& boundaryFile)
{
	Constellation *cons = NULL;
	unsigned int i, j;

	vector<vector<Vec3f> *>::iterator iter;
	for (iter = allBoundarySegments.begin(); iter != allBoundarySegments.end(); ++iter) {
		delete (*iter);
	}
	allBoundarySegments.clear();

	// Modified boundary file by Torsten Bronger with permission
	// http://pp3.sourceforge.net

	ifstream dataFile;
	dataFile.open(boundaryFile.c_str());
	if (!dataFile.is_open()) {
//        cerr << "Boundary file " << boundaryFile << " not found" << endl;
		return false;
	}

	cout << "Loading Constellation boundary data...";

	float DE, RA;
	float oDE, oRA;
	Vec3f XYZ;
	unsigned num, numc;
	vector<Vec3f> *points = NULL;
	string consname;
	i = 0;
	while (!dataFile.eof()) {
		points = new vector<Vec3f>;

		num = 0;
		dataFile >> num;
		if (num == 0) continue; // empty line

		for (j=0; j<num; j++) {
			dataFile >> RA >> DE;

			oRA =RA;
			oDE= DE;

			RA*=M_PI/12.;     // Convert from hours to rad
			DE*=M_PI/180.;    // Convert from deg to rad

			// Calc the Cartesian coord with RA and DE
			sphe_to_rect(RA,DE,XYZ);
			points->push_back(XYZ);
		}

		// this list is for the de-allocation
		allBoundarySegments.push_back(points);

		dataFile >> numc;
		// there are 2 constellations per boundary

		for (j=0; j<numc; j++) {
			dataFile >> consname;
			// not used?
			if (consname == "SER1" || consname == "SER2") consname = "SER";

			cons = findFromAbbreviation(consname);
			if (!cons) {
// save on common error delay
				// cout << "ERROR : Can't find constellation called : " << consname << endl;
			} else cons->isolatedBoundarySegments.push_back(points);
		}

		if (cons) cons->sharedBoundarySegments.push_back(points);
		i++;

	}
	dataFile.close();
	cout << "(" << i << " segments loaded)" << endl;
	delete points;

	return true;
}

// Draw constellations lines
void ConstellationMgr::drawBoundaries(Projector * prj) const
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glLineStipple(2, 0x3333);
	glEnable(GL_LINE_STIPPLE);

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->draw_boundary_optim(prj);
	}
	glDisable(GL_LINE_STIPPLE);
}

///unsigned int ConstellationMgr::getFirstSelectedHP(void) {
///  if (selected) return selected->asterism[0]->get_hp_number();
///  return 0;
///}

//! Return the matching constellation object's pointer if exists or NULL
//! @param nameI18n The case sensistive constellation name
Object ConstellationMgr::searchByNameI18n(const string& nameI18n) const
{
	string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	vector <Constellation*>::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		string objwcap = (*iter)->nameI18;
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return *iter;
	}
	return NULL;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
vector<string> ConstellationMgr::listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		string constw = (*iter)->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}
