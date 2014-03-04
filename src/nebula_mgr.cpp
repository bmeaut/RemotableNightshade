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

// class used to manage groups of Nebulas

#include <fstream>
#include <algorithm>

#include "nebula_mgr.h"
#include "nebula.h"
#include "nightshade.h"
#include "s_texture.h"
#include "s_font.h"
#include "navigator.h"
#include "translator.h"
#include "loadingbar.h"

#define RADIUS_NEB 1.

void NebulaMgr::setLabelColor(const Vec3f& c)
{
	Nebula::label_color = c;
	SettingsState state;
	state.m_state.nebula_names[0] = c[0];
	state.m_state.nebula_names[1] = c[1];
	state.m_state.nebula_names[2] = c[2];
	SharedData::Instance()->Settings(state);
}
const Vec3f &NebulaMgr::getLabelColor(void) const
{
	return Nebula::label_color;
}
void NebulaMgr::setCircleColor(const Vec3f& c)
{
	Nebula::circle_color = c;
	SettingsState state;
	state.m_state.nebula_circles[0] = c[0];
	state.m_state.nebula_circles[1] = c[1];
	state.m_state.nebula_circles[2] = c[2];
	SharedData::Instance()->Settings(state);
}
const Vec3f &NebulaMgr::getCircleColor(void) const
{
	return Nebula::circle_color;
}
void NebulaMgr::setNebulaCircleScale(float scale)
{
	Nebula::circleScale = scale;
}
float NebulaMgr::getNebulaCircleScale(void) const
{
	return Nebula::circleScale;
}
void NebulaMgr::setFlagBright(bool b)
{
	Nebula::flagBright = b;
}
bool NebulaMgr::getFlagBright(void) const
{
	return Nebula::flagBright;
}


NebulaMgr::NebulaMgr(void) : displayNoTexture(false)
{
	nebZones = new vector<Nebula*>[nebGrid.getNbPoints()];
}

NebulaMgr::~NebulaMgr()
{
	vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		delete (*iter);
	}

	if (Nebula::tex_circle) delete Nebula::tex_circle;
	Nebula::tex_circle = NULL;

	if (Nebula::nebula_font) delete Nebula::nebula_font;
	Nebula::nebula_font = NULL;

	delete[] nebZones;
}

// read from stream
bool NebulaMgr::read(float font_size, const string& font_name, const string& catNGC, const string& catNGCNames, const string& catTextures, LoadingBar& lb)
{
	loadNGC(catNGC, lb);
	loadNGCNames(catNGCNames);
	loadTextures(catTextures, lb);

	if (!Nebula::nebula_font) Nebula::nebula_font = new s_font(font_size, font_name); // load Font
	if (!Nebula::nebula_font) {
		printf("Can't create nebulaFont\n");
		return(1);
	}

	if (!Nebula::tex_circle)
		Nebula::tex_circle = new s_texture("neb.png");   // Load circle texture

	return true;
}

// Load individual nebula for script
bool NebulaMgr::loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
						   string name, string filename, string credit, double texture_luminance_adjust, 
						   double distance)
{

	// TODO: not supporting NGC designations since not used currently

	// No duplicates
	Nebula *e = searchNebula(name, false);
	if(e) {
		if(e->isDeleteable()) {
			cout << "Warning: replacing user added nebula with name " << name << ".\n";
			removeNebula(name, false);
		} else {
			// Autohide default nebula with same name
			e->hide();
		}
	}

	e = new Nebula;

	if(!e->readTexture(ra, de, magnitude, angular_size, rotation,
					   name, filename, credit, texture_luminance_adjust, distance)) {
		cerr << "Error while creating nebula " << e->englishName << endl;
		delete e;

		return false;
		
	} else {
		
		neb_array.push_back(e);
		nebZones[nebGrid.GetNearest(e->XYZ)].push_back(e);

//		cerr << "Nebula created: " << e->englishName << endl;
		return true;
	}

}

