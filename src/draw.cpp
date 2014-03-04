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

#include "planet.h"
#include "draw.h"
#include "s_texture.h"
#include "utility.h"

// rms added color as parameter
SkyGrid::SkyGrid(SKY_GRID_TYPE grid_type, unsigned int _nb_meridian, unsigned int _nb_parallel, double _radius,
                 unsigned int _nb_alt_segment, unsigned int _nb_azi_segment) :
	nb_meridian(_nb_meridian), nb_parallel(_nb_parallel), 	radius(_radius),
	nb_alt_segment(_nb_alt_segment), nb_azi_segment(_nb_azi_segment), color(0.2,0.2,0.2)
{
	transparent_top = true;
	gtype = grid_type;
	switch (grid_type) {
	case ALTAZIMUTAL :
		proj_func = &Projector::project_local;
		break;
	case EQUATORIAL :
		proj_func = &Projector::project_earth_equ;
		break;
	case GALACTIC :
		proj_func = &Projector::project_galactic;
		break;
	default :
		proj_func = &Projector::project_earth_equ;
	}

	// Alt points are the points to draw along the meridian
	alt_points = new Vec3f*[nb_meridian];
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		alt_points[nm] = new Vec3f[nb_alt_segment+1];
		for (unsigned int i=0; i<nb_alt_segment+1; ++i) {
			sphe_to_rect((float)nm/(nb_meridian)*2.f*M_PI,
			             (float)i/nb_alt_segment*M_PI-M_PI_2, alt_points[nm][i]);
			alt_points[nm][i] *= radius;
		}
	}

	// Alt points are the points to draw along the meridian
	azi_points = new Vec3f*[nb_parallel];
	for (unsigned int np=0; np<nb_parallel; ++np) {
		azi_points[np] = new Vec3f[nb_azi_segment+1];
		for (unsigned int i=0; i<nb_azi_segment+1; ++i) {
			sphe_to_rect((float)i/(nb_azi_segment)*2.f*M_PI,
			             (float)(np+1)/(nb_parallel+1)*M_PI-M_PI_2, azi_points[np][i]);
			azi_points[np][i] *= radius;
		}
	}
}

SkyGrid::~SkyGrid()
{
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		delete [] alt_points[nm];
	}
	delete [] alt_points;

	for (unsigned int np=0; np<nb_parallel; ++np) {
		delete [] azi_points[np];
	}
	delete [] azi_points;

	if (font) delete font;
	font = NULL;

}

void SkyGrid::set_font(float font_size, const string& font_name)
{
	font = new s_font(font_size, font_name);
	assert(font);
}

