/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 *
 * Copyright (C) 2004-2009 Digitalis Education Solutions, Inc.
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

#include "fisheye_projector.h"
#include "utility.h"

FisheyeProjector::FisheyeProjector(const Vec4i& viewport, double _fov)
		:CustomProjector(viewport, _fov)
{
	min_fov = 0.0001;
	max_fov = 359.9;

	determine_distort_params();
	Lens = determine_lens();

	updateDistortionParameters();

	//	fisheye_pixel_diameter = MY_MIN(viewport[2],viewport[3])/2.f;
	//	fisheye_pixel_diameter = viewport_diameter;

	set_fov(_fov);

}

bool FisheyeProjector::project_custom(const Vec3d &v,Vec3d &win,
                                      const Mat4d &mat) const
{
	// optimization by
	// 1) calling atan instead of asin (very good on Intel CPUs)
	// 2) calling sqrt only once
	// Interestingly on my Amd64 asin works slightly faster than atan
	// (although it is done in software!),
	// but the omitted sqrt is still worth it.
	// I think that for calculating win[2] we need no sqrt.
	// Johannes.
	// Fab) Removed one division
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];

	//const double depth = win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();

	if (offset_y) {
		win.normalize();
		win[1] += offset_y * fov/180.f;
		win.normalize();
	}

	// Refactored from SVN 20081002 MappingClasses.hpp
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = M_PI_2 + atan(win[2]*oneoverh);

	double f = a * fisheye_scale_factor *fov_scale;

	// Legacy
	if (Lens == 0) {
		if (f<1.01) f = 1.1798*f -.1889*f*f;
		else f = f - 0.01;
	} else {

		if (f < geometryDistortion.plimit ) {
			f = f*geometryDistortion.pa + f*f*geometryDistortion.pb + f*f*f*geometryDistortion.pc;
		} else {
			f = f + geometryDistortion.pfactor;
		}

		if (f < lensDistortion.plimit ) {
			f = f*lensDistortion.pa + f*f*lensDistortion.pb + f*f*f*lensDistortion.pc;
		} else {
			f = f + lensDistortion.pfactor;
		}
	}

	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + shear_horz * win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*M_PI) ? true : false;

}

bool FisheyeProjector::project_custom_fixed_fov(const Vec3d &v,Vec3d &win,
        const Mat4d &mat) const
{
	// optimization by
	// 1) calling atan instead of asin (very good on Intel CPUs)
	// 2) calling sqrt only once
	// Interestingly on my Amd64 asin works slightly faster than atan
	// (although it is done in software!),
	// but the omitted sqrt is still worth it.
	// I think that for calculating win[2] we need no sqrt.
	// Johannes.
	// Fab) Removed one division
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	const double depth = win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];

	if (offset_y) {
		win.normalize();
		win[1] += offset_y;
		win.normalize();
	}

	// Refactored from SVN 20081002 MappingClasses.hpp
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = M_PI_2 + atan(win[2]*oneoverh);

	// TODO this is not exact, should use init fov
	double f = a / M_PI_2 *fov_scale;


	// Legacy
	if (Lens == 0) {
		if (f<1.01) f = 1.1798*f -.1889*f*f;
		else f = f - 0.01;
	} else {

		if (f < geometryDistortion.plimit ) {
			f = f*geometryDistortion.pa + f*f*geometryDistortion.pb + f*f*f*geometryDistortion.pc;
		} else {
			f = f + geometryDistortion.pfactor;
		}

		if (f < lensDistortion.plimit ) {
			f = f*lensDistortion.pa + f*f*lensDistortion.pb + f*f*f*lensDistortion.pc;
		} else {
			f = f + lensDistortion.pfactor;
		}
	}

	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + shear_horz * win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*M_PI) ? true : false;

}


