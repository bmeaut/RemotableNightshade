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

#include <iostream>
#include "nebula.h"
#include "s_texture.h"
#include "nightshade.h"
#include "s_font.h"
#include "navigator.h"
#include "utility.h"
#include "s_gui.h"
#include <nshade_state.h>

s_texture * Nebula::tex_circle = NULL;
s_font* Nebula::nebula_font = NULL;
float Nebula::circleScale = 1.f;
float Nebula::hints_brightness = 0;
float Nebula::nebula_brightness = 1;
Vec3f Nebula::label_color = Vec3f(0.4,0.3,0.5);
Vec3f Nebula::circle_color = Vec3f(0.8,0.8,0.1);
bool Nebula::flagBright = false;
const float Nebula::RADIUS_NEB = 1.f;


Nebula::Nebula() :
		M_nb(0),
		NGC_nb(0),
		IC_nb(0),
		/*UGC_nb(0),*/
		neb_tex(NULL),
		m_deleteable(false),
		m_hidden(false)
{
	inc_lum = rand()/RAND_MAX*M_PI;
	nameI18 = "";
}

Nebula::~Nebula()
{
	if ( SharedData::Instance()->DB() ){
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_NEBULA;
		if( cursor.select(q) )
			cursor.remove();
		SharedData::Instance()->DB()->commit();
	}

	delete neb_tex;
	neb_tex = NULL;
}