void SkyGrid::draw(const Projector* prj) const
{
	if (!fader.getInterstate()) return;

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	Vec3d pt1;
	Vec3d pt2;

	prj->set_orthographic_projection();	// set 2D coordinate

	// Draw meridians
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		if (transparent_top) {	// Transparency for the first and last points
			if ((prj->*proj_func)(alt_points[nm][0], pt1) &&
				(prj->*proj_func)(alt_points[nm][1], pt2) ) {
				glColor4f(color[0],color[1],color[2],0.f);

				glBegin (GL_LINES);
				glVertex2f(pt1[0],pt1[1]);
				glColor4f(color[0],color[1],color[2],fader.getInterstate());
				glVertex2f(pt2[0],pt2[1]);
				glEnd();
			}

			glColor4f(color[0],color[1],color[2],fader.getInterstate());

			for (unsigned int i=1; i<nb_alt_segment-1; ++i) {
				if ((prj->*proj_func)(alt_points[nm][i], pt1) &&
					(prj->*proj_func)(alt_points[nm][i+1], pt2) ) {
					glBegin(GL_LINES);
					glVertex2f(pt1[0],pt1[1]);
					glVertex2f(pt2[0],pt2[1]);
					glEnd();

					static char str[255];	// TODO use c++ string

					glEnable(GL_TEXTURE_2D);

					double angle;

					const double dx = pt1[0]-pt2[0];
					const double dy = pt1[1]-pt2[1];
					const double dq = dx*dx+dy*dy;

					// TODO: allow for other numbers of meridians and parallels without
					// screwing up labels?
					//if( gtype == EQUATORIAL && i == 8 ) {
					if ( i == 8 ) {
						// draw labels along equator for RA
						const double d = sqrt(dq);

						angle = acos((pt1[1]-pt2[1])/d);
						if ( pt1[0] < pt2[0] ) {
							angle *= -1;
						}

						if ( gtype == EQUATORIAL ) {
#ifdef NAV 
							if (nm<=12) sprintf( str, "%d°W", nm*15); else sprintf( str, "%d°E", (24-nm)*15);
#else
							sprintf( str, "%dh", nm);
#endif
						} else if ( gtype == GALACTIC ) {
							sprintf( str, "%d", nm*15);
						} else {
							sprintf( str, "%d", nm<=12 ? (12-nm)*15 : (36-nm)*15 );
						}

						prj->set_orthographic_projection();

						glTranslatef(pt2[0],pt2[1],0);
						glRotatef(90+angle*180./M_PI,0,0,-1);

						if ( gtype == EQUATORIAL ) {
							font->print(2,-2,str,1,1);
						} else {
							font->print(6,-2,str,1,1);
						}

						prj->reset_perspective_projection();


					} else if (nm % 8 == 0 && i != 16) {

						const double d = sqrt(dq);

						angle = acos((pt1[1]-pt2[1])/d);
						if ( pt1[0] < pt2[0] ) {
							angle *= -1;
						}

						sprintf( str, "%d", (i-8)*10);

						if ( gtype == ALTAZIMUTAL || i > 8) {
							angle += M_PI;
						}

						prj->set_orthographic_projection();

						glTranslatef(pt2[0],pt2[1],0);
						glRotatef(angle*180./M_PI,0,0,-1);
						font->print(2,-2,str,1,1);
						prj->reset_perspective_projection();

					}
					glDisable(GL_TEXTURE_2D);
				}


			}

			if ((prj->*proj_func)(alt_points[nm][nb_alt_segment-1], pt1) &&
				(prj->*proj_func)(alt_points[nm][nb_alt_segment], pt2) ) {
				glColor4f(color[0],color[1],color[2],fader.getInterstate());
				glBegin (GL_LINES);
				glVertex2f(pt1[0],pt1[1]);
				glColor4f(color[0],color[1],color[2],0.f);
				glVertex2f(pt2[0],pt2[1]);
				glEnd();
			}

		} else {
			glColor4f(color[0],color[1],color[2],fader.getInterstate());
			for (unsigned int i=0; i<nb_alt_segment; ++i) {
				if ((prj->*proj_func)(alt_points[nm][i], pt1) &&
					(prj->*proj_func)(alt_points[nm][i+1], pt2) ) {
					glBegin (GL_LINES);
					glVertex2f(pt1[0],pt1[1]);
					glVertex2f(pt2[0],pt2[1]);
					glEnd();
				}
			}
		}
	}

	// Draw parallels
	glColor4f(color[0],color[1],color[2],fader.getInterstate());
	for (unsigned int np=0; np<nb_parallel; ++np) {
		for (unsigned int i=0; i<nb_azi_segment; ++i) {
			if ((prj->*proj_func)(azi_points[np][i], pt1) &&
				(prj->*proj_func)(azi_points[np][i+1], pt2) ) {
				glBegin (GL_LINES);
				glVertex2f(pt1[0],pt1[1]);
				glVertex2f(pt2[0],pt2[1]);
				glEnd();
			}
		}
#ifdef NAV 
		np++;
#endif
	}

	prj->reset_perspective_projection();
}