void FisheyeProjector::unproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double d = getViewportRadius();

	//	printf("unproject x,y: %f, %f   cx,cy: %f, %f\n", x, y, center[0], center[1]);

	v[0] = (x - viewport_center[0])/shear_horz;
	v[1] = y - viewport_center[1];
	v[2] = 0;

	double length = v.length()/d;

	//  printf("viewport radius = %f, length = %f \n", d, length);

	// Legacy
	if (Lens == 0) {
		if (length) length = (sqrt(1.3919-.7556*length/1.0114)-1.1798)/-.3778;
	} else {

		if (length < geometryDistortion.ulimit ) {
			length = length*geometryDistortion.ua + length*length*geometryDistortion.ub
			         + length*length*length*geometryDistortion.uc;
		} else {
			length = length + geometryDistortion.ufactor;
		}

		if (length < lensDistortion.ulimit ) {
			length = length*lensDistortion.ua + length*length*lensDistortion.ub
			         + length*length*length*lensDistortion.uc;
		} else {
			length = length + lensDistortion.ufactor;
		}
	}

	double angle_center = length * fov/2*M_PI/180;
	//double angle_center = length * M_PI_2;

	double r = sin(angle_center);

	if (length!=0) {
		v.normalize();
		v*= r;
		v[2] = sqrt(1.-(v[0]*v[0]+v[1]*v[1]));
	} else {
		v.set(0.,0.,1.);
	}

	if (angle_center>M_PI_2) v[2] = -v[2];

	if (offset_x || offset_y) {

		double offx = -offset_x  * fov/180.f;
		double offy = -offset_y  * fov/180.f;

		double a = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
		double b = 2 * (v[0]*offx + v[1]*offy);
		double c = offx*offx + offy*offy - 1;
		double sq = b*b - 4*a*c;

		if (a == 0) {
			printf("Divide by zero in unproject\n");
			// TEMP TODO what now?
		} else if (sq < 0) {
			printf("imaginary root in unproject\n");
			// TEMP TODO ?
		} else {

			sq = sqrt(sq);
			double root1 = (-b + sq)/(2*a);
			double root2 = (-b - sq)/(2*a);

			//      printf( "roots: %f, %f\n", root1, root2);

			// Use positive root?
			double t = (root1 > root2) ? root1 : root2;

			Vec3d p;
			p[0] = offx + v[0] * t;
			p[1] = offy + v[1] * t;
			p[2] = v[2] * t;

			p.normalize();

			v = p;

		}
	}

	v.transfo4d(m);

	// printf("v after transform = (%f,%f, %f)\n", v[0], v[1], v[2]);

}


// visible may not be same as absolute
void FisheyeProjector::set_fov(double f)
{

	if (f>max_fov) f = max_fov;
	if (f<min_fov) f = min_fov;

	if (Lens == 0 ) {
		vis_fov = f;
		fov = vis_fov * 1.0114;
		fov_scale = 1.0114;

	} else {
		fov = vis_fov = f;
		fov_scale = 1.0;
	}

	init_project_matrix();

	fisheye_scale_factor = 1.0/fov*180./M_PI*2;
	//cout << "FOV = " << fov << endl;
	//cout << "FISHEYE DIAM = " << fisheye_pixel_diameter << endl;
	//cout << "FISHEYE SCALE = " << fisheye_scale_factor << endl;

}


int FisheyeProjector::determine_lens(void)
{

#ifdef DESKTOP
	cout << "Desktop: fisheye\n";
	return(1);
#endif

#ifdef LSS
	cout << "Desktop: fisheye\n";
	return(1);
#endif

	FILE * fic = fopen("/etc/hardware","r");
	if (!fic) {
		printf("# # # Can't determine lens configuration.  Defaulting to lens-0.\n");
		return(0);
	}

	// TODO clean up

	char buff[255];
	while (!feof(fic)) {
		fscanf(fic,"%250s",buff);
		if (!strcmp(buff, "lens-1")) {
			printf("# # # Configuring for lens-1\n");
			fclose(fic);
			return(1);
		}

		if (!strcmp(buff, "lens-2")) {
			printf("# # # Configuring for lens-2\n");
			fclose(fic);
			return(2);
		}

		if (!strcmp(buff, "lens-3")) {
			printf("# # # Configuring for lens-3\n");
			fclose(fic);
			return(3);
		}

		if (!strcmp(buff, "lens-4")) {
			printf("# # # Configuring for lens-4\n");
			fclose(fic);
			return(4);
		}

		if (!strcmp(buff, "lens-5")) {
			printf("# # # Configuring for lens-5\n");
			fclose(fic);
			return(5);
		}

		if (!strcmp(buff, "lens-6")) {
			printf("# # # Configuring for lens-6\n");
			fclose(fic);
			return(6);
		}

		if (!strcmp(buff, "lens-test")) {
			printf("# # # Configuring for lens-test\n");
			fclose(fic);
			return(-1);
		}

	}

	printf("# # # Read configuration, no lens found.  Defaulting to lens-0.\n");
	fclose(fic);
	return(0);
}

void FisheyeProjector::setProjectorConfiguration( int config )
{

	//cout << "* * * Fisheye Projector Config = " << config << endl;
	/*
	if (Lens == 2 && displayH == 1200) {

		if (!config) {
			// set truncated
			setDiskViewport(displayW/2, 672, displayW, displayH, 672);
		} else {
			// set nontruncated
			setDiskViewport(displayW/2, displayH/2, displayW, displayH, displayH/2);
		}
	}
	*/

	ProjectorConfiguration = config;

	updateDistortionParameters();

	if (lensDistortion.radius > 0) {
		// set truncated based on distortion file definition

		float x = (lensDistortion.centerx < 1) ? lensDistortion.centerx*displayW : lensDistortion.centerx;
		float y = (lensDistortion.centery < 1) ? lensDistortion.centery*displayW : lensDistortion.centery;
		float radius = (lensDistortion.radius < 1) ? lensDistortion.radius*displayW : lensDistortion.radius;

		//cout << "Adjusting viewport based on distortion configuration\n" << x << y << radius;

		setDiskViewport(x, y, displayW, displayH, radius);

	} else {
		// otherwise default to standard fulldome
		setDiskViewport(displayW/2, displayH/2, displayW, displayH, displayH/2);
	}

}


