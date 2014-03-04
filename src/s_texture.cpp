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
#include <stdlib.h>
#include <Magick++.h>
#include <exception>

#include "s_texture.h"
#include "nightshade.h"

string s_texture::texDir = "./";

s_texture::s_texture(const string& _textureName) : textureName(_textureName), texID(0),
		loadType(PNG_BLEND1), loadType2(GL_CLAMP)
{
	load( texDir + textureName );
}

// when need to load images outside texture directory
s_texture::s_texture(bool full_path, const string& _textureName, int _loadType) : textureName(_textureName),
		texID(0), loadType(PNG_BLEND1), loadType2(GL_CLAMP_TO_EDGE)
{
	switch (_loadType) {
	case TEX_LOAD_TYPE_PNG_ALPHA :
		loadType=PNG_ALPHA;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID :
		loadType=PNG_SOLID;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND3:
		loadType=PNG_BLEND3;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND1:
		loadType=PNG_BLEND1;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID_REPEAT:
		loadType=PNG_SOLID;
		loadType2=GL_REPEAT;
		break;
	default :
		loadType=PNG_BLEND3;
	}
	texID=0;
	whole_path = full_path;
	if (full_path) load(textureName );
	else load( texDir + textureName );
}

s_texture::s_texture(bool full_path, const string& _textureName, int _loadType, const bool mipmap) : textureName(_textureName),
		texID(0), loadType(PNG_BLEND1), loadType2(GL_CLAMP_TO_EDGE)
{
	switch (_loadType) {
	case TEX_LOAD_TYPE_PNG_ALPHA :
		loadType=PNG_ALPHA;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID :
		loadType=PNG_SOLID;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND3:
		loadType=PNG_BLEND3;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND1:
		loadType=PNG_BLEND1;
	case TEX_LOAD_TYPE_PNG_SOLID_REPEAT:
		loadType=PNG_SOLID;
		loadType2=GL_REPEAT;
		break;
	default :
		loadType=PNG_BLEND3;
	}
	texID=0;
	whole_path = full_path;
	if (full_path) load(textureName, mipmap );
	else load( texDir + textureName, mipmap );
}


s_texture::s_texture(const s_texture &t)
{
	textureName = t.textureName;
	loadType = t.loadType;
	loadType2 = t.loadType2;
	whole_path = t.whole_path;
	texID=0;
	load(texDir + textureName);
}

const s_texture &s_texture::operator=(const s_texture &t)
{
	unload();
	textureName = t.textureName;
	loadType = t.loadType;
	loadType2 = t.loadType2;
	whole_path = t.whole_path;
	texID=0;
	load(texDir + textureName);
	return *this;
}

s_texture::s_texture(const string& _textureName, int _loadType, const bool mipmap) : textureName(_textureName),
		texID(0), loadType(PNG_BLEND1), loadType2(GL_CLAMP_TO_EDGE)
{
	switch (_loadType) {
	case TEX_LOAD_TYPE_PNG_ALPHA :
		loadType=PNG_ALPHA;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID :
		loadType=PNG_SOLID;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND3:
		loadType=PNG_BLEND3;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND1:
		loadType=PNG_BLEND1;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID_REPEAT:
		loadType=PNG_SOLID;
		loadType2=GL_REPEAT;
		break;
	default :
		loadType=PNG_BLEND3;
	}
	texID=0;
	load( texDir + textureName, mipmap);
}

s_texture::s_texture(const string& _textureName, int _loadType) : textureName(_textureName),
		texID(0), loadType(PNG_BLEND1), loadType2(GL_CLAMP_TO_EDGE)
{
	switch (_loadType) {
	case TEX_LOAD_TYPE_PNG_ALPHA :
		loadType=PNG_ALPHA;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID :
		loadType=PNG_SOLID;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND3:
		loadType=PNG_BLEND3;
		break;
	case TEX_LOAD_TYPE_PNG_BLEND1:
		loadType=PNG_BLEND1;
		break;
	case TEX_LOAD_TYPE_PNG_SOLID_REPEAT:
		loadType=PNG_SOLID;
		loadType2=GL_REPEAT;
		break;
	default :
		loadType=PNG_BLEND3;
	}
	texID=0;
	load( texDir + textureName);
}


s_texture::~s_texture()
{
	unload();
}

bool s_texture::bind( void ) {
	glBindTexture(GL_TEXTURE_2D, texID);
	return true;
}

void s_texture::blend( const int type, char* const data, const unsigned int sz ) {
	char* a = NULL;
	char* ptr = data;
	int r, g, b;

	switch( type ){

	case PNG_BLEND1:
		for( unsigned int i = 0; i < sz; i+=4 ) {
			r = *ptr++;
			g = *ptr++;
			b = *ptr++;
			a = ptr++;
			if (r+g+b > 255) *a = 255;
			else *a = r+g+b;
		}
		break;

	case PNG_BLEND3:
		for( unsigned int i = 0; i < sz; i+=4 ) {
			r = *ptr++;
			g = *ptr++;
			b = *ptr++;
			a = ptr++;
			*a = (r+g+b)/3;
		}
		break;

	default:
		break;
	}
}