// Clear user added nebula
string NebulaMgr::removeNebula(const string& name, bool showOriginal=true)
{
	string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	vector <Nebula*>::iterator iter;
	vector <Nebula*>::iterator iter2;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		string testName = (*iter)->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);

		// if(testName != "" ) cout << ">" << testName << "< " << endl;

		if (testName==uname) {

			if(!(*iter)->isDeleteable()) {
				if(showOriginal) (*iter)->show(); // make sure original is now visible
				continue;
			}

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ);

			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
			neb_array.erase(iter);
//			cerr << "Erased nebula " << uname << endl;

			return "";
		}
	}

	return "Requested nebula to delete not found by name.";
}

// remove all user added nebula
// and make standard nebulae visible again
string NebulaMgr::removeSupplementalNebulae() 
{

	vector<Nebula *>::iterator iter;
	vector<Nebula *>::iterator iter2;

	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {

		if (!(*iter)->isDeleteable()) {
			(*iter)->show();
		} else {

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ);

			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
					//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
			neb_array.erase(iter);
			iter--;
			// cerr << "Erased nebula " << uname << endl;
		}
	}

	return "";
}

// Draw all the Nebulae
void NebulaMgr::draw(Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness)
{
	Nebula::hints_brightness = hintsFader.getInterstate()*flagShow.getInterstate();
	Nebula::nebula_brightness = flagShow.getInterstate();

	if(!flagShow) return;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBlendFunc(GL_ONE, GL_ONE);
	// if (draw_mode == DM_NORMAL) glBlendFunc(GL_ONE, GL_ONE);
	// else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // charting

	Vec3f pXYZ;

// 	vector<Nebula *>::iterator iter;
// 	for(iter=neb_array.begin();iter!=neb_array.end();iter++)

	// Find the star zones which are in the screen
	int nbZones=0;
	// FOV is currently measured vertically, so need to adjust for wide screens
	// TODO: projector should probably use largest measurement itself
	float max_fov = MY_MAX( prj->get_fov(), prj->get_fov()*prj->getViewportWidth()/prj->getViewportHeight());
	nbZones = nebGrid.Intersect(nav->get_prec_equ_vision(), max_fov*M_PI/180.f*1.2f);
	static int * zoneList = nebGrid.getResult();

	prj->set_orthographic_projection();	// set 2D coordinate

	// Print all the stars of all the selected zones
	static vector<Nebula *>::iterator end;
	static vector<Nebula *>::iterator iter;
	Nebula* n;

	// speed up the computation of n->get_on_screen_size(prj, nav)>5:
	const float size_limit = 5.0 * (M_PI/180.0)
	                         * (prj->get_fov()/prj->getViewportHeight());

	for (int i=0; i<nbZones; ++i) {
		end = nebZones[zoneList[i]].end();
		for (iter = nebZones[zoneList[i]].begin(); iter!=end; ++iter) {

			n = *iter;

			if (!displayNoTexture && !n->hasTex()) continue;

			// improve performance by skipping if too small to see
			// TODO: skip if too faint to see
			if (n->m_angular_size>size_limit ||
			        (hintsFader.getInterstate()>0.0001 &&
			         n->mag <= getMaxMagHints())) {

// draw when partially off screen even
				//				if ( !prj->project_j2000_check(n->XYZ,n->XY) ) continue;
				//				prj->project_j2000_check(n->XYZ,n->XY);

				prj->project_j2000(n->XYZ,n->XY);

				//			  cout << "projected " << n->getEnglishName()<< endl;

				if (n->m_angular_size>size_limit) {
					if (n->hasTex()) n->draw_tex(prj, nav, eye, sky_brightness);
					else n->draw_no_tex(prj, nav, eye);
				}

				if (hintsFader.getInterstate()>0.00001 && n->mag <= getMaxMagHints()) {
					n->draw_name(prj);
					n->draw_circle(prj, nav);
				}
			}
		}
	}
	prj->reset_perspective_projection();
}