// for large objects like planets
// check mask, not just viewport if disk
bool FisheyeProjector::check_in_mask(const Vec3d& pos, const int object_pixel_radius) const
{

	if (maskType == DISK) {
		float radius = sqrt( powf(pos[0]-viewport_center[0], 2) + powf(pos[1]-viewport_center[1], 2));
		return 	(radius - 1.75 * object_pixel_radius <= viewport_radius);  // 1.75 is safety factor
	} else {
		return 	Projector::check_in_mask(pos, object_pixel_radius);
	}

}


// Can different projection configurations be supported for
// the current lens and resolution?
bool FisheyeProjector::projectorConfigurationSupported()
{

	std::map< int, distort_params_t >::iterator iter;

	if (lensDistortMap[Lens].find(displayH) != lensDistortMap[Lens].end() ) {

		iter = (lensDistortMap[Lens][displayH]).find(1);

		if (iter != lensDistortMap[Lens][displayH].end() ) {
			// there is at least a pconfig == 1 for this lens/resolution, so must be configurable
			cout << "Lens " << Lens << " has projector configuration options.\n";
			return 1;
		}
	}

	iter = (lensDistortMap[Lens][0]).find(1);

	if (iter != lensDistortMap[Lens][0].end() ) {
		// there is at least a pconfig == 1 for this lens, so must be configurable
		cout << "Lens " << Lens << " has projector configuration options.\n";
		return 1;
	}

	// No match, not configurable
	cout << "Lens " << Lens << " has no projector configuration options.\n";
	return 0;

}

//! Read distortion.dat file for distortion correction and truncated configuration info

int FisheyeProjector::determine_distort_params(void)
{
	string distortFile = Utility::getDataRoot() + "/data/distortion.dat";

	std::ifstream dfs(distortFile.c_str());

	if (!dfs.is_open()) {
		cout << "Can't open distortion data file " << distortFile << endl;
		return 1;
	}

	string record;
	string lineType, tmpstr;
	int lens=0, resolution=0, pconfig=0;
	float a, b, c, limit, factor;

	int line=0;
	while (!dfs.eof() && std::getline(dfs, record)) {

		line++;
		if(record == "" || record[0] == '#') continue;

		std::istringstream istr(record);

		lineType = "";
		istr >> lineType;

		if(lineType == "lens") {

			if(!(istr >> lens >> tmpstr >> resolution >> tmpstr >> pconfig)) {
				cout << "Error parsing " << distortFile << " on line " << line << endl;
			} else {
				//cout << "Read lens heading " << lens << " " << resolution << " " << pconfig << endl;
			}

		} else if(lineType == "viewport") {

			// Read a viewport definition, can be pixels or fraction of display width if < 1
			if(!(istr >> a >> b >> c)) {
				cout << "Error parsing " << distortFile << " on line " << line << endl;
			} else {

				//	cout << "Read viewport " << a << " " << b << " " << c << endl;
				lensDistortMap[lens][resolution][pconfig].centerx = a;
				lensDistortMap[lens][resolution][pconfig].centery = b;
				lensDistortMap[lens][resolution][pconfig].radius = c;
			}
		} else {

			if(!(istr >> a >> b >> c >> limit >> factor)) {
				cout << "Error parsing " << distortFile << " on line " << line << endl;
				continue;
			}

			//			printf("Read %s %d %f %f %f %f %f\n", lineType.c_str(), lens, a, b, c, limit, factor);

			if(lineType[0] == 'L') {
				if (lineType[1] == 'P') {
					lensDistortMap[lens][resolution][pconfig].pa = a;
					lensDistortMap[lens][resolution][pconfig].pb = b;
					lensDistortMap[lens][resolution][pconfig].pc = c;
					lensDistortMap[lens][resolution][pconfig].plimit = limit;
					lensDistortMap[lens][resolution][pconfig].pfactor = factor;
				}
				if (lineType[1] == 'U') {
					lensDistortMap[lens][resolution][pconfig].ua = a;
					lensDistortMap[lens][resolution][pconfig].ub = b;
					lensDistortMap[lens][resolution][pconfig].uc = c;
					lensDistortMap[lens][resolution][pconfig].ulimit = limit;
					lensDistortMap[lens][resolution][pconfig].ufactor = factor;
				}
			}

			if (lineType[0] == 'G') {
				if (lineType[1] == 'P') {
					geometryDistortMap[lens][resolution][pconfig].pa = a;
					geometryDistortMap[lens][resolution][pconfig].pb = b;
					geometryDistortMap[lens][resolution][pconfig].pc = c;
					geometryDistortMap[lens][resolution][pconfig].plimit = limit;
					geometryDistortMap[lens][resolution][pconfig].pfactor = factor;
				}
				if (lineType[1] == 'U') {
					geometryDistortMap[lens][resolution][pconfig].ua = a;
					geometryDistortMap[lens][resolution][pconfig].ub = b;
					geometryDistortMap[lens][resolution][pconfig].uc = c;
					geometryDistortMap[lens][resolution][pconfig].ulimit = limit;
					geometryDistortMap[lens][resolution][pconfig].ufactor = factor;
				}
			}
		} 
	}

	cout << "Read distortion configuration\n";
	dfs.close();
	return(0);
}

