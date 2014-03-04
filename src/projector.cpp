/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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
#include <cstdio>
#include "projector.h"
#include "fisheye_projector.h"
#include "stereographic_projector.h"
#include "spheric_mirror_projector.h"
#include "external_viewer.h"
#include "mapping_classes.h"

#ifndef DESKTOP
#include <unistd.h>
#include <fcntl.h>
#endif

const char *Projector::typeToString(PROJECTOR_TYPE type)
{
	switch (type) {
	case PERSPECTIVE_PROJECTOR:
		return "perspective";
	case FISHEYE_PROJECTOR:
		return "fisheye";
	case STEREOGRAPHIC_PROJECTOR:
		return "stereographic";
	case SPHERIC_MIRROR_PROJECTOR:
		return "spheric_mirror";
	}
	cerr << "fatal: Projector::typeToString(" << type << ") failed" << endl;
	assert(0);
	// just shutup the compiler, this point will never be reached
	return 0;
}

Projector::PROJECTOR_TYPE Projector::stringToType(const string &s)
{
	if (s=="perspective")    return PERSPECTIVE_PROJECTOR;
	if (s=="fisheye")        return FISHEYE_PROJECTOR;
	if (s=="stereographic")  return STEREOGRAPHIC_PROJECTOR;
	if (s=="spheric_mirror") return SPHERIC_MIRROR_PROJECTOR;
	cerr << "fatal: Projector::stringToType(" << s << ") failed" << endl;
	assert(0);
	// just shutup the compiler, this point will never be reached
	return PERSPECTIVE_PROJECTOR;
}

const char *Projector::maskTypeToString(PROJECTOR_MASK_TYPE type)
{
	if (type == DISK ) return "disk";
	else return "none";
}

Projector::PROJECTOR_MASK_TYPE Projector::stringToMaskType(const string &s)
{
	if (s=="disk")    return DISK;
	return NONE;
}


Projector *Projector::create(PROJECTOR_TYPE type,
                             const Vec4i& viewport,
                             double _fov)
{
	Projector *rval = 0;
	switch (type) {
	case PERSPECTIVE_PROJECTOR:
		rval = new Projector(viewport,_fov);
		break;
	case FISHEYE_PROJECTOR:
		rval = new FisheyeProjector(viewport,_fov);
		break;
	case STEREOGRAPHIC_PROJECTOR:
		rval = new StereographicProjector(viewport,_fov);
		break;
	case SPHERIC_MIRROR_PROJECTOR:
		rval = new SphericMirrorProjector(viewport,_fov);
		break;
	}

	if (rval == 0) {
		cerr << "fatal: Projector::create(" << type << ") failed" << endl;
		exit(1);
	}

	// Register the default mappings
	rval->registerProjectionMapping(Mapping::Create(Mapping::EQUALAREA));
	rval->registerProjectionMapping(Mapping::Create(Mapping::STEREOGRAPHIC));
	rval->registerProjectionMapping(Mapping::Create(Mapping::CYLINDER));
	rval->registerProjectionMapping(Mapping::Create(Mapping::PERSPECTIVE));
	rval->registerProjectionMapping(Mapping::Create(Mapping::FISHEYE));
	rval->registerProjectionMapping(Mapping::Create(Mapping::ORTHOGRAPHIC));

	string stype(typeToString(type));
	rval->setCurrentProjection(stype);

	return rval;
}

Projector::Projector(const Vec4i& viewport, double _fov)
		:maskType(NONE), fov(1.0), min_fov(0.0001), max_fov(100),
		zNear(0.1), zFar(10000),
		mapping(NULL),
		vec_viewport(viewport),
		flag_auto_zoom(0), gravityLabels(0),
		center_horizontal_offset(0),
		viewport_fov_diameter(std::min<int>(viewport[2],viewport[3]))
{
	ProjectorConfiguration = 0;  // Default
	viewport_radius = -1;  // unset value DIGITALIS
	flip_horz = 1.0;
	flip_vert = 1.0;
	offset_x = offset_y = 0;
	flagGlPointSprite = false;

	setShearHorz( 1.0 );

	setViewport(viewport);
	set_fov(_fov);
}

Projector::~Projector()
{
}

void Projector::registerProjectionMapping(Mapping *c)
{
	if (c) projectionMapping[c->getName()] = c;
}

// Init the viewing matrix, setting the field of view, the clipping planes, and screen ratio
// The function is a reimplementation of gluPerspective
void Projector::init_project_matrix(void)
{
	double f = 1./tan(fov*M_PI/360.);
	double ratio = (double)getViewportHeight()/getViewportWidth();
	mat_projection.set(	flip_horz*f*ratio, 0., 0., 0.,
	                    0., flip_vert*f, 0., 0.,
	                    0., 0., (zFar + zNear)/(zNear - zFar), -1.,
	                    0., 0., (2.*zFar*zNear)/(zNear - zFar), 0.);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(mat_projection);
	glMatrixMode(GL_MODELVIEW);
}