// Returns the closest power of 2 to i
unsigned int s_texture::roundToPow2( unsigned int i ) {
	unsigned int val = 0;

	for (unsigned int p = 0; p < 32; p++) {
		if (i <= (unsigned int)(1<<p)) {
			val = 1<<p;
			break;
		}
	}

	return val;
}

bool s_texture::ProxyLoad( unsigned int w, unsigned int h, GLint format, GLint type ){
	GLint width = 0;

	glTexImage2D( GL_PROXY_TEXTURE_2D, 0, format, w, h, 0, format, type, NULL );
	glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);

	return (width == 0 ? false : true);
}

double s_texture::checkForGammaEnv()
{
	double viewingGamma = 1.8;
	char *gammaEnv = getenv("VIEWING_GAMMA");

	if(gammaEnv)
		sscanf(gammaEnv, "%lf", &viewingGamma);

	return 1.0/viewingGamma;
}

int s_texture::load(string fullName)
{

	// assume NO mipmap - DIGITALIS - put in svn
	return load(fullName, false);
}

int s_texture::load(string fullName, bool mipmap)
{
	try {
		// Load image and ensure geometry a power of 2 for older GL versions
		Magick::Image image(fullName);
		image.flip();
		unsigned int w = roundToPow2(image.columns()), h = roundToPow2(image.rows());
		if (w != image.columns() || h != image.rows()) {
			Magick::Geometry sz( w, h );
			image.scale( sz );
		}

		// Convert image to RGBA format in memory
		Magick::Blob blob;
		image.magick("RGBA");
		image.write(&blob);

		// Massage alpha channel data
		unsigned int len = blob.length();
		char* raw = new char[len];
		memcpy( raw, blob.data(), len );
		blend( loadType, raw, len );
		blob.updateNoCopy( raw, len );
		// !!! Do not delete 'raw' ptr, blob takes ownership on updateNoCopy !!!

		// Simulate load of texture prior to actual load. This is safer than querying GL_MAX_TEXTURE_SIZE
		// as it takes into account image format. Keep trying until fake load is successful.
		unsigned int width = image.columns();
		unsigned int height = image.rows();
		while( width >> 1 && height >> 1 ) {
			if ( ProxyLoad(width, height, GL_RGBA, GL_UNSIGNED_BYTE) )
				break;
			else {
				width >>= 1;
				height >>= 1;
			}
		}
		if( width != image.columns() || height != image.rows() ) {
			Magick::Geometry sz( width, height );
			image.scale( sz );
		}

		glGenTextures(1, &texID);
		if( texID ) {
			glBindTexture(GL_TEXTURE_2D, texID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, loadType2);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, loadType2);

			if( mipmap ) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				if( gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.columns(), image.rows(), GL_RGBA,
						GL_UNSIGNED_BYTE, blob.data()) )
					throw std::exception();
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.columns(), image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());
			}
		}
	}
	catch( std::exception &e ) {
		cerr << "WARNING : failed loading texture file! " << e.what() << endl;
	}

	return (texID!=0);
}

void s_texture::unload()
{
	glDeleteTextures(1, &texID);	// Delete The Texture
	texID = 0;
}

int s_texture::reload()
{
	unload();
	if (whole_path) return load(textureName);
	else return load( texDir + textureName );
}

// Deprecated
// Return the texture WIDTH in pixels
int s_texture::getSize(void) const
{
	glBindTexture(GL_TEXTURE_2D, texID);
	GLint w;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	return w;
}

void s_texture::getDimensions(int &width, int &height) const
{
	glBindTexture(GL_TEXTURE_2D, texID);

	GLint w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	width = w;
	height = h;

}

// Return the average texture luminance : 0 is black, 1 is white
float s_texture::get_average_luminance(void) const
{
	glBindTexture(GL_TEXTURE_2D, texID);
	GLint w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	GLfloat* p = (GLfloat*)calloc(w*h, sizeof(GLfloat));
	assert(p);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_FLOAT, p);
	float sum = 0.f;
	for (int i=0; i<w*h; ++i) {
		sum += p[i];
	}
	free(p);


	/*
	// This provides more correct result on some video cards (matrox)
	// TODO test more before switching

	GLubyte* pix = (GLubyte*)calloc(w*h*3, sizeof(GLubyte));

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);

	float lum = 0.f;
	for (int i=0;i<w*h*3;i+=3)
	{
	  double r = pix[i]/255.;
	  double g = pix[i+1]/255.;
	  double b = pix[i+2]/255.;
	  lum += r*.299 + g*.587 + b*.114;
	}
	free(pix);

	printf("Luminance calc 2: Sum %f\tw %d h %d\tlum %f\n", lum, w, h, lum/(w*h));
	*/

	return sum/(w*h);

}
