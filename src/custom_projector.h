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

#ifndef _CUSTOM_PROJECTOR_H_
#define _CUSTOM_PROJECTOR_H_

#include "projector.h"

// Class which handle projection modes and projection matrix
// Overide some function usually handled by glu
class CustomProjector : public Projector
{
protected:
	CustomProjector(const Vec4i& viewport, double _fov = 175.);
private:
	// Reimplementation of gluSphere : glu is overrided for non standard projection
	void sSphere(GLdouble radius, GLdouble one_minus_oblateness,
	             GLint slices, GLint stacks,
	             const Mat4d& mat, int orient_inside = 0) const;

	// Draw only a partial sphere with top and/or bottom missing
	void sPartialSphere(GLdouble radius, GLdouble one_minus_oblateness,
	                    GLint slices, GLint stacks,
	                    const Mat4d& mat, int orient_inside = 0,
	                    double bottom_altitude = -90, double top_altitude = 90) const;

	// Reimplementation of gluCylinder : glu is overrided for non standard projection
	void sCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks,
	               const Mat4d& mat, int orient_inside = 0) const;

	// Override glVertex3f and glVertex3d
	void sVertex3(double x, double y, double z, const Mat4d& mat) const;
	void oVertex3(double x, double y, double z, const Mat4d& mat) const;

	const Vec3d convert_pos(const Vec3d& v, const Mat4d& mat) const;
protected:
	// Init the viewing matrix from the fov, the clipping planes and screen ratio
	// The function is a reimplementation of gluPerspective
	void init_project_matrix(void);
};

#endif