string Nebula::getInfoString(const Navigator* nav) const
{
	float tempDE, tempRA;

	Vec3d equPos = nav->j2000_to_earth_equ(XYZ);
	rect_to_sphe(&tempRA,&tempDE,equPos);

	ostringstream oss;
	if (nameI18!="") {
		oss << nameI18 << " (";
	}
	if ((M_nb > 0) && (M_nb < 111)) {
		oss << "M " << M_nb << " - ";
	}
	if (NGC_nb > 0) {
		oss << "NGC " << NGC_nb;
	}
	if (IC_nb > 0) {
		oss << "IC " << IC_nb;
	}
	/*if (UGC_nb > 0)
	{
		oss << "UGC " << UGC_nb;
	}*/
	if (nameI18!="") {
		oss << ")";
	}
	oss << endl;

	oss.setf(ios::fixed);
	oss.precision(2);

	if(mag < 99) oss << _("Magnitude: ") << mag << endl;

	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << " / " << Utility::printAngleDMS(tempDE) << endl;

	// calculate alt az
	Vec3d localPos = nav->earth_equ_to_local(equPos);
	rect_to_sphe(&tempRA,&tempDE,localPos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;

	oss << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << " / " << Utility::printAngleDMS(tempRA) << endl;

	oss << _("Type: ") << getTypeString() << endl;

	oss << _("Size: ") << Utility::printAngleDMS(m_angular_size*M_PI/180.) << endl;

	return oss.str();
}

string Nebula::getShortInfoString(const Navigator*) const
{
	if (nameI18!="") {
		ostringstream oss;
		oss << nameI18 << "  ";
		if (mag < 99) oss << _("Magnitude: ") << mag;

		if( m_distance > 0 ) {
			string units = _("ly");
			double distance = m_distance;
		
			if(distance >= 1000) {
				distance /= 1000;
				if(distance < 1000) units = _("kly");
				else {
					distance /= 1000;
					units = _("Mly");
				}
			}

			oss.precision(5);
//			oss.setf(ios::fixed, ios::floatfield);
			oss << "  " <<  _("Distance: ") << distance << " " << _(units);
		}

		return oss.str();
	} else {
		if (M_nb > 0) {
			return "M " + Utility::intToString(M_nb);
		} else if (NGC_nb > 0) {
			return "NGC " + Utility::intToString(NGC_nb);
		} else if (IC_nb > 0) {
			return "IC " + Utility::intToString(IC_nb);
		}
	}

	// All nebula have at least an NGC or IC number
	assert(false);
	return "";
}

double Nebula::get_close_fov(const Navigator*) const
{
	return m_angular_size * 180./M_PI * 4;
}


// Read nebula data passed in and compute x,y and z;
// returns false if can't parse record
bool Nebula::readTexture(double ra, double de, double magnitude, double angular_size, double rotation,
						 string name, string filename, string credit, double texture_luminance_adjust, double distance)
{
	string tex_name = filename;
	float tex_angular_size = angular_size;
	float tex_rotation = rotation;

	m_distance = distance;
	m_credit = credit;
	mag = magnitude;
	tex_luminance_adjust = texture_luminance_adjust;

	m_deleteable = true;

	if (m_credit  == "none")
		m_credit = "";
	else
		m_credit = string("Credit: ") + m_credit;

	for (string::size_type i=0; i<m_credit.length(); ++i) {
		if (m_credit[i]=='_') m_credit[i]=' ';
	}

	// Only set name if not already set from NGC data
	if (englishName == "") {
		for (string::size_type i=0; i<name.length(); ++i) {
			if (name[i]=='_') name[i]=' ';
		}
		englishName = name;
	}

	nameI18 = englishName;  // To start with until next translation event

	// Calc the RA and DE from the datas
	float RaRad = ra*M_PI/180.;
	float DecRad = de*M_PI/180.;

	// - keep base info for drawing (in radians)
	RA = RaRad;
	DE = DecRad;
	ASIZE = tex_angular_size/60*M_PI/180;
	ROTATION = tex_rotation*M_PI/180;

	// Calc the Cartesian coord with RA and DE
	sphe_to_rect(RaRad,DecRad,XYZ);
	XYZ*=RADIUS_NEB;

	// Calc the angular size in radian : TODO this should be independant of tex_angular_size
	m_angular_size = tex_angular_size/2/60*M_PI/180;

	neb_tex = new s_texture(true, tex_name, TEX_LOAD_TYPE_PNG_ALPHA, true);  // use mipmaps

	//tex_angular_size*tex_angular_size*3600/4*M_PI
	//	luminance = mag_to_luminance(mag, tex_angular_size*tex_angular_size*3600) /	neb_tex->get_average_luminance() * 50;
	luminance = mag_to_luminance(mag, tex_angular_size*tex_angular_size*3600);

	// this is a huge performance drag if called every frame, so cache here
	tex_avg_luminance = neb_tex->get_average_luminance();

	float tex_size = RADIUS_NEB * sin(tex_angular_size/2/60*M_PI/180);

	// Precomputation of the rotation/translation matrix
	Mat4f mat_precomp = Mat4f::translation(XYZ) *
	                    Mat4f::zrotation(RaRad) *
	                    Mat4f::yrotation(-DecRad) *
	                    Mat4f::xrotation(tex_rotation*M_PI/180.);

	tex_quad_vertex[0] = mat_precomp * Vec3f(0.,-tex_size,-tex_size); // Bottom Right
	tex_quad_vertex[1] = mat_precomp * Vec3f(0., tex_size,-tex_size); // Bottom Right
	tex_quad_vertex[2] = mat_precomp * Vec3f(0.,-tex_size, tex_size); // Bottom Right
	tex_quad_vertex[3] = mat_precomp * Vec3f(0., tex_size, tex_size); // Bottom Right

	//	cout << "Created new nebula " << name << " " << filename << " " << magnitude << endl;

	if( SharedData::Instance()->DB() ) {
		ObjectRecord rec( englishName.c_str(), nameI18.c_str(), ObjectRecord::OBJECT_NEBULA, mag );
		insert(rec);
		SharedData::Instance()->DB()->commit();
	}

	return true;
}


// Read nebula data from file and compute x,y and z;
// returns false if can't parse record
bool Nebula::readTexture(const string& record)
{
	string tex_name;
	string name;
	float ra;
	float de;
	float tex_angular_size;
	float tex_rotation;
	int ngc;
	double distance;

	std::istringstream istr(record);

// read in texture drawing magnitude (different from object magnitude)
	if (!(istr >> ngc >> ra >> de >> mag >> tex_angular_size >> tex_rotation >> name >> tex_name >> m_credit >> tex_luminance_adjust >> distance)) return false;
	//	if (!(istr >> ngc >> ra >> de >> mag >> tex_angular_size >> tex_rotation >> name >> tex_name >> m_credit)) return false ;

	//	cout << name << " " << mag << " " << draw_mag << endl;

	m_distance = distance;

	if (m_credit  == "none")
		m_credit = "";
	else
		m_credit = string("Credit: ") + m_credit;

	for (string::size_type i=0; i<m_credit.length(); ++i) {
		if (m_credit[i]=='_') m_credit[i]=' ';
	}

	// Only set name if not already set from NGC data
	if (englishName == "") {
		for (string::size_type i=0; i<name.length(); ++i) {
			if (name[i]=='_') name[i]=' ';
		}
		englishName = name;
	}

	// Calc the RA and DE from the datas
	float RaRad = ra*M_PI/180.;
	float DecRad = de*M_PI/180.;

// - keep base info for drawing (in radians)
	RA = RaRad;
	DE = DecRad;
	ASIZE = tex_angular_size/60*M_PI/180;
	ROTATION = tex_rotation*M_PI/180;

	// Calc the Cartesian coord with RA and DE
	sphe_to_rect(RaRad,DecRad,XYZ);
	XYZ*=RADIUS_NEB;

	// Calc the angular size in radian : TODO this should be independant of tex_angular_size
	m_angular_size = tex_angular_size/2/60*M_PI/180;

	neb_tex = new s_texture(tex_name, TEX_LOAD_TYPE_PNG_ALPHA, true);  // use mipmaps

	//tex_angular_size*tex_angular_size*3600/4*M_PI
	//	luminance = mag_to_luminance(mag, tex_angular_size*tex_angular_size*3600) /	neb_tex->get_average_luminance() * 50;
	luminance = mag_to_luminance(mag, tex_angular_size*tex_angular_size*3600);

	// this is a huge performance drag if called every frame, so cache here
	tex_avg_luminance = neb_tex->get_average_luminance();

	float tex_size = RADIUS_NEB * sin(tex_angular_size/2/60*M_PI/180);

	// Precomputation of the rotation/translation matrix
	Mat4f mat_precomp = Mat4f::translation(XYZ) *
	                    Mat4f::zrotation(RaRad) *
	                    Mat4f::yrotation(-DecRad) *
	                    Mat4f::xrotation(tex_rotation*M_PI/180.);

	tex_quad_vertex[0] = mat_precomp * Vec3f(0.,-tex_size,-tex_size); // Bottom Right
	tex_quad_vertex[1] = mat_precomp * Vec3f(0., tex_size,-tex_size); // Bottom Right
	tex_quad_vertex[2] = mat_precomp * Vec3f(0.,-tex_size, tex_size); // Bottom Right
	tex_quad_vertex[3] = mat_precomp * Vec3f(0., tex_size, tex_size); // Bottom Right

	if( SharedData::Instance()->DB() ) {
		ObjectRecord rec( englishName.c_str(), nameI18.c_str(), ObjectRecord::OBJECT_NEBULA, mag );
		insert(rec);
		SharedData::Instance()->DB()->commit();
	}

	return true;
}


void Nebula::draw_chart(const Projector* prj, const Navigator * nav)
{
	bool lastState = glIsEnabled(GL_TEXTURE_2D);
	float r = (get_on_screen_size(prj, nav)/2)* 1.2; // slightly bigger than actual!
	if (r < 5) r = 5;
	r *= circleScale;

	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);

	glColor3fv(circle_color);
	if (nType == NEB_UNKNOWN) {
		glCircle(XY,r);
	} else if (nType == NEB_N) { // supernova reemnant
		glCircle(XY,r);
	} else if (nType == NEB_PN) { // planetary nebula
		glCircle(XY,0.4*r);

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0]-r, XY[1],0.f);
		glVertex3f(XY[0]-0.4*r, XY[1],0.f);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0]+r, XY[1],0.f);
		glVertex3f(XY[0]+0.4*r, XY[1],0.f);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0], XY[1]+r,0.f);
		glVertex3f(XY[0], XY[1]+0.4*r,0.f);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0], XY[1]-r,0.f);
		glVertex3f(XY[0], XY[1]-0.4*r,0.f);
		glEnd();
	} else if (nType == NEB_OC) { // open cluster
		glLineStipple(2, 0x3333);
		glEnable(GL_LINE_STIPPLE);
		glCircle(XY,r);
		glDisable(GL_LINE_STIPPLE);
	} else if (nType == NEB_GC) { // Globular cluster
		glCircle(XY,r);

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0]-r, XY[1],0.f);
		glVertex3f(XY[0]+r, XY[1],0.f);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(XY[0], XY[1]-r,0.f);
		glVertex3f(XY[0], XY[1]+r,0.f);
		glEnd();
	} else if (nType == NEB_DN) { // Diffuse Nebula
		glLineStipple(1, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
		glCircle(XY,r);
		glDisable(GL_LINE_STIPPLE);
	} else if (nType == NEB_IG) { // Irregular
		glEllipse(XY,r,0.5);
	} else { // not sure what type!!!
		glCircle(XY,r);
	}
	glLineWidth(1.0f);

	if (lastState) glEnable(GL_TEXTURE_2D);
}