SkyLine::SkyLine(SKY_LINE_TYPE _line_type, double _radius, unsigned int _nb_segment) :
	radius(_radius), nb_segment(_nb_segment), color(0.f, 0.f, 1.f), font(NULL)
{
	float inclination = 0.f;
	line_type = _line_type;

	switch (line_type) {
	case LOCAL :
		proj_func = &Projector::project_local;
		break;
	case MERIDIAN :
		proj_func = &Projector::project_local;
		inclination = 90;
		break;
	case ECLIPTIC :
		proj_func = &Projector::project_j2000;
		inclination = 23.4392803055555555556;
		break;
	case PRECESSION :
		proj_func = &Projector::project_j2000;
		inclination = 23.4392803055555555556;
		break;
	case EQUATOR :
		proj_func = &Projector::project_earth_equ;
		break;
	case CIRCUMPOLAR :
		proj_func = &Projector::project_earth_equ;
		break;
	case TROPIC :
		proj_func = &Projector::project_earth_equ;
		break;
	default :
		proj_func = &Projector::project_earth_equ;
	}

	Mat4f rotation = Mat4f::xrotation(inclination*M_PI/180.f);

	// Ecliptic month labels need to be redone
	// correct for month labels
	// TODO: can make this more accurate
	//	if(line_type == ECLIPTIC ) r = r * Mat4f::zrotation(-77.9*M_PI/180.);

	// Points to draw along the circle
	points = new Vec3f[3*nb_segment+3];
	for (unsigned int i=0; i<nb_segment+1; ++i) {
		sphe_to_rect((float)i/(nb_segment)*2.f*M_PI, 0.f, points[i]);
		points[i] *= radius;
		points[i].transfo4d(rotation);
	}
}

SkyLine::~SkyLine()
{
	delete [] points;
	points = NULL;
	if (font) delete font;
	font = NULL;
}

void SkyLine::set_font(float font_size, const string& font_name)
{
	if (font) delete font;
	font = new s_font(font_size, font_name);
	assert(font);
}

// Translate labels for lines (currently just ecliptic) with gettext to current sky language
void SkyLine::translateLabels(Translator& trans)
{

	month[1] = trans.translateUTF8("JAN");
	month[2] = trans.translateUTF8("FEB");
	month[3] = trans.translateUTF8("MAR");
	month[4] = trans.translateUTF8("APR");
	month[5] = trans.translateUTF8("MAY");
	month[6] = trans.translateUTF8("JUN");
	month[7] = trans.translateUTF8("JUL");
	month[8] = trans.translateUTF8("AUG");
	month[9] = trans.translateUTF8("SEP");
	month[10] = trans.translateUTF8("OCT");
	month[11] = trans.translateUTF8("NOV");
	month[12] = trans.translateUTF8("DEC");

	if(font) font->clearCache();
}