// search by name
Object NebulaMgr::search(const string& name)
{
	Nebula *n = searchNebula(name, false);
	return Object(n);
}

// search by name
Nebula *NebulaMgr::searchNebula(const string& name, bool search_hidden=false)
{

	string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	vector <Nebula*>::const_iterator iter;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		string testName = (*iter)->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);
		//		if(testName != "" ) cout << ">" << testName << "< " << endl;
		if (testName==uname  && ((*iter)->m_hidden==false || search_hidden)) return *iter;
	}

	// If no match found, try search by catalog reference

	string         cat;
	istringstream  iss_cats;
	string         n = name;
	int            num;
	bool           catfound = false;

	iss_cats.str("M NGC ARP TCP");

	while (iss_cats >> cat) {
		if ( ! n.find(cat, 0) ) {
			n.replace(0, cat.length(), "");
			catfound = true;
			break;
		}
	}

	if ( ! catfound ) {
		return NULL;
	}

	istringstream ss(n);
	ss >> num;
	if ( ss.fail() ) {
		return NULL;
	}

	// TODO: not really used, and won't search hidden
	if (cat == "M") return searchM(num);
	if (cat == "NGC") return searchNGC(num);
	if (cat == "IC") return searchIC(num);
	// if (cat == "UGC") return searchUGC(num);
	return NULL;


}


// Look for a nebulae by XYZ coords
Object NebulaMgr::search(Vec3f Pos)
{
	Pos.normalize();
	vector<Nebula *>::iterator iter;
	Nebula * plusProche=NULL;
	float anglePlusProche=0.;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if((*iter)->m_hidden==true) continue;
		if ((*iter)->XYZ[0]*Pos[0]+(*iter)->XYZ[1]*Pos[1]+(*iter)->XYZ[2]*Pos[2]>anglePlusProche) {
			anglePlusProche=(*iter)->XYZ[0]*Pos[0]+(*iter)->XYZ[1]*Pos[1]+(*iter)->XYZ[2]*Pos[2];
			plusProche=(*iter);
		}
	}
	if (anglePlusProche>RADIUS_NEB*0.999) {
		return plusProche;
	} else return NULL;
}

// Return a stl vector containing the nebulas located inside the lim_fov circle around position v
vector<Object> NebulaMgr::search_around(Vec3d v, double lim_fov) const
{
	vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * M_PI/180.);
	static Vec3d equPos;

	vector<Nebula*>::const_iterator iter = neb_array.begin();
	while (iter != neb_array.end()) {
		equPos = (*iter)->XYZ;
		equPos.normalize();
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {

			// NOTE: non-labeled nebulas are not returned!
			// Otherwise cursor select gets invisible nebulas - Rob
			if ((*iter)->getNameI18n() != "" && (*iter)->m_hidden==false) result.push_back(*iter);
		}
		iter++;
	}
	return result;
}

Nebula *NebulaMgr::searchM(unsigned int M)
{
	vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if ((*iter)->M_nb == M  && (*iter)->m_hidden==false) return (*iter);
	}

	return NULL;
}

Nebula *NebulaMgr::searchNGC(unsigned int NGC)
{
	vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if ((*iter)->NGC_nb == NGC && (*iter)->m_hidden==false) return (*iter);
	}

	return NULL;
}

Nebula *NebulaMgr::searchIC(unsigned int IC)
{
	vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if ((*iter)->IC_nb == IC  && (*iter)->m_hidden==false) return (*iter);
	}

	return NULL;
}

/*Object NebulaMgr::searchUGC(unsigned int UGC)
{
	vector<Nebula *>::iterator iter;
	for(iter=neb_array.begin();iter!=neb_array.end();iter++)
	{
		if ((*iter)->UGC_nb == UGC) return (*iter);
	}

	return NULL;
}*/

