/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009, 2011 Digitalis Education Solutions, Inc.
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

#ifndef _NEBULA_H_
#define _NEBULA_H_

#include "object_base.h"
#include "nightshade.h"
#include "projector.h"
#include "navigator.h"
#include "s_texture.h"
#include "s_font.h"
#include "tone_reproductor.h"
#include "translator.h"

/*
Gx Galaxy
OC Open star cluster
Gb Globular star cluster, usually in the Milky Way Galaxy
Nb Bright emission or reflection nebula
Pl Planetary nebula
C+N Cluster associated with nebulosity
Ast Asterism or group of a few stars
Kt Knot or nebulous region in an external galaxy
*** Triple star
D* Double star
* Single star
? Uncertain type or may not exist
blank Unidentified at the place given, or type unknown
- Object called nonexistent in the RNGC (Sulentic and Tifft 1973)
PD Photographic plate defect
*/


class Nebula : public ObjectBase
{
	friend class NebulaMgr;
public:
	enum nebula_type { NEB_GX, NEB_OC, NEB_GC, NEB_N, NEB_PN, NEB_DN, NEB_IG, NEB_CN, NEB_UNKNOWN };

	Nebula();
	~Nebula();

	string getInfoString(const Navigator * nav) const;
	string getShortInfoString(const Navigator * nav = NULL) const;
	ObjectRecord::OBJECT_TYPE get_type(void) const {
		return ObjectRecord::OBJECT_NEBULA;
	}
	Vec3d get_earth_equ_pos(const Navigator *nav) const {
		return nav->j2000_to_earth_equ(XYZ);
	}
	// observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const {
		return XYZ;
	}
	double get_close_fov(const Navigator * nav = NULL) const;
	float get_mag(const Navigator * nav = NULL) const {
		return mag;
	}

	void setLabelColor(const Vec3f& v) {
		label_color = v;
	}
	void setCircleColor(const Vec3f& v) {
		circle_color = v;
	}

	// Read the Nebula data from a file
	bool readTexture(const string&);

	// Create Nebula from passed data and then read in texture
	bool readTexture(double ra, double de, double magnitude, double angular_size, double rotation,
					 string name, string filename, string credit, double texture_luminance_adjust,
					 double distance );

	bool readNGC(char *record);

	string getNameI18n(void) const {
		return nameI18;
	}
	string getEnglishName(void) const {
		return englishName;
	}

	/**
	 * Return the texture associated with this nebula.
	 * @author: Ákos Pap
	 */
	s_texture getTexture() { return *neb_tex; }

	//! @brief Get the printable nebula Type
	//! @return the nebula type code.
	//string getTypeString(void) const {return L"TODO";}
	string getTypeString(void) const;

	void translateName(Translator&);

	bool isDeleteable() const {
		return m_deleteable;    // If allowed to delete from script
	}

protected:
	// Return the radius of a circle containing the object on screen
	float get_on_screen_size(const Projector* prj,
	                         const Navigator * nav = NULL,
	                         bool orb_only = false) {
		return m_angular_size * (180./M_PI)
		       * (prj->getViewportHeight()/prj->get_fov());
	}

	void hide() {
		m_hidden = true;
	}

	void show() {
		m_hidden = false;
	}


private:
	void draw_chart(const Projector* prj, const Navigator * nav);
	void draw_tex(const Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness);
	void draw_no_tex(const Projector* prj, const Navigator * nav, ToneReproductor* eye);
	void draw_name(const Projector* prj);
	void draw_circle(const Projector* prj, const Navigator * nav);
	bool hasTex(void) {
		return (neb_tex != NULL);
	}

	unsigned int M_nb;			// Messier Catalog number
	unsigned int NGC_nb;			// New General Catalog number
	unsigned int IC_nb;			// Index Catalog number
	/*unsigned int UGC_nb;			// Uppsala General  Catalog number*/
	string englishName;				// English name
	string nameI18;				// Nebula name
	string m_credit;					// Nebula image credit
	float mag;						// Apparent magnitude for object

	float tex_luminance_adjust;    // draw texture luminance adjustment for overexposed images
	float m_angular_size;				// Angular size in radians
	Vec3f XYZ;						// Cartesian equatorial position
	Vec3d XY;						// Store temporary 2D position
	nebula_type nType;

	s_texture * neb_tex;			// Texture
	Vec3f tex_quad_vertex[4];		// The 4 vertex used to draw the nebula texture
	float luminance;			// Object luminance to use (value computed to compensate
	// the texture avergae luminosity)
	float tex_avg_luminance;        // avg luminance of the texture (saved here for performance)
	float inc_lum;					// Local counter for symbol animation

	float RA, DE;  // in radians
	float ASIZE;  // angular texture size in radians
	float ROTATION;  // rotation in radians
	double m_distance; // in light years

	bool m_deleteable; // whether a script added this nebula
	bool m_hidden;  // whether hidden from being visible or searchable (TODO)

	static s_texture * tex_circle;	// The symbolic circle texture
	static s_font* nebula_font;		// Font used for names printing
	static float hints_brightness;
	static float nebula_brightness;

	static Vec3f label_color, circle_color;
	static float circleScale;		// Define the sclaing of the hints circle
	static bool flagBright;			// Define if nebulae must be drawn in bright mode

	static const float RADIUS_NEB;
};

#endif // _NEBULA_H_
