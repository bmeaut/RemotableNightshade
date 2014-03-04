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

#ifndef _PROJECTOR_H_
#define _PROJECTOR_H_

#include "nightshade.h"
#include "vecmath.h"
#include "s_font.h"
#include "mapping.h"
#include "sphere_geometry.h"

class s_font;

// Class which handle projection modes and projection matrix
// Overide some function usually handled by glu
class Projector
{
public:
	enum PROJECTOR_TYPE {
		PERSPECTIVE_PROJECTOR    = 0,
		FISHEYE_PROJECTOR        = 1,
		STEREOGRAPHIC_PROJECTOR  = 2,
		SPHERIC_MIRROR_PROJECTOR = 3
	};

	enum FRAME_TYPE
	{
		FRAME_LOCAL,
		FRAME_HELIO,
		FRAME_EARTH_EQU,
		FRAME_J2000
	};

	enum PROJECTOR_MASK_TYPE {
		DISK,
		NONE
	};

	static const char *typeToString(PROJECTOR_TYPE type);
	static PROJECTOR_TYPE stringToType(const string &s);

	static const char *maskTypeToString(PROJECTOR_MASK_TYPE type);
	static PROJECTOR_MASK_TYPE stringToMaskType(const string &s);


	// Main factory constructor
	static Projector *create(PROJECTOR_TYPE type,
	                         const Vec4i& viewport,
	                         double _fov = 60.);
	virtual ~Projector();

	virtual PROJECTOR_TYPE getType(void) const {
		return PERSPECTIVE_PROJECTOR;
	}

	//! Get the type of the mask if any
	PROJECTOR_MASK_TYPE getMaskType(void) const {
		return maskType;
	}
	void setMaskType(PROJECTOR_MASK_TYPE m) {
		maskType = m;
	}

	// Quickly reset disk viewport with new center hoffset
	void resetDiskViewport();

	//! Get and set to define and get viewport size
	virtual void setDiskViewport(int cx, int cy, int screenW, int screenH, int diameter);
	Vec3d getViewportCenter(void) {
		return Vec3d(viewport_center[0], viewport_center[1], viewport_radius);
	}
	double getViewportRadius(void) const {
		return viewport_radius;
	}

	// Can projector support different configurations?
	virtual bool projectorConfigurationSupported() {
		return 0;
	}

// what lens is being used?
	virtual int getLens() const {
		return 0;
	}