/*************************************************************************
 Set the current projection mapping to use
*************************************************************************/
void Projector::setCurrentProjection(std::string& projectionName)
{
	if (currentProjectionType==projectionName)
		return;

	std::map<std::string,const Mapping*>::const_iterator
		i(projectionMapping.find(projectionName));
	if (i!=projectionMapping.end())
	{
		currentProjectionType = projectionName;

		// Redefine the projection functions
		mapping = i->second;
		min_fov = mapping->minFov;
		max_fov = mapping->maxFov;

		set_fov(fov);
		initGlMatrixOrtho2d();
	}
	else
	{
		cerr << "Unknown projection type: " << projectionName << "." << endl;
	}
}

/*************************************************************************
 Init the real openGL Matrices to a 2d orthographic projection
*************************************************************************/
void Projector::initGlMatrixOrtho2d(void) const
{
	// Set the real openGL projection and modelview matrix to orthographic projection
	// thus we never need to change to 2dMode from now on before drawing
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(vec_viewport[0], vec_viewport[0] + vec_viewport[2],
			vec_viewport[1], vec_viewport[1] + vec_viewport[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Projector::setViewport(int x, int y, int w, int h)
{
	vec_viewport[0] = x;
	vec_viewport[1] = y;
	vec_viewport[2] = w;
	vec_viewport[3] = h;

	viewport_fov_diameter = std::min<int>(w,h);

	// may differ from viewport used, so store
	displayW = w;
	displayH = h;

	// cout << "CALLED NORMAL set viewport\n\n";

	viewport_center.set(vec_viewport[0]+vec_viewport[2]/2,vec_viewport[1]+vec_viewport[3]/2,0);
	//	center.set(vec_viewport[0]+vec_viewport[2]/2,vec_viewport[1]+vec_viewport[3]/4,0);
	pixel_per_rad = 0.5 * fov / (mapping ? mapping->fovToViewScalingFactor(fov*(M_PI/360.0)) : 1.0);
	view_scaling_factor = 1.0/fov*180./M_PI*MY_MIN(getViewportWidth(),getViewportHeight());
	glViewport(x, y, w, h);
	init_project_matrix();
}

// Quickly reset disk viewport to use new center hoffset
// reuses last values for everything else
// NB: must have already called setDiskViewport once
void Projector::resetDiskViewport()
{

	setDiskViewport(last_cx, last_cy, displayW, displayH, last_radius);
}

// allow truncated fisheye projection easily
// This method IS NOT AWARE of projectorConfiguration value
void Projector::setDiskViewport(int cx, int cy, int screenW, int screenH, int radius)
{

	// may differ from viewport used, so store
	displayW = screenW;
	displayH = screenH;
	last_cx = cx;
	last_cy = cy;
	last_radius = radius;

	if (cx-radius+center_horizontal_offset < 0 ) vec_viewport[0] = 0;
	else vec_viewport[0] = cx-radius+center_horizontal_offset;

	if (cy-radius < 0) vec_viewport[1] = 0;
	else vec_viewport[1] = cy-radius;

	if (2*radius > screenW) vec_viewport[2] = screenW;
	else vec_viewport[2] = 2*radius;

	if (2*radius > screenH) vec_viewport[3] = screenH;
	else vec_viewport[3] = 2*radius;

	/*
	  cout << "CALLED DISK set viewport\n\n";
	  cout << "viewport " << vec_viewport[0] << " " << vec_viewport[1] << " " << vec_viewport[2] << " " << vec_viewport[3] << endl;
	  cout << "set center " << cx << " with offset " << center_horizontal_offset << ", " << cy << endl;
	*/

	viewport_center.set(cx+center_horizontal_offset, cy, 0);

	viewport_radius = radius;

#ifdef EXTERNAL_VIEWER_IPC
	char command[50];
	char output_str[101];

// IPC
	int lipc = open( LIRCM_IPC, O_WRONLY | O_NONBLOCK );
	if (lipc!=-1) {
		sprintf(command, "v %5d %5d %5d         \n", cx+center_horizontal_offset, displayH - cy, radius);
		write(lipc, command, 29);
		close(lipc);
	}

	int tipc = open( EXTERNAL_VIEWER_IPC, O_WRONLY | O_NONBLOCK );
	if (tipc!=-1) {
		sprintf(command, "v %5d %5d %5d %5.2f", cx+center_horizontal_offset, displayH - cy, radius, NO_VALUE);
		sprintf(output_str, "%-99s\n", command);
		write(tipc, output_str, 100);
		close(tipc);
	}
#endif

	view_scaling_factor = 1.0/fov*180./M_PI*viewport_radius*2;
	glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	init_project_matrix();
}


void Projector::setShearHorz(double shear)
{

	shear_horz = shear;
	updateShearOffsetIPC();
}

void Projector::setProjectionOffset(double x, double y)
{

	offset_x = x;
	offset_y = y;
	updateShearOffsetIPC();
}

void Projector::updateShearOffsetIPC()
{
#ifdef EXTERNAL_VIEWER_IPC
	char command[50];
	char output_str[101];

	int tipc = open( EXTERNAL_VIEWER_IPC, O_WRONLY | O_NONBLOCK );
	if (tipc!=-1) {
		sprintf(command, "a %4.3f %4.2f %4.2f %4.2f", shear_horz, 1.0, offset_x, -offset_y);
		sprintf(output_str, "%-99s\n", command);
		write(tipc, output_str, 100);
		close(tipc);

	}
#endif
}

void Projector::set_fov(double f)
{
	fov = f;
	if (f>max_fov) fov = max_fov;
	if (f<min_fov) fov = min_fov;

	if (viewport_radius < 0) {
		view_scaling_factor = 1.0/fov*180./M_PI*MY_MIN(getViewportWidth(),getViewportHeight());
	} else {
		// disk viewport
		view_scaling_factor = 1.0/fov*180./M_PI*viewport_radius*2;
	}
	pixel_per_rad = 0.5 * viewport_fov_diameter
	  / (mapping ? mapping->fovToViewScalingFactor(fov*(M_PI/360.0)) : 1.0);
	init_project_matrix();
}

void Projector::setMaxFov(double max)
{
	if (fov > max) set_fov(max);
	max_fov = max;
}

// Fill with black around the circle
void Projector::draw_viewport_shape(void)
{
	if (maskType != DISK) return;

	glDisable(GL_BLEND);
	glColor3f(0.f,0.f,0.f);
	set_orthographic_projection();
	glTranslatef(viewport_center[0],viewport_center[1],0.f);
	GLUquadricObj * p = gluNewQuadric();
	gluDisk(p, viewport_radius, getViewportWidth()+getViewportHeight(), 256, 1);  // should always cover whole screen
	gluDeleteQuadric(p);
	reset_perspective_projection();
}

/*************************************************************************
 Draw the string at the given position and angle with the given font
*************************************************************************/
void Projector::drawText(s_font* const font, float x, float y, const string& str, float angleDeg, float xshift, float yshift, bool noGravity) const
{
	if (gravityLabels && !noGravity)
	{
		print_gravity180(font, x, y, str, true, xshift, yshift);
		return;
	}

	glPushMatrix();
	glTranslatef(x,y,0);
	glRotatef(angleDeg,0,0,1);
	glTranslatef(0,font->getLineHeight(),0);
	font->print(xshift, yshift, str);
	glPopMatrix();
}

/*************************************************************************
 Same function but gives the already projected 2d position in input
*************************************************************************/
void Projector::drawSprite2dMode(double x, double y, double size) const
{
	// Use GL_POINT_SPRITE_ARB extension if available
	if (flagGlPointSprite)
	{
		glPointSize(size);
		glBegin(GL_POINTS);
			glVertex2f(x,y);
		glEnd();
		return;
	}

	const double radius = size*0.5;
	glBegin(GL_QUADS );
		glTexCoord2i(0,0);
		glVertex2f(x-radius,y-radius);
		glTexCoord2i(1,0);
		glVertex2f(x+radius,y-radius);
		glTexCoord2i(1,1);
		glVertex2f(x+radius,y+radius);
		glTexCoord2i(0,1);
		glVertex2f(x-radius,y+radius);
	glEnd();
}

/*************************************************************************
 Same function but with a rotation angle
*************************************************************************/
void Projector::drawSprite2dMode(double x, double y, double size, double rotation) const
{
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glRotatef(rotation,0.,0.,1.);
	const double radius = size*0.5;
	glBegin(GL_QUADS );
		glTexCoord2i(0,0);
		glVertex2f(-radius,-radius);
		glTexCoord2i(1,0);
		glVertex2f(+radius,-radius);
		glTexCoord2i(1,1);
		glVertex2f(+radius,+radius);
		glTexCoord2i(0,1);
		glVertex2f(-radius,+radius);
	glEnd();
	glPopMatrix();
}

/*************************************************************************
 Draw a GL_POINT at the given position
*************************************************************************/
void Projector::drawPoint2d(double x, double y) const
{
	if (flagGlPointSprite)
	{
		glDisable(GL_POINT_SPRITE_ARB);
		glBegin(GL_POINTS);
			glVertex2f(x, y);
		glEnd();
		glEnable(GL_POINT_SPRITE_ARB);
		return;
	}

	glBegin(GL_POINTS);
		glVertex2f(x, y);
	glEnd();
}

/*************************************************************************
 Draw a rotated rectangle using the current texture at the given position
*************************************************************************/
void Projector::drawRectSprite2dMode(double x, double y, double sizex, double sizey, double rotation) const
{
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glRotatef(rotation,0.,0.,1.);
	const double radiusx = sizex*0.5;
	const double radiusy = sizey*0.5;
	glBegin(GL_QUADS );
		glTexCoord2i(0,0);
		glVertex2f(-radiusx,-radiusy);
		glTexCoord2i(1,0);
		glVertex2f(+radiusx,-radiusy);
		glTexCoord2i(1,1);
		glVertex2f(+radiusx,+radiusy);
		glTexCoord2i(0,1);
		glVertex2f(-radiusx,+radiusy);
	glEnd();
	glPopMatrix();
}

StelGeom::ConvexS Projector::unprojectViewport(void) const {
    // This is quite ugly, but already better than nothing.
    // In fact this function should have different implementations
    // for the different mapping types. And maskType, viewport_fov_diameter,
    // viewport_center, viewport_xywh must be taken into account, too.
    // Last not least all halfplanes n*x>d really should have d<=0
    // or at least very small d/n.length().
  if ((dynamic_cast<const MappingCylinder*>(mapping) == 0 || fov < 90) &&
      fov < 360.0) {
    Vec3d e0,e1,e2,e3;
    bool ok;
    if (maskType == DISK) {
      if (fov >= 120.0) {
    	unproject_j2000(viewport_center[0],viewport_center[1],e0);
        StelGeom::ConvexS rval(1);
        rval[0].n = e0;
        rval[0].d = (fov<360.0) ? cos(fov*(M_PI/360.0)) : -1.0;
        return rval;
      }
	  ok  = unproject_j2000(viewport_center[0] - 0.5*viewport_fov_diameter,
                      viewport_center[1] - 0.5*viewport_fov_diameter,e0);
	  ok &= unproject_j2000(viewport_center[0] + 0.5*viewport_fov_diameter,
                      viewport_center[1] + 0.5*viewport_fov_diameter,e2);
	  if (needGlFrontFaceCW()) {
        ok &= unproject_j2000(viewport_center[0] - 0.5*viewport_fov_diameter,
                        viewport_center[1] + 0.5*viewport_fov_diameter,e3);
        ok &= unproject_j2000(viewport_center[0] + 0.5*viewport_fov_diameter,
                        viewport_center[1] - 0.5*viewport_fov_diameter,e1);
	  } else {
        ok &= unproject_j2000(viewport_center[0] - 0.5*viewport_fov_diameter,
                        viewport_center[1] + 0.5*viewport_fov_diameter,e1);
        ok &= unproject_j2000(viewport_center[0] + 0.5*viewport_fov_diameter,
                        viewport_center[1] - 0.5*viewport_fov_diameter,e3);
	  }
    } else {
      ok  = unproject_j2000(vec_viewport[0],vec_viewport[1],e0);
      ok &= unproject_j2000(vec_viewport[0]+vec_viewport[2],
    		  vec_viewport[1]+vec_viewport[3],e2);
      if (needGlFrontFaceCW()) {
        ok &= unproject_j2000(vec_viewport[0],vec_viewport[1]+vec_viewport[3],e3);
        ok &= unproject_j2000(vec_viewport[0]+vec_viewport[2],vec_viewport[1],e1);
      } else {
        ok &= unproject_j2000(vec_viewport[0],vec_viewport[1]+vec_viewport[3],e1);
        ok &= unproject_j2000(vec_viewport[0]+vec_viewport[2],vec_viewport[1],e3);
      }
    }
    if (ok) {
      StelGeom::HalfSpace h0(e0^e1);
      StelGeom::HalfSpace h1(e1^e2);
      StelGeom::HalfSpace h2(e2^e3);
      StelGeom::HalfSpace h3(e3^e0);
      if (h0.contains(e2) && h0.contains(e3) &&
          h1.contains(e3) && h1.contains(e0) &&
          h2.contains(e0) && h2.contains(e1) &&
          h3.contains(e1) && h3.contains(e2)) {
        StelGeom::ConvexS rval(4);
        rval[0] = h0;
        rval[1] = h1;
        rval[2] = h2;
        rval[3] = h3;
        return rval;
      } else {
        Vec3d middle;
        if (unproject_j2000(vec_viewport[0]+0.5*vec_viewport[2],
        		vec_viewport[1]+0.5*vec_viewport[3],middle)) {
          double d = middle*e0;
          double h = middle*e1;
          if (d > h) d = h;
          h = middle*e2;
          if (d > h) d = h;
          h = middle*e3;
          if (d > h) d = h;
          StelGeom::ConvexS rval(1);
          rval[0].n = middle;
          rval[0].d = d;
          return rval;
        }
      }
    }
  }
  StelGeom::ConvexS rval(1);
  rval[0].n = Vec3d(1.0,0.0,0.0);
  rval[0].d = -2.0;
  return rval;
}

void Projector::set_clipping_planes(double znear, double zfar)
{
	zNear = znear;
	zFar = zfar;
	init_project_matrix();
}


bool Projector::check_in_viewport(const Vec3d& pos) const
{
	return 	(pos[1]>vec_viewport[1] && pos[1]<(vec_viewport[1] + vec_viewport[3]) &&
	         pos[0]>vec_viewport[0] && pos[0]<(vec_viewport[0] + vec_viewport[2]));
}

// to support large object check
bool Projector::check_in_mask(const Vec3d& pos, const int object_pixel_radius) const
{
	// No mask in plain projection
	return 	(pos[1]+object_pixel_radius>vec_viewport[1] &&
	         pos[1]-object_pixel_radius<(vec_viewport[1] + vec_viewport[3]) &&
	         pos[0]+object_pixel_radius>vec_viewport[0] &&
	         pos[0]-object_pixel_radius<(vec_viewport[0] + vec_viewport[2]));
}



void Projector::change_fov(double deltaFov)
{
	// if we are zooming in or out
	if (deltaFov) set_fov(fov+deltaFov);
}


// Set the standard modelview matrices used for projection
void Projector::set_modelview_matrices(	const Mat4d& _mat_earth_equ_to_eye,
                                        const Mat4d& _mat_helio_to_eye,
                                        const Mat4d& _mat_local_to_eye,
                                        const Mat4d& _mat_j2000_to_eye,
                                        const Mat4d& _mat_galactic_to_eye,
                                        const Mat4d& _mat_dome,
                                        const Mat4d& _mat_dome_fixed)
{
	mat_earth_equ_to_eye = _mat_earth_equ_to_eye;
	mat_j2000_to_eye = _mat_j2000_to_eye;
	mat_galactic_to_eye = _mat_galactic_to_eye;
	mat_helio_to_eye = _mat_helio_to_eye;
	mat_local_to_eye = _mat_local_to_eye;
	mat_dome = _mat_dome;
	mat_dome_fixed = _mat_dome_fixed;

	inv_mat_earth_equ_to_eye = (mat_projection*mat_earth_equ_to_eye).inverse();
	inv_mat_j2000_to_eye = (mat_projection*mat_j2000_to_eye).inverse();
	inv_mat_galactic_to_eye = (mat_projection*mat_galactic_to_eye).inverse();
	inv_mat_helio_to_eye = (mat_projection*mat_helio_to_eye).inverse();
	inv_mat_local_to_eye = (mat_projection*mat_local_to_eye).inverse();
	inv_mat_dome = (mat_projection*mat_dome).inverse();
	inv_mat_dome_fixed = (mat_projection*mat_dome_fixed).inverse();
}

// Update auto_zoom if activated
void Projector::update_auto_zoom(int delta_time, bool manual_zoom)
{
	if (flag_auto_zoom) {
		// Use a smooth function
		double c;

// - manual zoom out (semi auto actually) requires fast at start to be smooth
		if ( manual_zoom || zoom_move.start > zoom_move.aim ) {
			// slow down as approach final view
			c = 1 - (1-zoom_move.coef)*(1-zoom_move.coef)*(1-zoom_move.coef);
		} else {
			// speed up as leave zoom target
			c = (zoom_move.coef)*(zoom_move.coef)*(zoom_move.coef);
		}

		set_fov(zoom_move.start + (zoom_move.aim - zoom_move.start) * c);
		zoom_move.coef+=zoom_move.speed*delta_time;
		if (zoom_move.coef>=1.) {
			flag_auto_zoom = 0;
			set_fov(zoom_move.aim);
		}
	}
	/*
	if (flag_auto_zoom)
	{
		// Use a smooth function
		float smooth = 3.f;
		double c = atanf(smooth * 2.*zoom_move.coef-smooth)/atanf(smooth)/2+0.5;
		set_fov(zoom_move.start + (zoom_move.aim - zoom_move.start) * c);
	    zoom_move.coef+=zoom_move.speed*delta_time;
	    if (zoom_move.coef>=1.)
	    {
			flag_auto_zoom = 0;
	        set_fov(zoom_move.aim);
	    }
	}*/
}

// Zoom to the given field of view
void Projector::zoom_to(double aim_fov, float move_duration)
{

	if ( flag_auto_zoom && fabs(zoom_move.aim - aim_fov) <= .0000001f ) {
		//    cout << "Already zooming here\n";
		return;  // already zooming to this fov!
	}

	zoom_move.aim=aim_fov;
	zoom_move.start=fov;
	zoom_move.speed=1.f/(move_duration*1000);
	zoom_move.coef=0.;
	flag_auto_zoom = true;
}


// Set the drawing mode in 2D. Use reset_perspective_projection() to reset
// previous projection mode
void Projector::set_orthographic_projection(void) const
{
	glMatrixMode(GL_PROJECTION);		// projection matrix mode
	glPushMatrix();						// store previous matrix
	glLoadIdentity();
	gluOrtho2D(	vec_viewport[0], vec_viewport[0] + vec_viewport[2],
	            vec_viewport[1], vec_viewport[1] + vec_viewport[3]);	// set a 2D orthographic projection
	glMatrixMode(GL_MODELVIEW);			// modelview matrix mode
	glPushMatrix();
	glLoadIdentity();
}

// Reset the previous projection mode after a call to set_orthographic_projection()
void Projector::reset_perspective_projection(void) const
{
	glMatrixMode(GL_PROJECTION);		// Restore previous matrix
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}



// Reimplementation of gluSphere : glu is overrided for non standard projection
// Novgorod version with shader support

void Projector::sSphere(GLdouble radius, GLdouble scale, GLdouble one_minus_oblateness,
                        GLint slices, GLint stacks,
                        const Mat4d& mat, int orient_inside, bool shader) const
{
    glPushMatrix();
    glLoadMatrixd(mat);

      // It is really good for performance to have Vec4f,Vec3f objects
      // static rather than on the stack. But why?
      // Is the constructor/destructor so expensive?
    static Vec4f lightPos4;
    static Vec3f lightPos3;
    GLboolean isLightOn;
    static Vec3f transNorm;
    float c;

    static Vec4f ambientLight;
    static Vec4f diffuseLight;

	double scaledRadius = radius * scale;

    glGetBooleanv(GL_LIGHTING, &isLightOn);

    if (isLightOn)
    {
        glGetLightfv(GL_LIGHT0, GL_POSITION, lightPos4);
        lightPos3 = lightPos4;
        lightPos3 -= mat * Vec3d(0.,0.,0.); // -posCenterEye
        lightPos3.normalize();
        glGetLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glGetLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glDisable(GL_LIGHTING);
    }

    GLfloat x, y, z;
    GLfloat s, t;
    GLint i, j;
    GLfloat nsign;

    if (orient_inside) {
      nsign = -1.0;
      t=0.0; // from inside texture is reversed
    } else {
      nsign = 1.0;
      t=1.0;
    }

    const GLfloat drho = M_PI / (GLfloat) stacks;
    double cos_sin_rho[2*(stacks+1)];
    double *cos_sin_rho_p = cos_sin_rho;
    for (i = 0; i <= stacks; i++) {
      double rho = i * drho;
      *cos_sin_rho_p++ = cos(rho);
      *cos_sin_rho_p++ = sin(rho);
    }

    const GLfloat dtheta = 2.0 * M_PI / (GLfloat) slices;
    double cos_sin_theta[2*(slices+1)];
    double *cos_sin_theta_p = cos_sin_theta;
    for (i = 0; i <= slices; i++) {
      double theta = (i == slices) ? 0.0 : i * dtheta;
      *cos_sin_theta_p++ = cos(theta);
      *cos_sin_theta_p++ = sin(theta);
    }

    // texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
    // t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
    // cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
    const GLfloat ds = 1.0 / slices;
    const GLfloat dt = nsign / stacks; // from inside texture is reversed

    // draw intermediate as quad strips
    for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks;
         i++,cos_sin_rho_p+=2)
    {
        glBegin(GL_QUAD_STRIP);
        s = 0.0;
        for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices;
             j++,cos_sin_theta_p+=2)
        {
            x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
            y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
            z = nsign * cos_sin_rho_p[0];
            glTexCoord2f(s, t);
			if(shader) 
				glNormal3f(x, y, z);
            if (isLightOn)
            {
                transNorm = mat.multiplyWithoutTranslation(
                                  Vec3d(x * one_minus_oblateness * nsign,
                                        y * one_minus_oblateness * nsign,
                                        z * nsign));
                c = lightPos3.dot(transNorm);
                if (c<0) c=0;
                //kornyakov: planet fading
				//Ljubov: something for shaders
			    if (shader) 
					glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
				else
					glColor3f((c*diffuseLight[0] + ambientLight[0]),
                              (c*diffuseLight[1] + ambientLight[1]),
                              (c*diffuseLight[2] + ambientLight[2])); 
            }
            sVertex3(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
            x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
            y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
            z = nsign * cos_sin_rho_p[2];
            glTexCoord2f(s, t - dt);
		    if(shader) 
				glNormal3f(x, y, z);
            if (isLightOn)
            {
                transNorm = mat.multiplyWithoutTranslation(
                                  Vec3d(x * one_minus_oblateness * nsign,
                                        y * one_minus_oblateness * nsign,
                                        z * nsign));
                c = lightPos3.dot(transNorm);
                if (c<0) c=0;
                //kornyakov: planet fading
                //Ljubov: something for shaders
			    if (shader) 
					glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
				else
					glColor3f((c*diffuseLight[0] + ambientLight[0]),
                              (c*diffuseLight[1] + ambientLight[1]),
                              (c*diffuseLight[2] + ambientLight[2])); 
            }
            sVertex3(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
            s += ds;
        }
        glEnd();
        t -= dt;
    }
    glPopMatrix();
    if (isLightOn) glEnable(GL_LIGHTING);

}


// save on spherical texture sizes since a lot is transparent
// Draw only a partial sphere with top and/or bottom missing

// bottom_altitude is angle above (- for below) horizon for bottom of texture
// top_altitude is altitude angle for top of texture
// both are in degrees

void Projector::sPartialSphere(GLdouble radius, GLdouble one_minus_oblateness,
                               GLint slices, GLint stacks,
                               const Mat4d& mat, int orient_inside,
                               double bottom_altitude, double top_altitude ) const
{
	glPushMatrix();
	glLoadMatrixd(mat);

	//GLfloat rho, theta;
	GLfloat x, y, z;
	GLfloat s, t, ds, dt;
	GLint i, j;
	GLfloat nsign;

	double bottom = M_PI / 180. * bottom_altitude;
	double angular_height = M_PI / 180. * top_altitude - bottom;


	if (orient_inside) nsign = -1.0;
	else nsign = 1.0;

	const GLfloat drho = angular_height / (GLfloat) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = M_PI_2 + bottom + i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const GLfloat dtheta = 2.0 * M_PI / (GLfloat) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	ds = 1.0 / slices;
	dt = 1.0 / stacks;
	t = 1.0;            // because loop now runs from 0

	// draw intermediate stacks as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks;
	        i++,cos_sin_rho_p+=2) {
		glBegin(GL_QUAD_STRIP);
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices;
		        j++,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = nsign * cos_sin_rho_p[0];
			glNormal3f(x * one_minus_oblateness * nsign,
			           y * one_minus_oblateness * nsign,
			           z * nsign);
			glTexCoord2f(s, t);
			sVertex3(x * radius,
			         y * radius,
			         one_minus_oblateness * z * radius, mat);
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = nsign * cos_sin_rho_p[2];
			glNormal3f(x * one_minus_oblateness * nsign,
			           y * one_minus_oblateness * nsign,
			           z * nsign);
			glTexCoord2f(s, t - dt);
			s += ds;
			sVertex3(x * radius,
			         y * radius,
			         one_minus_oblateness * z * radius, mat);
		}
		glEnd();
		t -= dt;
	}

	glPopMatrix();
}

/*

	if (orient_inside) nsign = -1.0;
	else nsign = 1.0;

//
	const GLfloat drho = M_PI / (GLfloat) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const GLfloat dtheta = 2.0 * M_PI / (GLfloat) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	ds = 1.0 / slices;
	dt = 1.0 / stacks;
	t = 1.0;            // because loop now runs from 0

	// draw intermediate stacks as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks;
		 i++,cos_sin_rho_p+=2)
	{
		glBegin(GL_QUAD_STRIP);
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices;
			 j++,cos_sin_theta_p+=2)
		{
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = nsign * cos_sin_rho_p[0];
			glNormal3f(x * one_minus_oblateness * nsign,
					   y * one_minus_oblateness * nsign,
					   z * nsign);
			glTexCoord2f(s, t);
			sVertex3(x * radius,
					 y * radius,
					 one_minus_oblateness * z * radius, mat);
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = nsign * cos_sin_rho_p[2];
			glNormal3f(x * one_minus_oblateness * nsign,
					   y * one_minus_oblateness * nsign,
					   z * nsign);
			glTexCoord2f(s, t - dt);
			s += ds;
			sVertex3(x * radius,
					 y * radius,
					 one_minus_oblateness * z * radius, mat);
		}
		glEnd();
		t -= dt;
	}


	glPopMatrix();
}
*/


// Draw a half sphere
void Projector::sHalfSphere(GLdouble radius, GLint slices, GLint stacks,
                            const Mat4d& mat, int orient_inside) const
{
	glPushMatrix();
	glLoadMatrixd(mat);

	GLfloat rho, drho, theta, dtheta;
	GLfloat x, y, z;
	GLfloat s, t, ds, dt;
	GLint i, j, imin, imax;
	GLfloat nsign;

	if (orient_inside) nsign = -1.0;
	else nsign = 1.0;

	drho = M_PI / (GLfloat) stacks;
	dtheta = 2.0 * M_PI / (GLfloat) slices;

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	ds = 1.0 / slices;
	dt = 1.0 / stacks;
	t = 1.0;			// because loop now runs from 0
	imin = 0;
	imax = stacks;

	// draw intermediate stacks as quad strips
	for (i = imin; i < imax/2; i++) {
		rho = i * drho;
		glBegin(GL_QUAD_STRIP);
		s = 0.0;
		for (j = 0; j <= slices; j++) {
			theta = (j == slices) ? 0.0 : j * dtheta;
			x = -sin(theta) * sin(rho);
			y = cos(theta) * sin(rho);
			z = nsign * cos(rho);
			glNormal3f(x * nsign, y * nsign, z * nsign);
			glTexCoord2f(s, t);
			sVertex3(x * radius, y * radius, z * radius, mat);
			x = -sin(theta) * sin(rho + drho);
			y = cos(theta) * sin(rho + drho);
			z = nsign * cos(rho + drho);
			glNormal3f(x * nsign, y * nsign, z * nsign);
			glTexCoord2f(s, t - dt);
			s += ds;
			sVertex3(x * radius, y * radius, z * radius, mat);
		}
		glEnd();
		t -= dt;
	}
	glPopMatrix();
}

// Draw a disk with a special texturing mode having texture center at disk center
void Projector::sDisk(GLdouble radius, GLint slices, GLint stacks,
                      const Mat4d& mat, int orient_inside) const
{
	glPushMatrix();
	glLoadMatrixd(mat);

	GLfloat r, dr, theta, dtheta;
	GLfloat x, y;
	GLint j;
	GLfloat nsign;

	if (orient_inside) nsign = -1.0;
	else nsign = 1.0;

	dr = radius / (GLfloat) stacks;
	dtheta = 2.0 * M_PI / (GLfloat) slices;
	if (slices < 0) slices = -slices;

	// draw intermediate stacks as quad strips
	for (r = 0; r < radius; r+=dr) {
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j <= slices; j++) {
			theta = (j == slices) ? 0.0 : j * dtheta;
			x = r*cos(theta);
			y = r*sin(theta);
			glNormal3f(0, 0, nsign);
			glTexCoord2f(0.5+x/2/radius, 0.5+y/2/radius);
			sVertex3(x, y, 0, mat);
			x = (r+dr)*cos(theta);
			y = (r+dr)*sin(theta);
			glNormal3f(0, 0, nsign);
			glTexCoord2f(0.5+x/2/radius, 0.5+y/2/radius);
			sVertex3(x, y, 0, mat);
		}
		glEnd();
	}
	glPopMatrix();
}