// void SkyLine::draw(const Projector* prj) const
void SkyLine::draw(const Projector *prj,const Navigator *nav) const
{
	float inclination = 0.0f;
	if (!fader.getInterstate()) return;

	Vec3d pt1;
	Vec3d pt2;

	glColor4f(color[0], color[1], color[2], fader.getInterstate());
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	prj->set_orthographic_projection();	// set 2D coordinate

	if (line_type == ECLIPTIC) {
		// special drawing of the ecliptic line
		const Mat4d m = nav->getHomePlanet()->getRotEquatorialToVsop87().transpose();
		const bool draw_labels = (nav->getHomePlanet()->getEnglishName()=="Earth" && font);
		// start labeling from the vernal equinox
		//	  const double corr = draw_labels ? (atan2(m.r[4],m.r[0]) - 3*M_PI/6) : 0.0;
		const double corr = draw_labels ? (atan2(m.r[4],m.r[0]) - 2.68*M_PI/6) : 0.0;
		Vec3d point(radius*cos(corr),radius*sin(corr),0.0);
		point.transfo4d(m);
		bool prev_on_screen = prj->project_earth_equ(point,pt1);
		for (unsigned int i=1; i<nb_segment+1; ++i) {
			const double phi = corr+2*i*M_PI/nb_segment;
			Vec3d point(radius*cos(phi),radius*sin(phi),0.0);
			point.transfo4d(m);
			const bool on_screen = prj->project_earth_equ(point,pt2);
			if (on_screen && prev_on_screen) {
				const double dx = pt2[0]-pt1[0];
				const double dy = pt2[1]-pt1[1];
				const double dq = dx*dx+dy*dy;

				glBegin (GL_LINES);
				glVertex2f(pt2[0],pt2[1]);
				glVertex2f(pt1[0],pt1[1]);
				glEnd();

				if(i % 4 == 0) {
					const double d = sqrt(dq);
					double angle;

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					glPushMatrix();
					glTranslatef(pt2[0],pt2[1],0);
					glRotatef(180+angle*180./M_PI,0,0,-1);
					glBegin (GL_LINES);
					glVertex2f(-3,0);
					glVertex2f(3,0);
					glEnd();	
					glPopMatrix();
				}
				
				if (draw_labels && (i+2) % 4 == 0) {

					const double d = sqrt(dq);

					double angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					// draw text label
					std::ostringstream oss;

					// TODO: center labels

					if (nav->getHomePlanet()->getEnglishName()=="Earth") {
						oss << month[ (i+3)/4 ];
					}

					glPushMatrix();
					glTranslatef(pt2[0],pt2[1],0);
					glRotatef(-90+angle*180./M_PI,0,0,-1);

					glEnable(GL_TEXTURE_2D);

					font->print(0,-2,oss.str(),1,1);
					glPopMatrix();
					glDisable(GL_TEXTURE_2D);

				}
			}
			prev_on_screen = on_screen;
			pt1 = pt2;
		}
		
	} else if(line_type == PRECESSION) {

		// Currently limit precession circle drawing to Earth only
		if(nav->getHomePlanet()->getEnglishName()=="Earth") {

			// special drawing of the precession lines (both poles)
			const Mat4d m = nav->getHomePlanet()->getRotEquatorialToVsop87().transpose();
			const bool draw_labels = (font != NULL);
		    
			const double corr = draw_labels ? (atan2(m.r[4],m.r[0]) - 2.68*M_PI/6) : 0.0;
			
			bool prev_on_screen;
			
			for(int pole=1; pole>=-1; pole-=2) {

				Vec3d point(radius*cos(corr),radius*sin(corr),pole*radius*2.3213f);
				point.transfo4d(m);			
				prev_on_screen = prj->project_earth_equ(point,pt1);
	
				for (unsigned int i=1; i<104+1; ++i) {
					const double phi = corr+2*i*M_PI/104;
					Vec3d point(radius*cos(phi),radius*sin(phi),pole*radius*2.3213f);
					point.transfo4d(m);
					const bool on_screen = prj->project_earth_equ(point,pt2);
					if (on_screen && prev_on_screen) {
						const double dx = pt2[0]-pt1[0];
						const double dy = pt2[1]-pt1[1];
						const double dq = dx*dx+dy*dy;
					
						glBegin (GL_LINES);
						glVertex2f(pt2[0],pt2[1]);
						glVertex2f(pt1[0],pt1[1]);
						glEnd();
					
						if(i % 4 == 0) {
							const double d = sqrt(dq);
							double angle;
						
							angle = acos((pt1[1]-pt2[1])/d);
							if ( pt1[0] < pt2[0] ) {
								angle *= -1;
							}
						
							glPushMatrix();
							glTranslatef(pt2[0],pt2[1],0);
							glRotatef(180+angle*180./M_PI,0,0,-1);
							glBegin (GL_LINES);
							glVertex2f(-3,0);
							glVertex2f(3,0);
							glEnd();	
							glPopMatrix();
						}					
					}
					prev_on_screen = on_screen;
					pt1 = pt2;
				}
			}
		}
				
	} else {

		for (unsigned int i=0; i<nb_segment; ++i) {
			
			// Only draw for planets
			if(line_type == TROPIC) {

				// Not valid on non-planets
				if(!(nav->getHomePlanet()->isSatellite()) && nav->getHomePlanet()->getEnglishName() != "Sun") {

					inclination=nav->getHomePlanet()->getAxialTilt()*M_PI/180.;
					for (unsigned int j=0; j<nb_segment+1; ++j) {
						sphe_to_rect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
						points[j+nb_segment+1] *= radius;
						sphe_to_rect((float)j/(nb_segment)*2.f*M_PI, -inclination, points[j+2*nb_segment+2]);
						points[j+2*nb_segment+2] *= radius;
					}
				
#ifdef LSS
					// Draw equator
					if ((prj->*proj_func)(points[i], pt1) &&
						(prj->*proj_func)(points[i+1], pt2) ) {
						
						glBegin (GL_LINES);
						glVertex2f(pt1[0],pt1[1]);
						glVertex2f(pt2[0],pt2[1]);
						glEnd();			  
						
						if((i+1) % 2 == 0) {
							
							const double dx = pt1[0]-pt2[0];
							const double dy = pt1[1]-pt2[1];
							const double dq = dx*dx+dy*dy;
							double angle;
							const double d = sqrt(dq);
							
							angle = acos((pt1[1]-pt2[1])/d);
							if( pt1[0] < pt2[0] ) {
								angle *= -1;
							}
							
							glPushMatrix();
							glTranslatef(pt2[0],pt2[1],0);
							glRotatef(180+angle*180./M_PI,0,0,-1);
							
							glBegin (GL_LINES);
							glVertex2f(-3,0);
							glVertex2f(3,0);
							glEnd();
							
							glPopMatrix();
						}
					}
#endif				

					if((prj->*proj_func)(points[nb_segment+1+i], pt1) 
					   && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {
						
						glBegin (GL_LINES);
						glVertex2f(pt1[0],pt1[1]);
						glVertex2f(pt2[0],pt2[1]);
						glEnd();			  
						if((i+1) % 2 == 0) {

							const double dx = pt1[0]-pt2[0];
							const double dy = pt1[1]-pt2[1];
							const double dq = dx*dx+dy*dy;
							double angle;
							const double d = sqrt(dq);
							      
							angle = acos((pt1[1]-pt2[1])/d);
							if( pt1[0] < pt2[0] ) {
								angle *= -1;
							}

							glPushMatrix();
							glTranslatef(pt2[0],pt2[1],0);
							glRotatef(180+angle*180./M_PI,0,0,-1);
							    
							glBegin (GL_LINES);
							glVertex2f(-3,0);
							glVertex2f(3,0);
							glEnd();

							glPopMatrix();
						}
					
						if( (prj->*proj_func)(points[2*nb_segment+2+i], pt1) 
							&& (prj->*proj_func)(points[2*nb_segment+2+i+1], pt2)) {
						
							glBegin (GL_LINES);
							glVertex2f(pt1[0],pt1[1]);
							glVertex2f(pt2[0],pt2[1]);
							glEnd();
						}
					
						// Draw hour ticks
						if ((i+1) % 2 == 0) drawTick( pt1, pt2);

					}
				}	
			} else if(line_type == CIRCUMPOLAR) {
					inclination=(90.0-abs(nav->get_latitude()))*M_PI/180.;
					if (nav->get_latitude()<0.0) inclination *= -1;
					for (unsigned int j=0; j<nb_segment+1; ++j) {
						sphe_to_rect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
						points[j+nb_segment+1] *= radius;
					}
					if((prj->*proj_func)(points[nb_segment+1+i], pt1) 
					   && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {
						glBegin (GL_LINES);
						glVertex2f(pt1[0],pt1[1]);
						glVertex2f(pt2[0],pt2[1]);
						glEnd();			  
						if((i+1) % 2 == 0) {
							const double dx = pt1[0]-pt2[0];
							const double dy = pt1[1]-pt2[1];
							const double dq = dx*dx+dy*dy;
							double angle;
							const double d = sqrt(dq);
							angle = acos((pt1[1]-pt2[1])/d);
							if( pt1[0] < pt2[0] ) {
								angle *= -1;
							}
							glPushMatrix();
							glTranslatef(pt2[0],pt2[1],0);
							glRotatef(180+angle*180./M_PI,0,0,-1);
							glPopMatrix();
						}
					}
							  
			} else {  // not TROPIC

				if ((prj->*proj_func)(points[i], pt1) &&
					(prj->*proj_func)(points[i+1], pt2) ) {
					const double dx = pt1[0]-pt2[0];
					const double dy = pt1[1]-pt2[1];
					const double dq = dx*dx+dy*dy;
					
					double angle;
					
					// TODO: allow for other numbers of meridians and parallels without
					// screwing up labels?
					
					glBegin (GL_LINES);
					glVertex2f(pt1[0],pt1[1]);
					glVertex2f(pt2[0],pt2[1]);
					glEnd();
					
					// Draw text labels and ticks on meridian
					if (line_type == MERIDIAN) {
						const double d = sqrt(dq);
						
						angle = acos((pt1[1]-pt2[1])/d);
						if ( pt1[0] < pt2[0] ) {
							angle *= -1;
						}

						// draw text label
						std::ostringstream oss;

						if (i<=8) oss << (i+1)*10;
						else if (i<=16) {
							oss << (17-i)*10;
							angle += M_PI;
						} else oss << "";

						glPushMatrix();
						glTranslatef(pt2[0],pt2[1],0);
						glRotatef(180+angle*180./M_PI,0,0,-1);

						glBegin (GL_LINES);
						glVertex2f(-3,0);
						glVertex2f(3,0);
						glEnd();
						glEnable(GL_TEXTURE_2D);

						if (font) font->print(2,-2,oss.str(),1,1);
						glPopMatrix();
						glDisable(GL_TEXTURE_2D);

					}

					// Draw text labels and ticks on equator
					if ((line_type == EQUATOR && (i+1) % 2 == 0)) {

						const double d = sqrt(dq);

						angle = acos((pt1[1]-pt2[1])/d);
						if ( pt1[0] < pt2[0] ) {
							angle *= -1;
						}

						// draw text label
						std::ostringstream oss;

						if ((i+1)/2 == 24) oss << "0h";
						else oss << (i+1)/2 << "h";
						glPushMatrix();
						glTranslatef(pt2[0],pt2[1],0);
						glRotatef(180+angle*180./M_PI,0,0,-1);

						glBegin (GL_LINES);
						glVertex2f(-3,0);
						glVertex2f(3,0);
						glEnd();
						glEnable(GL_TEXTURE_2D);

						if (font) font->print(2,-2,oss.str(),1,1);
						glPopMatrix();
						glDisable(GL_TEXTURE_2D);

					}
				}
			}
		}
	}
	prj->reset_perspective_projection();
}


// Draw small tick marks
void SkyLine::drawTick(Vec3d &pt1, Vec3d &pt2) const
{
	const double dx = pt1[0]-pt2[0];
	const double dy = pt1[1]-pt2[1];
	const double dq = dx*dx+dy*dy;
					
	double angle;
	
	const double d = sqrt(dq);
	
	angle = acos((pt1[1]-pt2[1])/d);
	if ( pt1[0] < pt2[0] ) {
		angle *= -1;
	}

	glPushMatrix();
	glTranslatef(pt2[0],pt2[1],0);
	glRotatef(180+angle*180./M_PI,0,0,-1);
	
	glBegin (GL_LINES);
	glVertex2f(-3,0);
	glVertex2f(3,0);
	glEnd();
	
	glPopMatrix();

}


Cardinals::Cardinals(float _radius) : radius(_radius), font(NULL), color(0.6,0.2,0.2)
{
	// Default labels - if sky locale specified, loaded later
	// Improvement for gettext translation
	sNorth = "N";
	sSouth = "S";
	sEast = "E";
	sWest = "W";
}

Cardinals::~Cardinals()
{
	if (font) delete font;
	font = NULL;
}

/**
 * Set the font for cardinal points
 * @param font_size size in pixel
 * @param font_name name of the font
 */
void Cardinals::set_font(float font_size, const string& font_name)
{
	font = new s_font(font_size, font_name);
	assert(font);
}

// Draw the cardinals points : N S E W
// handles special cases at poles
void Cardinals::draw(const Projector* prj, double latitude, bool gravityON) const
{

	if (!fader.getInterstate()) return;

	// direction text
	string d[4];

	d[0] = sNorth;
	d[1] = sSouth;
	d[2] = sEast;
	d[3] = sWest;

	// fun polar special cases
	if (latitude ==  90.0 ) d[0] = d[1] = d[2] = d[3] = sSouth;
	if (latitude == -90.0 ) d[0] = d[1] = d[2] = d[3] = sNorth;

	glColor4f(color[0],color[1],color[2],fader.getInterstate());
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	// Normal transparency mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3f pos;
	Vec3d xy;

	prj->set_orthographic_projection();

	float shift = font->getStrLen(sNorth)/2;

	if (prj->getFlagGravityLabels()) {
		// N for North
		pos.set(-1.f, 0.f, 0.12f);
		if (prj->project_local(pos,xy)) prj->print_gravity180(font, xy[0], xy[1], d[0], -shift, -shift);

		// S for South
		pos.set(1.f, 0.f, 0.12f);
		if (prj->project_local(pos,xy)) prj->print_gravity180(font, xy[0], xy[1], d[1], -shift, -shift);

		// E for East
		pos.set(0.f, 1.f, 0.12f);
		if (prj->project_local(pos,xy)) prj->print_gravity180(font, xy[0], xy[1], d[2], -shift, -shift);

		// W for West
		pos.set(0.f, -1.f, 0.12f);
		if (prj->project_local(pos,xy)) prj->print_gravity180(font, xy[0], xy[1], d[3], -shift, -shift);
	} else {
		// N for North
		pos.set(-1.f, 0.f, 0.f);
		if (prj->project_local(pos,xy)) font->print(xy[0]-shift, xy[1]-shift, d[0], 1, 1);

		// S for South
		pos.set(1.f, 0.f, 0.f);
		if (prj->project_local(pos,xy)) font->print(xy[0]-shift, xy[1]-shift, d[1], 1, 1);

		// E for East
		pos.set(0.f, 1.f, 0.f);
		if (prj->project_local(pos,xy)) font->print(xy[0]-shift, xy[1]-shift, d[2], 1, 1);

		// W for West
		pos.set(0.f, -1.f, 0.f);
		if (prj->project_local(pos,xy)) font->print(xy[0]-shift, xy[1]-shift, d[3], 1, 1);
	}

	prj->reset_perspective_projection();
}

// Translate cardinal labels with gettext to current sky language
void Cardinals::translateLabels(Translator& trans)
{
	sNorth = trans.translateUTF8("N");
	sSouth = trans.translateUTF8("S");
	sEast = trans.translateUTF8("E");
	sWest = trans.translateUTF8("W");

	if(font) font->clearCache();
}

// Class which manages the displaying of the Milky Way
MilkyWay::MilkyWay(float _radius) : radius(_radius), color(1.f, 1.f, 1.f)
{
	tex = NULL;
	default_tex = NULL;
}

MilkyWay::~MilkyWay()
{
	if(default_tex == tex) default_tex = NULL;
	else delete default_tex;

	if (tex) delete tex;
}

// If tex_file is empty or "default" revert to default texture
// If make_default is true, this new texture will also replace default texture
void MilkyWay::set_texture(const string& tex_file, bool blend, bool make_default)
{
	if (tex && tex != default_tex) delete tex;

	if(default_tex && (tex_file == "" || tex_file == "default")) {

		tex = default_tex;
	} else {
 
		tex = new s_texture(1,tex_file, !blend ? TEX_LOAD_TYPE_PNG_SOLID_REPEAT : TEX_LOAD_TYPE_PNG_BLEND3);

		if(make_default || default_tex == NULL) default_tex = tex;
	}
}


void MilkyWay::set_intensity(float _intensity)
{
	intensity = _intensity;
	SettingsState state;
	state.m_state.milky_way_intensity = _intensity;
	SharedData::Instance()->Settings( state );
}

void MilkyWay::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const
{
	assert(tex);	// A texture must be loaded before calling this

	// .045 chosen so that ad_lum = 1 at standard NELM of 6.5
	float ad_lum=eye->adapt_luminance(.045);

	// NB The tone reproducer code is simply incorrect, so this function 
	// fades out the Milky Way by the time eye limiting mag gets to ~5
	if(ad_lum < .9987 ) 
		ad_lum = -ad_lum*3.6168 +ad_lum*ad_lum*9.6253 -ad_lum*ad_lum*ad_lum*5.0121;

	if(ad_lum < 0) ad_lum = 0;

	// Special case, default texture is drawn dimmer by default so can
	// increase brightness through milky way intensity setting.
	// With user supplied images the expectation is full intensity.
	if(tex == default_tex) ad_lum /= 2.5;

	float cmag = ad_lum * intensity * fader.getInterstate();
	glColor4f(cmag, cmag, cmag, 1);


	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, tex->getID());

	int divisions = 20;
#ifdef LSS
	divisions = 40;
#endif

	// Most textures are in Alpha/Delta and not galactic coordinates
	// To fit perfectly, it must be a fine sphere...
	prj->sSphere(radius,1.0,divisions,divisions,
	             nav->get_j2000_to_eye_mat()*
	             Mat4d::xrotation(M_PI)*
	             Mat4d::yrotation(M_PI)*
	             Mat4d::zrotation(M_PI/180*270), 1);

	glDisable(GL_CULL_FACE);
}


// Draw a point... (used for tests)
void Draw::drawPoint(float X,float Y,float Z)
{
	glColor3f(0.8, 1.0, 0.8);
	glDisable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	glPointSize(20.);
	glBegin(GL_POINTS);
	glVertex3f(X,Y,Z);
	glEnd();
}