// Caution globals
void FisheyeProjector::updateDistortionParameters()
{
	geometryDistortion = geometryDistortMap[Lens][displayH][ProjectorConfiguration];

	std::map< int, distort_params_t >::iterator iter;

	if (lensDistortMap[Lens].find(displayH) != lensDistortMap[Lens].end() ) {

		iter = (lensDistortMap[Lens][displayH]).find(ProjectorConfiguration);

		if (iter != lensDistortMap[Lens][displayH].end() ) {
			//cout << "resolution and pconfig matched * * * * \n";
			lensDistortion = lensDistortMap[Lens][displayH][ProjectorConfiguration];
		} else {
			//cout << "only resolution matched * * * * \n";
			lensDistortion = lensDistortMap[Lens][0][ProjectorConfiguration];
		}
	} else {
		iter = (lensDistortMap[Lens][0]).find(ProjectorConfiguration);

		if (iter != lensDistortMap[Lens][0].end() ) {
			//cout << "pconfig matched * * * * \n";
			lensDistortion = lensDistortMap[Lens][0][ProjectorConfiguration];
		} else {
			//cout << "default lens distortion * * * * \n";
			lensDistortion = lensDistortMap[Lens][0][0];
		}
	}

	/*
		cout << "Updated lens distortion params:\n";
		cout << "P: " << lensDistortion.pa << " " << lensDistortion.pb << " " << lensDistortion.pc
			 << " " << lensDistortion.plimit << " " << lensDistortion.pfactor << endl;
		cout << "U: " << lensDistortion.ua << " " << lensDistortion.ub << " " << lensDistortion.uc
			 << " " << lensDistortion.ulimit << " " << lensDistortion.ufactor << endl;
	*/

	if (geometryDistortMap[Lens].find(displayH) != geometryDistortMap[Lens].end() ) {

		iter = (geometryDistortMap[Lens][displayH]).find(ProjectorConfiguration);

		if (iter != geometryDistortMap[Lens][displayH].end() ) {
//			cout << "resolution and pconfig matched * * * * \n";
			geometryDistortion = geometryDistortMap[Lens][displayH][ProjectorConfiguration];
		} else {
//			cout << "only resolution matched * * * * \n";
			geometryDistortion = geometryDistortMap[Lens][0][ProjectorConfiguration];
		}
	} else {
//		cout << "default geometry distortion * * * * \n";
		geometryDistortion = geometryDistortMap[Lens][0][0];
	}

	/*
		cout << "Updated geometry distortion params:\n";
		cout << "P: " << geometryDistortion.pa << " " << geometryDistortion.pb << " " << geometryDistortion.pc
			 << " " << geometryDistortion.plimit << " " << geometryDistortion.pfactor << endl;
		cout << "U: " << geometryDistortion.ua << " " << geometryDistortion.ub << " " << geometryDistortion.uc
			 << " " << geometryDistortion.ulimit << " " << geometryDistortion.ufactor << endl;
	*/

#ifdef EXTERNAL_VIEWER_IPC
	char command[101];
	char output_str[101];
	int tipc = open( EXTERNAL_VIEWER_IPC, O_WRONLY | O_NONBLOCK );
	if (tipc!=-1) {
		snprintf(command, 100, "d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f",
		         lensDistortion.pa, lensDistortion.pb, lensDistortion.pc, lensDistortion.plimit, lensDistortion.pfactor,
		         geometryDistortion.pa, geometryDistortion.pb, geometryDistortion.pc, geometryDistortion.plimit, geometryDistortion.pfactor);
		snprintf(output_str, 100, "%-99s\n", command);
		write(tipc, output_str, 100);
		close(tipc);
	}
#endif

}