void Projector::sRing(GLdouble r_min, GLdouble r_max,
                      GLint slices, GLint stacks,
                      const Mat4d& mat, int orient_inside, bool shader) const
{
	glPushMatrix();
	glLoadMatrixd(mat);

	double theta;
	double x,y;
	int j;

	const double nsign = (orient_inside)?-1.0:1.0;

	const double dr = (r_max-r_min) / stacks;
	const double dtheta = 2.0 * M_PI / slices;
	if (slices < 0) slices = -slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (j = 0; j <= slices; j++) {
		const double theta = (j == slices) ? 0.0 : j * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// draw intermediate stacks as quad strips
	for (double r = r_min; r < r_max; r+=dr) {
		const double tex_r0 = (r-r_min)/(r_max-r_min);
		const double tex_r1 = (r+dr-r_min)/(r_max-r_min);
		glBegin(GL_QUAD_STRIP /*GL_TRIANGLE_STRIP*/);
		for (j=0,cos_sin_theta_p=cos_sin_theta;
		        j<=slices;
		        j++,cos_sin_theta_p+=2) {
			theta = (j == slices) ? 0.0 : j * dtheta;
			x = r*cos_sin_theta_p[0];
			y = r*cos_sin_theta_p[1];
			glNormal3d(0, 0, nsign);
			glTexCoord2d(tex_r0, 0.5);
			if (shader) glColor3f(x,y,0);
			sVertex3(x, y, 0, mat);
			x = (r+dr)*cos_sin_theta_p[0];
			y = (r+dr)*cos_sin_theta_p[1];
			glNormal3d(0, 0, nsign);
			glTexCoord2d(tex_r1, 0.5);
			if (shader) glColor3f(x,y,0);
			sVertex3(x, y, 0, mat);
		}
		glEnd();
	}
	glPopMatrix();
}

static
inline void sSphereMapTexCoordFast(double rho_div_fov,
                                   double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	glTexCoord2d(0.5 + rho_div_fov * costheta,
	             0.5 + rho_div_fov * sintheta);
}

void Projector::sSphere_map(GLdouble radius, GLint slices, GLint stacks,
                            const Mat4d& mat, double texture_fov,
                            int orient_inside) const
{
	glPushMatrix();
	glLoadMatrixd(mat);

	double rho,x,y,z;
	int i, j;
	const double nsign = orient_inside?-1:1;

	const double drho = M_PI / stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		const double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const double dtheta = 2.0 * M_PI / slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		const double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)

	const int imax = stacks;

	// draw intermediate stacks as quad strips
	if (!orient_inside) { // nsign==1
		for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0;
		        i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {
			glBegin(GL_QUAD_STRIP);
			for (j=0,cos_sin_theta_p=cos_sin_theta;
			        j<=slices; ++j,cos_sin_theta_p+=2) {
				x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
				y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
				z = cos_sin_rho_p[0];
				glNormal3d(x * nsign, y * nsign, z * nsign);
				sSphereMapTexCoordFast(rho/texture_fov,
				                       cos_sin_theta_p[0],
				                       cos_sin_theta_p[1]);
				sVertex3(x * radius, y * radius, z * radius, mat);

				x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
				y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
				z = cos_sin_rho_p[2];
				glNormal3d(x * nsign, y * nsign, z * nsign);
				sSphereMapTexCoordFast((rho + drho)/texture_fov,
				                       cos_sin_theta_p[0],
				                       cos_sin_theta_p[1]);
				sVertex3(x * radius, y * radius, z * radius, mat);
			}
			glEnd();
		}
	} else {
		for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0;
		        i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {
			glBegin(GL_QUAD_STRIP);
			for (j=0,cos_sin_theta_p=cos_sin_theta;
			        j<=slices; ++j,cos_sin_theta_p+=2) {
				x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
				y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
				z = cos_sin_rho_p[2];
				glNormal3d(x * nsign, y * nsign, z * nsign);
				sSphereMapTexCoordFast((rho + drho)/texture_fov,
				                       cos_sin_theta_p[0],
				                       -cos_sin_theta_p[1]);
				sVertex3(x * radius, y * radius, z * radius, mat);

				x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
				y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
				z = cos_sin_rho_p[0];
				glNormal3d(x * nsign, y * nsign, z * nsign);
				sSphereMapTexCoordFast(rho/texture_fov,
				                       cos_sin_theta_p[0],
				                       -cos_sin_theta_p[1]);
				sVertex3(x * radius, y * radius, z * radius, mat);
			}
			glEnd();
		}
	}
	glPopMatrix();
}