// read from stream
bool NebulaMgr::loadNGC(const string& catNGC, LoadingBar& lb)
{
	char recordstr[512];
	unsigned int i;
	unsigned int data_drop;

	cout << "Loading NGC data... ";
	FILE * ngcFile = fopen(catNGC.c_str(),"rb");
	if (!ngcFile) {
		cerr << "NGC data file " << catNGC << " not found" << endl;
		return false;
	}

	// read the number of lines
	unsigned int catalogSize = 0;
	while (fgets(recordstr,512,ngcFile)) catalogSize++;
	rewind(ngcFile);

	// Read the NGC entries
	i = 0;
	data_drop = 0;
	while (fgets(recordstr,512,ngcFile)) { // temporary for testing
		if (!(i%200) || (i == catalogSize-1)) {
			// Draw loading bar
			lb.SetMessage(_("Loading NGC catalog: ") + Utility::intToString((i == catalogSize-1 ? catalogSize : i)) +
			              "/" + Utility::intToString(catalogSize));
			lb.Draw((float)i/catalogSize);
		}
		Nebula *e = new Nebula;
		if (!e->readNGC(recordstr)) { // reading error
			delete e;
			e = NULL;
			data_drop++;
		} else {
			neb_array.push_back(e);
			nebZones[nebGrid.GetNearest(e->XYZ)].push_back(e);
		}
		i++;
	}
	fclose(ngcFile);
	printf("(%d items loaded [%d dropped])\n", i, data_drop);

	return true;
}


bool NebulaMgr::loadNGCNames(const string& catNGCNames)
{
	char recordstr[512];
	cout << "Loading NGC name data...";
	FILE * ngcNameFile = fopen(catNGCNames.c_str(),"rb");
	if (!ngcNameFile) {
		cerr << "NGC name data file " << catNGCNames << " not found." << endl;
		return false;
	}

	// Read the names of the NGC objects
	int i = 0;
	char n[40];
	int nb, k;
	Nebula *e;

	while (fgets(recordstr,512,ngcNameFile)) {

		sscanf(&recordstr[38],"%d",&nb);
		if (recordstr[37] == 'I') {
			e = (Nebula*)searchIC(nb);
		} else {
			e = (Nebula*)searchNGC(nb);
		}

		if (e) {
			strncpy(n, recordstr, 36);
			// trim the white spaces at the back
			n[36] = '\0';
			k = 36;
			while (n[--k] == ' ' && k >= 0) {
				n[k] = '\0';
			}
			// If the name is not a messier number perhaps one is already
			// defined for this object
			if (strncmp(n, "M ",2)) {
				e->englishName = n;
			}

			// If it's a messiernumber, we will call it a messier if there is no better name
			if (!strncmp(n, "M ",2)) {
				istringstream iss(string(n).substr(1));  // remove the 'M'
				ostringstream oss;

				int num;

				// Let us keep the right number in the Messier catalog
				iss >> num;
				e->M_nb=(unsigned int)(num);

				oss << "M" << num;
				e->englishName = oss.str();
			}
		} else
			cerr << endl << "...no position data for " << n;
		i++;
	}
	fclose(ngcNameFile);
	cout << "( " << i << " names loaded)" << endl;

	return true;
}