void Nebula::draw_tex(const Projector* prj, const Navigator* nav, ToneReproductor* eye, double sky_brightness)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	if (!neb_tex || m_hidden) return;

	// if start zooming in, turn up brightness to full for DSO images
	// gradual change might be better

// daylight hackery
	float ad_lum=eye->adapt_luminance(luminance);

	float color = 1;
	if (flagBright && sky_brightness < 0.011 && (get_on_screen_size(prj, nav) > prj->getViewportHeight()/64.))  	{
		//	  cout << "Bright nebula drawn for" << getEnglishName() << endl;
		color *= nebula_brightness;
	} else {

		// TODO this should be revisited to be less ad hoc
		// 3 is a fudge factor since only about 1/3 of a texture is not black background
		float cmag = 3 * ad_lum / tex_avg_luminance * tex_luminance_adjust;
		color = color * cmag * nebula_brightness;
	}

	glColor4f(1, 1, 1, color);

	glBindTexture(GL_TEXTURE_2D, neb_tex->getID());


	/* DIGITALIS added distortion correction 20080312

	Vec3d v;

	glBegin(GL_TRIANGLE_STRIP);
	    glTexCoord2i(1,0);              // Bottom Right
		prj->project_j2000(tex_quad_vertex[0],v); glVertex3dv(v);
	    glTexCoord2i(0,0);              // Bottom Left
		prj->project_j2000(tex_quad_vertex[1],v); glVertex3dv(v);
	    glTexCoord2i(1,1);              // Top Right
		prj->project_j2000(tex_quad_vertex[2],v); glVertex3dv(v);
	    glTexCoord2i(0,1);              // Top Left
		prj->project_j2000(tex_quad_vertex[3],v); glVertex3dv(v);
		glEnd();
	*/

	Vec3d gridpt, onscreen;

	Vec3d imagev = Mat4d::zrotation(RA-M_PI_2)
	               * Mat4d::xrotation(DE) * Vec3d(0,1,0);

	Vec3d ortho1 = Mat4d::zrotation(RA-M_PI_2) * Vec3d(1,0,0);
	Vec3d ortho2 = imagev^ortho1;

	//	  cout << RA << " " << DE << " " << ASIZE << " " << ROTATION << endl;

	float image_scale = ASIZE;

	int grid_size = int(image_scale/5.);  // divisions per row, column
	if (grid_size < 5) grid_size = 5;

	for (int i=0; i<grid_size; i++) {

		glBegin(GL_QUAD_STRIP);

		for (int j=0; j<=grid_size; j++) {

			for (int k=0; k<=1; k++) {

				// image height is maximum angular dimension
				gridpt = Mat4d::rotation( imagev, ROTATION+M_PI) *
				         Mat4d::rotation( ortho1, image_scale*(j-grid_size/2.)/(float)grid_size) *
				         Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size) *
				         imagev;

				if ( prj->project_j2000(gridpt, onscreen)) {

					//		  cout << "on " << onscreen[0] << " " << onscreen[1] << " : "
					//   << (i+k)/(float)grid_size << " " << j/(float)grid_size << endl;
					glTexCoord2f((i+k)/(float)grid_size,j/(float)grid_size);

					glVertex3d(onscreen[0], onscreen[1], 0);

				}
			}

		}
		glEnd();

	}

}

