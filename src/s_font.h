/*
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

// Class to manage s_fonts

#ifndef _S_FONT_H
#define _S_FONT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <map>
#include "nightshade.h"

#include "utility.h"
#include "s_texture.h"
#include "projector.h"

class Projector;

typedef struct {
	GLuint stringTexture;  // Rendered string texture reference - remember to delete when done
	float textureW; 	   // Width of texture in pixels
	float textureH; 	   // Height of texture in pixels
	float stringW; 	       // Width of string portion in pixels
	float stringH; 	       // Height of string portion in pixels
} renderedString_struct;

typedef std::map< std::string, renderedString_struct > renderedStringHash_t;
typedef renderedStringHash_t::const_iterator renderedStringHashIter_t;

class s_font
{
public:
	s_font(float size_i, const string& ttfFileName);
    virtual ~s_font();
    
    void print(float x, float y, const string& s, int upsidedown = 1, int cache = 0);
	void print_gravity(Projector * prj, float altitude, float azimuth, const string& str,
					   int justify = 0, bool cache = 0, bool outline = 0, float xshift = 0, float yshift = 0);

    void clearCache(const string& s);
    void clearCache();

	renderedString_struct renderString(const string &s) const;
	
    float getStrLen(const string& s, bool cache = 0);
    float getLineHeight(void);  // TODO more accurate if per string
    float getAscent(void) { return getLineHeight(); }  // Kludge since data not available
    float getDescent(void) { return 0; }  // Kludge since data not available

	string escapeSpecialChars(const string &input) const;

protected:
	
	renderedStringHash_t renderCache;

	// return Pango markup string for setting font
	string getFontMarkup() const;
	void *context;  // Could not use SDLPango_Context * due to SDL_Pango.h issue which requires including only in one source file on some earlier versions
	string fontFamily;
	float fontSize;
	float lineHeightEstimate;
};




#endif  //_S_FONT_H