	virtual void setViewport(int x, int y, int w, int h);
	void setViewport(const Vec4i& v) {
		setViewport(v[0], v[1], v[2], v[3]);
	}
	void setViewportPosX(int x) {
		setViewport(x, vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	}
	void setViewportPosY(int y) {
		setViewport(vec_viewport[0], y, vec_viewport[2], vec_viewport[3]);
	}
	void setViewportWidth(int width) {
		setViewport(vec_viewport[0], vec_viewport[1], width, vec_viewport[3]);
	}
	void setViewportHeight(int height) {
		setViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], height);
	}
	int getViewportPosX(void) const {
		return vec_viewport[0];
	}
	int getViewportPosY(void) const {
		return vec_viewport[1];
	}
	int getViewportWidth(void) const {
		return vec_viewport[2];
	}
	int getViewportHeight(void) const {
		return vec_viewport[3];
	}
	const Vec4i& getViewport(void) const {
		return vec_viewport;
	}

	//! Get display width
	int getDisplayWidth(void) const {
		return displayW;
	}

	//! Get display height
	int getDisplayHeight(void) const {
		return displayH;
	}


	// Do not change viewport directly
	void setCenterHorizontalOffset(int h) {
		center_horizontal_offset = h;
		if (maskType == DISK) resetDiskViewport(); // sanity check
	}
	int getCenterHorizontalOffset() {
		return center_horizontal_offset;
	}

	virtual void setProjectorConfiguration( int configuration ) {
		ProjectorConfiguration = configuration;
		// cout << "* * * Projector Configuration = " << configuration << endl;
	}
	virtual int getProjectorConfiguration() {
		return ProjectorConfiguration;
	}

	//! Set the current openGL viewport to projector's viewport
	void applyViewport(void) const {
		glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	}


	bool getFlipHorz(void) const {
		return (flip_horz < 0.0);
	}
	bool getFlipVert(void) const {
		return (flip_vert < 0.0);
	}
	void setFlipHorz(bool flip) {
		flip_horz = flip ? -1.0 : 1.0;
		init_project_matrix();
	}
	void setFlipVert(bool flip) {
		flip_vert = flip ? -1.0 : 1.0;
		init_project_matrix();
	}
	virtual bool needGlFrontFaceCW(void) const {
		return (flip_horz*flip_vert < 0.0);
	}

	double getShearHorz(void) const {
		return shear_horz;
	}
	void setShearHorz(double shear);

	double getOffsetX(void) const {
		return offset_x;
	}
	double getOffsetY(void) const {
		return offset_y;
	}
	void setProjectionOffset(double x, double y);

	void updateShearOffsetIPC(void);

	//! Set the Field of View in degree
	virtual void set_fov(double f);
	//! Get the Field of View in degree
	double get_fov(void) const {
		return fov;
	}
	double getRadPerPixel(void) const {
		return view_scaling_factor;
	}

	//! Set the maximum Field of View in degree
	void setMaxFov(double max);
	//! Get the maximum Field of View in degree
	double getMaxFov(void) const {
		return max_fov;
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov(void) const {
		return (flag_auto_zoom ? zoom_move.aim : fov);
	}

	void change_fov(double deltaFov);

	// Update auto_zoom if activated
	void update_auto_zoom(int delta_time, bool manual_zoom = 0);

	// Zoom to the given field of view in degree
	void zoom_to(double aim_fov, float move_duration = 1.);

	// Fill with black around the circle
	void draw_viewport_shape(void);

	//! Draw the (multi-byte) string at the given position and angle with the given font.
	//! If the gravity label flag is set, uses drawTextGravity180.
	//! @param x horizontal position of the lower left corner of the first character of the text in pixel.
	//! @param y horizontal position of the lower left corner of the first character of the text in pixel.
	//! @param str the text to print.
	//! @param angleDeg rotation angle in degree. Rotation is around x,y.
	//! @param xshift shift in pixel in the rotated x direction.
	//! @param yshift shift in pixel in the rotated y direction.
	//! @param noGravity don't take into account the fact that the text should be written with gravity.
	void drawText(s_font* const font, float x, float y, const string& str, float angleDeg=0.f,
		      float xshift=0.f, float yshift=0.f, bool noGravity=true) const;

	void set_clipping_planes(double znear, double zfar);
	void get_clipping_planes(double* zn, double* zf) const {
		*zn = zNear;
		*zf = zFar;
	}

	// Return true if the 2D pos is inside the viewport
	bool check_in_viewport(const Vec3d& pos) const;

// for fisheye disk checking
	virtual bool check_in_mask(const Vec3d& pos, const int object_pixel_radius) const;

	// Set the standard modelview matrices used for projection
	void set_modelview_matrices(const Mat4d& _mat_earth_equ_to_eye,
	                            const Mat4d& _mat_helio_to_eye,
	                            const Mat4d& _mat_local_to_eye,
	                            const Mat4d& _mat_j2000_to_eye,
								const Mat4d& _mat_galactic_to_eye,
	                            const Mat4d& _mat_dome,
	                            const Mat4d& _mat_dome_fixed);

	// Return in vector "win" the projection on the screen of point v in earth equatorial coordinate
	// according to the current modelview and projection matrices (reimplementation of gluProject)
	// Return true if the z screen coordinate is < 1, ie if it isn't behind the observer
	// except for the _check version which return true if the projected point is inside the screen
	inline bool project_earth_equ(const Vec3d& v, Vec3d& win) const {
		return project_custom(v, win, mat_earth_equ_to_eye);
	}

	inline bool project_earth_equ_check(const Vec3d& v, Vec3d& win) const {
		return project_custom_check(v, win, mat_earth_equ_to_eye);
	}

	inline bool project_earth_equ_line_check(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const {
		return project_custom_line_check(v1, win1, v2, win2, mat_earth_equ_to_eye);
	}

	inline void unproject_earth_equ(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_earth_equ_to_eye, v);
	}

	inline bool unproject_j2000(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_j2000_to_eye, v);
		return true;
	}

	// taking account of precession
	inline bool project_j2000(const Vec3d& v, Vec3d& win) const {
		return project_custom(v, win, mat_j2000_to_eye);
	}

	inline bool project_j2000_check(const Vec3d& v, Vec3d& win) const {
		return project_custom_check(v, win, mat_j2000_to_eye);
	}

	inline bool project_j2000_line_check(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const {
		return project_custom_line_check(v1, win1, v2, win2, mat_j2000_to_eye);
	}


	inline bool project_galactic(const Vec3d& v, Vec3d& win) const {
		return project_custom(v, win, mat_galactic_to_eye);
	}

	inline bool project_galactic_check(const Vec3d& v, Vec3d& win) const {
		return project_custom_check(v, win, mat_galactic_to_eye);
	}

	inline bool project_galactic_line_check(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const {
		return project_custom_line_check(v1, win1, v2, win2, mat_galactic_to_eye);
	}


	// Same function with input vector v in heliocentric coordinate
	inline bool project_helio_check(const Vec3d& v, Vec3d& win) const {
		return project_custom_check(v, win, mat_helio_to_eye);
	}

	inline bool project_helio(const Vec3d& v, Vec3d& win) const {
		return project_custom(v, win, mat_helio_to_eye);
	}

	inline bool project_helio_line_check(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const {
		return project_custom_line_check(v1, win1, v2, win2, mat_helio_to_eye);
	}

	inline void unproject_helio(double x, double y, Vec3d& v) const {
		return unproject(x, y, inv_mat_helio_to_eye, v);
	}

	// Same function with input vector v in local coordinate
	inline bool project_local(const Vec3d& v, Vec3d& win) const {
		return project_custom(v, win, mat_local_to_eye);
	}

	inline bool project_local_check(const Vec3d& v, Vec3d& win) const {
		return project_custom_check(v, win, mat_local_to_eye);
	}

	inline void unproject_local(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_local_to_eye, v);
	}

	// Same function with input vector v in dome coordinates
	inline bool project_dome(const Vec3d& v, Vec3d& win) const {
		return project_custom_fixed_fov(v, win, mat_dome);
	}

	// Same function without heading
	inline bool project_dome_fixed(const Vec3d& v, Vec3d& win) const {
		return project_custom_fixed_fov(v, win, mat_dome_fixed);
	}

	virtual bool project_custom_fixed_fov(const Vec3d& v, Vec3d& win, const Mat4d& mat) const {
		gluProject(v[0],v[1],v[2],mat,mat_projection,vec_viewport,&win[0],&win[1],&win[2]);
		return (win[2]<1.);
	}

	// Same function but using a custom modelview matrix
	virtual bool project_custom(const Vec3d& v, Vec3d& win, const Mat4d& mat) const {
		gluProject(v[0],v[1],v[2],mat,mat_projection,vec_viewport,&win[0],&win[1],&win[2]);
		return (win[2]<1.);
	}

	bool project_custom_check(const Vec3f& v, Vec3d& win, const Mat4d& mat) const {
		return (project_custom(v, win, mat) && check_in_viewport(win));
	}

// for large objects
	bool project_custom_check(const Vec3f& v, Vec3d& win, const Mat4d& mat, const int object_pixel_radius) const {
		return (project_custom(v, win, mat) && check_in_mask(win, object_pixel_radius));
	}


	// project two points and make sure both are in front of viewer and that at least one is on screen
	bool project_custom_line_check(const Vec3f& v1, Vec3d& win1,
	                               const Vec3f& v2, Vec3d& win2, const Mat4d& mat) const {
		return project_custom(v1, win1, mat) && project_custom(v2, win2, mat) &&
		       (check_in_viewport(win1) || check_in_viewport(win2));
	}

	// Set the drawing mode in 2D for drawing inside the viewport only.
	// Use reset_perspective_projection() to restore previous projection mode
	void set_orthographic_projection(void) const;

	// Restore the previous projection mode after a call to set_orthographic_projection()
	void reset_perspective_projection(void) const;

	// Reimplementation of gluSphere : glu is overrided for non standard projection
	virtual void sSphere(GLdouble radius, GLdouble scale, GLdouble one_minus_oblateness,
	                     GLint slices, GLint stacks,
	                     const Mat4d& mat, int orient_inside = 0, bool shader = false) const;

	virtual void sSphere(GLdouble radius, GLdouble one_minus_oblateness,
	                     GLint slices, GLint stacks,
	                     const Mat4d& mat, int orient_inside = 0, bool shader = false) const {
		sSphere(radius, 1.0, one_minus_oblateness, slices, stacks, mat, orient_inside, shader);
	}

	// Draw only a partial sphere with top and/or bottom missing
	virtual void sPartialSphere(GLdouble radius, GLdouble one_minus_oblateness,
	                            GLint slices, GLint stacks,
	                            const Mat4d& mat, int orient_inside = 0,
	                            double bottom_altitude = -90, double top_altitude = 90) const;

	// Draw a half sphere
	virtual void sHalfSphere(GLdouble radius, GLint slices, GLint stacks,
	                         const Mat4d& mat, int orient_inside = 0) const;

	// Draw a disk with a special texturing mode having texture center at center
	virtual void sDisk(GLdouble radius, GLint slices, GLint stacks,
	                   const Mat4d& mat, int orient_inside = 0) const;

	// Draw a ring with a radial texturing
	virtual void sRing(GLdouble r_min, GLdouble r_max,
	                   GLint slices, GLint stacks,
	                   const Mat4d& mat, int orient_inside, bool shader=false) const;

	// Draw a fisheye texture in a sphere
	virtual void sSphere_map(GLdouble radius, GLint slices, GLint stacks,
	                         const Mat4d& mat, double texture_fov = 2.*M_PI, int orient_inside = 0) const;

	// Reimplementation of gluCylinder : glu is overrided for non standard projection
	virtual void sCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks,
	                       const Mat4d& mat, int orient_inside = 0) const;

	virtual void sVertex3(double x, double y, double z, const Mat4d& mat) const {
		glVertex3d(x,y,z);
	}

	virtual void oVertex3(double x, double y, double z, const Mat4d& mat) const {
		glVertex3d(x,y,z);
	}

	void print_gravity180(s_font* font, float x, float y, const string& str,
	                      bool speed_optimize = 1, float xshift = 0, float yshift = 0) const;

	void setFlagGravityLabels(bool gravity) {
		gravityLabels = gravity;
	}
	bool getFlagGravityLabels() const {
		return gravityLabels;
	}

	//! Draw a square using the current texture at the given projected 2d position.
	//! @param x x position in the viewport in pixel.
	//! @param y y position in the viewport in pixel.
	//! @param size the size of a square side in pixel.
	void drawSprite2dMode(double x, double y, double size) const;

	//! Draw a rotated square using the current texture at the given projected 2d position.
	//! @param x x position in the viewport in pixel.
	//! @param y y position in the viewport in pixel.
	//! @param size the size of a square side in pixel.
	//! @param rotation rotation angle in degree.
	void drawSprite2dMode(double x, double y, double size, double rotation) const;

	//! Draw a rotated rectangle using the current texture at the given projected 2d position.
	//! @param x x position in the viewport in pixel.
	//! @param y y position in the viewport in pixel.
	//! @param sizex the size of the rectangle x side in pixel.
	//! @param sizey the size of the rectangle y side in pixel.
	//! @param rotation rotation angle in degree.
	void drawRectSprite2dMode(double x, double y, double sizex, double sizey, double rotation) const;

	//! Draw a GL_POINT at the given position.
	//! @param x x position in the viewport in pixels.
	//! @param y y position in the viewport in pixels.
	void drawPoint2d(double x, double y) const;

	//! Un-project the entire viewport depending on mapping, maskType,
	//! viewport_fov_diameter, viewport_center, and viewport dimensions.
	StelGeom::ConvexS unprojectViewport(void) const;

	void setCurrentProjection(std::string& );

	int ProjectorConfiguration;  // for projection configuration


protected:
	Projector(const Vec4i& viewport, double _fov = 60.);

	// Struct used to store data for auto mov
	typedef struct {
		double start;
		double aim;
		float speed;
		float coef;
	} auto_zoom;

	// Init the viewing matrix from the fov, the clipping planes and screen ratio
	// The function is a reimplementation of gluPerspective
	virtual void init_project_matrix(void);

	//! The current projector mask
	PROJECTOR_MASK_TYPE maskType;

	double fov;					// Field of view in degree
	double min_fov;				// Minimum fov in degree
	double max_fov;				// Maximum fov in degree
	double zNear, zFar;			// Near and far clipping planes
	Vec4i vec_viewport;			// Viewport parameters
	Mat4d mat_projection;		// Projection matrix

	Vec3d viewport_center;				// Viewport center in screen pixel
	int viewport_radius;  				// Viewport radius in screen pixels
	int displayW, displayH;   			// track actual resolution
	int last_cx, last_cy, last_radius; 	// for refreshing disk viewport center hoffset easily
	double view_scaling_factor;			// ??
	double flip_horz,flip_vert;
	double shear_horz;
	double pixel_per_rad; 			// pixel per rad at the center of the viewport disk

	Mat4d mat_earth_equ_to_eye;		// Modelview Matrix for earth equatorial projection
	Mat4d mat_j2000_to_eye;         // for precessed equ coords
	Mat4d mat_galactic_to_eye;      // for galactic coords
	Mat4d mat_helio_to_eye;			// Modelview Matrix for earth equatorial projection
	Mat4d mat_local_to_eye;			// Modelview Matrix for earth equatorial projection
	Mat4d inv_mat_earth_equ_to_eye;	// Inverse of mat_projection*mat_earth_equ_to_eye
	Mat4d inv_mat_j2000_to_eye;		
	Mat4d inv_mat_galactic_to_eye;	
	Mat4d inv_mat_helio_to_eye;		// Inverse of mat_projection*mat_helio_to_eye
	Mat4d inv_mat_local_to_eye;		// Inverse of mat_projection*mat_local_to_eye


	Mat4d mat_dome;
	Mat4d inv_mat_dome;
	// Same without heading adjustment
	Mat4d mat_dome_fixed;
	Mat4d inv_mat_dome_fixed;

	const Mapping *mapping;
	std::map<std::string,const Mapping*> projectionMapping;

	// transformation from screen 2D point x,y to object
	// m is here the already inverted full tranfo matrix
	virtual
	void unproject(double x, double y, const Mat4d& m, Vec3d& v) const {
		v.set(	(x - vec_viewport[0]) * 2. / vec_viewport[2] - 1.0,
		       (y - vec_viewport[1]) * 2. / vec_viewport[3] - 1.0,
		       1.0);
		v.transfo4d(m);
	}
	bool unProject(double x, double y, Vec3d& v) const;

	// Automove
	auto_zoom zoom_move;		// Current auto movement
	bool flag_auto_zoom;		// Define if autozoom is on or off
	bool gravityLabels;			// should label text align with the horizon?

	double offset_x, offset_y;
	int center_horizontal_offset;  // For disk modes

private:
	void registerProjectionMapping(Mapping *c);
	void initGlMatrixOrtho2d(void) const;
	std::string currentProjectionType;	// Type of the projection currently used
	bool flagGlPointSprite;
	double viewport_fov_diameter;
};

#endif // _PROJECTOR_H_