// Reimplementation of gluCylinder : glu is overrided for non standard projection
void Projector::sCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks, const Mat4d& mat, int orient_inside) const
{
	glPushMatrix();
	glLoadMatrixd(mat);
	GLUquadricObj * p = gluNewQuadric();
	gluQuadricTexture(p,GL_TRUE);
	if (orient_inside) {
		glCullFace(GL_FRONT);
	}
	gluCylinder(p, radius, radius, height, slices, stacks);
	gluDeleteQuadric(p);
	glPopMatrix();
	if (orient_inside) {
		glCullFace(GL_BACK);
	}
}


void Projector::print_gravity180(s_font* font, float x, float y, const string& str,
                                 bool speed_optimize, float xshift, float yshift) const
{
	static float dx, dy, d, theta, psi;

	//	dx = x - (vec_viewport[0] + vec_viewport[2]/2);
	//	dy = y - (vec_viewport[1] + vec_viewport[3]/2);

	// ASSUME disk viewport
	dx = x - viewport_center[0];
	dy = y - viewport_center[1];
	d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if (d>MY_MAX(vec_viewport[3], vec_viewport[2])*2) return;


	theta = M_PI + atan2f(dx, dy - 1);
	psi = atan2f((float)font->getStrLen(str)/str.length(),d + 1) * 180./M_PI;

	if (psi>5) psi = 5;

	set_orthographic_projection();

	glTranslatef(x,y,0);
	if (gravityLabels) glRotatef(theta*180./M_PI,0,0,-1);
	glTranslatef(xshift, -yshift, 0);
	glScalef(1, -1, 1);

	font->print(0, 0, str, 0, speed_optimize);  // ASSUME speed optimized strings should be cached

	reset_perspective_projection();

}

