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

#ifndef _FISHEYE_PROJECTOR_H_
#define _FISHEYE_PROJECTOR_H_

#include "custom_projector.h"
#include "external_viewer.h"

class FisheyeProjector : public CustomProjector
{
public:
	FisheyeProjector(const Vec4i & viewport, double _fov = 175.);

	void set_fov(double f);
	// double get_visible_fov(void) const {return vis_fov;}

	void setProjectorConfiguration( int configuration );

// what lens is being used?
	int getLens() const {
		return Lens;
	}


protected:

	typedef struct distortparams {
		float plimit, pfactor, pa, pb, pc;
		float ulimit, ufactor, ua, ub, uc;
		float centerx, centery, radius;
		distortparams() : plimit(0), pfactor(0), pa(0), pb(0), pc(0),
			ulimit(0), ufactor(0), ua(0), ub(0), uc(0), centerx(0), centery(0), radius(0) {}
	} distort_params_t;

	// lens, resolution, projection config
	typedef std::map< int, std::map< int, std::map< int, distort_params_t > > > distort_map_t;
	distort_map_t geometryDistortMap;
	distort_map_t lensDistortMap;

private:
	PROJECTOR_TYPE getType(void) const {
		return FISHEYE_PROJECTOR;
	}
	bool project_custom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const;
	bool project_custom_fixed_fov(const Vec3d &v, Vec3d &win, const Mat4d &mat) const;
	void unproject(double x, double y, const Mat4d& m, Vec3d& v) const;
	bool projectorConfigurationSupported();
	void updateDistortionParameters();

	//  bool check_in_viewport(const Vec3d& pos) const;
// to support large object check
	bool check_in_mask(const Vec3d& pos, const int object_pixel_radius) const;
	int determine_lens(void);
	int determine_distort_params(void);

	int Lens;
	distort_params_t lensDistortion, geometryDistortion;

	double fov_scale;
	double vis_fov;
	double fisheye_scale_factor, fisheye_pixel_diameter;

};


#endif