bool NebulaMgr::loadTextures(const string& fileName, LoadingBar& lb)
{
	cout << "Loading Nebula Textures...";

	std::ifstream inf(fileName.c_str());
	if (!inf.is_open()) {
		cerr << "Can't open nebula catalog " << fileName << endl;
		return false;
	}

	// determine total number to be loaded for percent complete display
	string record;
	int total=0;
	while (!inf.eof() && getline(inf, record)) {
		++total;
	}
	inf.clear();
	inf.seekg(0);

	printf("(%d textures loaded)\n", total);

	int current = 0;
	int NGC;
	while (!inf.eof() && getline(inf, record)) {
		// Draw loading bar
		++current;
		lb.SetMessage(_("Loading Nebula Textures:") + Utility::intToString(current) + "/" + Utility::intToString(total));
		lb.Draw((float)current/total);

		istringstream istr(record);
		istr >> NGC;

		Nebula *e = (Nebula*)searchNGC(NGC);
		if (e) {
			if (!e->readTexture(record)) // reading error
				cerr << "Error while loading nebula record: " << record << endl;
		} else {
			// Allow non NGC nebulas/textures!

			if ( NGC != -1) cout << "Nebula with unrecognized NGC number " << NGC << endl;
			e = new Nebula;
			if (!e->readTexture(record)) { // reading error
				cerr << "Error while loading nebula record: " << record << endl;
				delete e;

			} else {

				neb_array.push_back(e);
				nebZones[nebGrid.GetNearest(e->XYZ)].push_back(e);
			}
		}

	}
	return true;
}

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void NebulaMgr::translateNames(Translator& trans)
{
	vector<Nebula*>::iterator iter;
	for ( iter = neb_array.begin(); iter < neb_array.end(); iter++ ) {
		(*iter)->translateName(trans);
	}
	if( SharedData::Instance()->DB() ) {
		SharedData::Instance()->DB()->commit();
	}

	if(Nebula::nebula_font) Nebula::nebula_font->clearCache();
}


//! Return the matching Nebula object's pointer if exists or NULL
Object NebulaMgr::searchByNameI18n(const string& nameI18n) const
{
	string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
	vector <Nebula*>::const_iterator iter;

	// Search by NGC numbers (possible formats are "NGC31" or "NGC 31")
	if (objw.substr(0, 3) == "NGC") {
		for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
			if((*iter)->m_hidden==true) continue;
			if (("NGC" + Utility::intToString((*iter)->NGC_nb)) == objw ||
			        ("NGC " + Utility::intToString((*iter)->NGC_nb)) == objw)
				return *iter;
		}
	}

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->m_hidden==true) continue;
		string objwcap = (*iter)->nameI18;
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return *iter;
	}

	// Search by Messier numbers (possible formats are "M31" or "M 31")
	if (objw.substr(0, 1) == "M") {
		for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
			if((*iter)->m_hidden==true) continue;
			if (("M" + Utility::intToString((*iter)->M_nb)) == objw ||
			        ("M " + Utility::intToString((*iter)->M_nb)) == objw)
				return *iter;
		}
	}

	return NULL;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
vector<string> NebulaMgr::listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	vector <Nebula*>::const_iterator iter;

	// Search by messier objects number (possible formats are "M31" or "M 31")
	if (objw.size() >= 1 && objw[0]==L'M') {
		for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
			if ((*iter)->M_nb==0) continue;
			if((*iter)->m_hidden==true) continue;
			string constw = "M" + Utility::intToString((*iter)->M_nb);
			string constws = constw.substr(0, objw.size());
			if (constws==objw) {
				result.push_back(constw);
				continue;	// Prevent adding both forms for name
			}
			constw = "M " + Utility::intToString((*iter)->M_nb);
			constws = constw.substr(0, objw.size());
			if (constws==objw) {
				result.push_back(constw);
			}
		}
	}

	// Search by NGC numbers (possible formats are "NGC31" or "NGC 31")
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->NGC_nb==0) continue;
		if((*iter)->m_hidden==true) continue;
		string constw = "NGC" + Utility::intToString((*iter)->NGC_nb);
		string constws = constw.substr(0, objw.size());
		if (constws==objw) {
			result.push_back(constw);
			continue;
		}
		constw = "NGC " + Utility::intToString((*iter)->NGC_nb);
		constws = constw.substr(0, objw.size());
		if (constws==objw) {
			result.push_back(constw);
		}
	}

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->m_hidden==true) continue;
		string constw = (*iter)->nameI18.substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->nameI18);
		}
	}

	sort(result.begin(), result.end());
	if (result.size()>maxNbItem) result.erase(result.begin()+maxNbItem, result.end());

	return result;
}

