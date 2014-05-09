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

#ifndef _S_TEXTURE_H_
#define _S_TEXTURE_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string>

#include "nightshade.h"

using namespace std;

#define PNG_ALPHA  0
#define PNG_SOLID  1
#define PNG_BLEND1 7
#define PNG_BLEND3 2

#define TEX_LOAD_TYPE_PNG_ALPHA 0
#define TEX_LOAD_TYPE_PNG_SOLID 1
#define TEX_LOAD_TYPE_PNG_BLEND3 2
#define TEX_LOAD_TYPE_PNG_SOLID_REPEAT 4
#define TEX_LOAD_TYPE_PNG_BLEND1 7

class s_texture
{
public:
	s_texture(){};
	s_texture(const string& _textureName);
	s_texture(bool full_path, const string& _textureName, int _loadType);
	s_texture(bool full_path, const string& _textureName, int _loadType, const bool mipmap);
	s_texture(const string& _textureName, int _loadType);
	s_texture(const string& _textureName, int _loadType, const bool mipmap);
	virtual ~s_texture();
	s_texture(const s_texture &t);
	const s_texture &operator=(const s_texture &t);
	int load(string fullName);
	int load(string fullName, bool mipmap);
	bool bind( void );
	void unload();
	int reload();
	unsigned int getID(void) const {
		return texID;
	}
	// Return the average texture luminance : 0 is black, 1 is white
	float get_average_luminance(void) const;
	int getSize(void) const;
	void getDimensions(int &width, int &height) const;
	static void set_texDir(const string& _texDir) {
		s_texture::texDir = _texDir;
	}

	/**
	 * Return the filename this texture was loaded from.
	 * @author: Ákos Pap
	 */
	string getFileName() { return (/*whole_path ? textureName : texDir + */textureName ); }

private:
	void blend( const int, char* const, const unsigned int );
	bool ProxyLoad( unsigned int, unsigned int, GLint, GLint );
	unsigned int roundToPow2( unsigned int );
	double checkForGammaEnv( void );

	string textureName;
	GLuint texID;
	int loadType;
	int loadType2;
	bool whole_path;

	static string texDir;
};


#endif // _S_TEXTURE_H_