void Nebula::draw_circle(const Projector* prj, const Navigator * nav)
{

	if (m_hidden) return;

	if (2.f/get_on_screen_size(prj, nav)<0.1) return;
	inc_lum++;
	float lum = MY_MIN(1,2.f/get_on_screen_size(prj, nav))*(0.8+0.2*sinf(inc_lum/10));
	glColor4f(circle_color[0], circle_color[1], circle_color[2], lum*hints_brightness);
	glBindTexture (GL_TEXTURE_2D, Nebula::tex_circle->getID());
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2i(1,0);              // Bottom Right
	glVertex3f(XY[0] + 4, XY[1] - 4, 0.0f);
	glTexCoord2i(0,0);              // Bottom Left
	glVertex3f(XY[0] - 4, XY[1] - 4, 0.0f);
	glTexCoord2i(1,1);              // Top Right
	glVertex3f(XY[0] + 4, XY[1] + 4,0.0f);
	glTexCoord2i(0,1);              // Top Left
	glVertex3f(XY[0] - 4, XY[1] + 4,0.0f);
	glEnd ();
}

void Nebula::draw_no_tex(const Projector* prj, const Navigator * nav,ToneReproductor* eye)
{

	if (m_hidden) return;

	float r = (get_on_screen_size(prj, nav)/2);
	float cmag = 0.20 * hints_brightness;

	glColor3f(cmag,cmag,cmag);
	glBindTexture(GL_TEXTURE_2D, tex_circle->getID());
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2f(XY[0]-r,XY[1]-r);	// Bottom left
	glTexCoord2i(1,0);
	glVertex2f(XY[0]+r,XY[1]-r);	// Bottom right
	glTexCoord2i(1,1);
	glVertex2f(XY[0]+r,XY[1]+r);	// Top right
	glTexCoord2i(0,1);
	glVertex2f(XY[0]-r,XY[1]+r);	// Top left
	glEnd();
}

void Nebula::draw_name(const Projector* prj)
{

	if (m_hidden) return;

	glColor4f(label_color[0], label_color[1], label_color[2], hints_brightness);
	float size = get_on_screen_size(prj);
	float shift = 8.f + size/2.f;

	string nebulaname = getNameI18n();

	if (prj->getFlagGravityLabels())
		prj->print_gravity180(nebula_font, XY[0], XY[1], nebulaname, 1, shift, shift);
	else
		nebula_font->print(XY[0]+shift, XY[1]+shift,nebulaname,1,1);

	// draw image credit, if it fits easily
	if (m_credit != "" && size > nebula_font->getStrLen(m_credit, 1)) {
		if (prj->getFlagGravityLabels())
			prj->print_gravity180(nebula_font, XY[0]-shift-40, XY[1]+-shift-40, m_credit, 1, 0, 0);
		else
			nebula_font->print(XY[0]-shift, XY[1]-shift-60, m_credit, 1, 1);
	}
}

bool Nebula::readNGC(char *recordstr)
{
	int rahr;
	float ramin;
	int dedeg;
	float demin;
	float tex_angular_size;
	int nb;

	sscanf(&recordstr[1],"%d",&nb);

	if (recordstr[0] == 'I') {
		IC_nb = nb;
	} else {
		NGC_nb = nb;
	}

	sscanf(&recordstr[12],"%d %f",&rahr, &ramin);
	sscanf(&recordstr[22],"%d %f",&dedeg, &demin);
	float RaRad = (double)rahr+ramin/60;
	float DecRad = (float)dedeg+demin/60;
	if (recordstr[21] == '-') DecRad *= -1.;

	RaRad*=M_PI/12.;     // Convert from hours to rad
	DecRad*=M_PI/180.;    // Convert from deg to rad

	// Calc the Cartesian coord with RA and DE
	sphe_to_rect(RaRad,DecRad,XYZ);
	XYZ*=Nebula::RADIUS_NEB;

	// Calc the angular size in radian : TODO this should be independant of tex_angular_size
	sscanf(&recordstr[47],"%f",&mag);
	if (mag < 1) mag = 99;

	sscanf(&recordstr[40],"%f",&tex_angular_size);
	if (tex_angular_size < 0)
		tex_angular_size = 1;
	if (tex_angular_size > 150)
		tex_angular_size = 150;

	m_angular_size = tex_angular_size/2/60*M_PI/180;

	luminance = mag_to_luminance(mag, tex_angular_size*tex_angular_size*3600);
	if (luminance < 0)
		luminance = .0075;

	// this is a huge performance drag if called every frame, so cache here
	if (neb_tex) delete neb_tex;
	neb_tex = NULL;

	if (!strncmp(&recordstr[8],"Gx",2)) {
		nType = NEB_GX;
	} else if (!strncmp(&recordstr[8],"OC",2)) {
		nType = NEB_OC;
	} else if (!strncmp(&recordstr[8],"Gb",2)) {
		nType = NEB_GC;
	} else if (!strncmp(&recordstr[8],"Nb",2)) {
		nType = NEB_N;
	} else if (!strncmp(&recordstr[8],"Pl",2)) {
		nType = NEB_PN;
	} else if (!strncmp(&recordstr[8],"  ",2)) {
		return false;
	} else if (!strncmp(&recordstr[8]," -",2)) {
		return false;
	} else if (!strncmp(&recordstr[8]," *",2)) {
		return false;
	} else if (!strncmp(&recordstr[8],"D*",2)) {
		return false;
	} else if (!strncmp(&recordstr[7],"***",3)) {
		return false;
	} else if (!strncmp(&recordstr[7],"C+N",3)) {
		nType = NEB_CN;
	} else if (!strncmp(&recordstr[8]," ?",2)) {
		nType = NEB_UNKNOWN;
	} else {
		nType = NEB_UNKNOWN;
	}

	return true;
}

string Nebula::getTypeString(void) const
{
	string wsType;

	switch (nType) {
	case NEB_GX:
		wsType = "Galaxy";
		break;
	case NEB_OC:
		wsType = "Open cluster";
		break;
	case NEB_GC:
		wsType = "Globular cluster";
		break;
	case NEB_N:
		wsType = "Nebula";
		break;
	case NEB_PN:
		wsType = "Planetary nebula";
		break;
	case NEB_CN:
		wsType = "Cluster associated with nebulosity";
		break;
	case NEB_UNKNOWN:
		wsType = "Unknown";
		break;
	default:
		wsType = "Undocumented type";
		break;
	}
	return wsType;
}

void Nebula::translateName(Translator& trans) {
	nameI18 = trans.translateUTF8(englishName);

	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "englishName=",englishName.c_str(),"and type=",ObjectRecord::OBJECT_NEBULA;
		if( cursor.select(q) ) {
			cursor->nameI18 = nameI18.c_str();
			cursor.update();
		}
	}
}

