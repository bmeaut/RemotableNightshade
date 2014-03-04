/*
 * Nightshade
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

// Class to manage fonts

#include <vector>
#include "s_font.h"
#include "utility.h"
#include "boost/regex.hpp"

#include "SDL_Pango.h"

s_font::s_font(float size_i, const string& ttfFileName) : lineHeightEstimate(0) {

    context = SDLPango_CreateContext();

    SDLPango_SetDefaultColor((SDLPango_Context *)context, MATRIX_TRANSPARENT_BACK_WHITE_LETTER);
    
    SDLPango_SetMinimumSize((SDLPango_Context *)context, -1, 0);
    //    SDLPango_SetBaseDirection((SDLPango_Context *)context, SDLPANGO_DIRECTION_RTL);

	fontFamily = ttfFileName; // TODO fix name/strings
	fontSize = size_i; // pixel height

	//cout << "Created new font with size: " << fontSize << " and family: " << fontFamily << endl;

}

s_font::~s_font() {

	clearCache();

	if(context) {   
		SDLPango_FreeContext((SDLPango_Context *)context);
	}
}

//! print out a string
//! cache == 0 means do not cache rendered string texture
//! cache == -1 means do not actually draw, just cache
void s_font::print(float x, float y, const string& s, int upsidedown, int cache) {

	if(s == "") return;

	renderedString_struct currentRender;

	// If not cached, create texture
	if( !cache || renderCache[s].textureW == 0 ) {

		currentRender = renderString(s);

		if( cache ) {
			renderCache[s] = currentRender;
			/*
.stringTexture = fontTexture;
			renderCache[s].textureW = w;
			renderCache[s].textureH = h;
			renderCache[s].stringW = textw;
			renderCache[s].stringH = texth;
			*/
			//cout << "Cached string: " << s << endl;
		}
	} else {

		// read from cache
		currentRender = renderCache[s];
			/*
.stringTexture;
		w = renderCache[s].textureW;
		h = renderCache[s].textureH;
		textw = renderCache[s].stringW;
		texth = renderCache[s].stringH;	
			*/

	}

	if(cache==-1) return; // do not draw, just wanted to cache

	// Draw
	glBindTexture( GL_TEXTURE_2D, currentRender.stringTexture);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
	// Avoid edge visibility
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float h = currentRender.textureH;
	float w = currentRender.textureW;

	if(!upsidedown) {	  
		y -= currentRender.stringH;  // adjust for base of text in texture
		glBegin(GL_QUADS );
		glTexCoord2i(0,0);    glVertex2f(x, y);	// Bottom left
		glTexCoord2i(1,0);    glVertex2f(x+w, y);	// Bottom right
		glTexCoord2i(1,1);    glVertex2f(x+w, y+h);	// Top right
		glTexCoord2i(0,1);    glVertex2f(x, y+h);	// Top left
		glEnd();
	} else {
		y -= currentRender.stringH;  // adjust for base of text in texture
		glBegin(GL_QUADS );
		glTexCoord2i(0,0);    glVertex2f(x, y+h);	// Bottom left
		glTexCoord2i(1,0);    glVertex2f(x+w, y+h);	// Bottom right
		glTexCoord2i(1,1);    glVertex2f(x+w, y);	// Top right
		glTexCoord2i(0,1);    glVertex2f(x, y);	// Top left
		glEnd();
	}

	if(!cache) glDeleteTextures( 1, &currentRender.stringTexture);

}
	
float s_font::getStrLen(const string& s, bool cache) {

	if(s == "") return 0;
	
	if( renderCache[s].textureW != 0 ) return renderCache[s].stringW;

	// otherwise calculate (and cache if desired)
	if(cache) {
		print(0, 0, s, 0, -1);
		return renderCache[s].stringW;
	} else {
		string escapedString = escapeSpecialChars(s);
		SDLPango_SetMarkup((SDLPango_Context *)context, (getFontMarkup() + escapedString + "</span>").c_str(), -1);
		return SDLPango_GetLayoutWidth((SDLPango_Context *)context);
	}
}

float s_font::getLineHeight(void) {

	// These are imperfect since not based on actual string
	if( lineHeightEstimate != 0 ) return lineHeightEstimate;

	SDLPango_SetMarkup((SDLPango_Context *)context, (getFontMarkup() + "Tp</span>").c_str(), -1);  // Kludge
	return (lineHeightEstimate = SDLPango_GetLayoutHeight((SDLPango_Context *)context));
}

string s_font::getFontMarkup() const {

	// TODO add font family
	std::ostringstream oss;
	oss << "<span font_desc=\"" << fontSize << "px\">";
	return oss.str();

}

//! remove cached texture for string
void s_font::clearCache(const string& s) {
	if( renderCache[s].textureW != 0 ) {	
		glDeleteTextures( 1, &renderCache[s].stringTexture);
		renderCache.erase(s);
	}

}

//! remove ALL cached textures
void s_font::clearCache() {
	for ( renderedStringHashIter_t iter = renderCache.begin(); iter != renderCache.end(); ++iter ) {
		if( (*iter).second.textureW != 0 ) {	
			glDeleteTextures( 1, &((*iter).second.stringTexture));
			//			cout << "Cleared cache for string: " << (*iter).first << endl; 
		}
	}

	renderCache.clear();
}

//! Render a string to a texture
renderedString_struct s_font::renderString(const string &s) const {

	string escapedString = escapeSpecialChars(s);

	renderedString_struct rendering;

	SDLPango_SetMarkup((SDLPango_Context *)context, (getFontMarkup() + escapedString + "</span>").c_str(), -1);
  
	// Calculate opengl texture size required
	rendering.stringW = SDLPango_GetLayoutWidth((SDLPango_Context *)context);
	rendering.stringH = SDLPango_GetLayoutHeight((SDLPango_Context *)context);

	// opengl texture dimensions must be powers of 2
	rendering.textureW = getNextPowerOf2((int)rendering.stringW); 
	rendering.textureH = getNextPowerOf2((int)rendering.stringH); 

	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	
	SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)rendering.textureW, (int)rendering.textureH,
												32, rmask, gmask, bmask, amask);
	renderedString_struct nothing;
	nothing.textureW = nothing.textureH = nothing.stringW = nothing.stringH = 0;
	nothing.stringTexture = 0;
	if(!surface) return nothing;
	
	SDLPango_Draw((SDLPango_Context *)context, surface, 0, 0);
    
	glGenTextures( 1, &rendering.stringTexture);
	glBindTexture( GL_TEXTURE_2D, rendering.stringTexture);
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
	// Avoid edge visibility
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// get the number of channels in the SDL surface
	GLenum texture_format;
	GLint nOfColors = surface->format->BytesPerPixel;
	if (nOfColors == 4) {     // contains an alpha channel
		if (surface->format->Rmask == 0x000000ff)
			texture_format = GL_RGBA;
		else
			texture_format = GL_BGRA;
	} else if (nOfColors == 3) {     // no alpha channel
		if (surface->format->Rmask == 0x000000ff)  // THIS IS WRONG for someplatforms
			texture_format = GL_RGB;
		else
			texture_format = GL_BGR;
	} else {
		cerr << "Error: unable to convert surface to font texture.\n";
		if(surface) SDL_FreeSurface(surface);  
		return nothing;
	}
	
	glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, (GLint)rendering.textureW, (GLint)rendering.textureH, 0,
				  texture_format, GL_UNSIGNED_BYTE, surface->pixels );

	if(surface) SDL_FreeSurface(surface);  

	return rendering;
}

/* May be useful later, but not happy with alignment

//! Draw text with baseline more or less parallel with horizon
//! justify: -1 left, 0 center, 1 right align
void s_font::print_gravity(Projector * prj, float altitude, float azimuth, const string& str,
						   int justify, bool cache, bool outline, float xshift, float yshift) {

	renderedString_struct rendering;	

	// Get rendered texture
	if(renderCache[str].textureW == 0) {
		rendering = renderString(str);
		if(cache) renderCache[str] = rendering;
	} else {
		rendering = renderCache[str];
	}

	//	cout << "drawing: " << str << " at " << x << ", " << y << " with tex id: " << rendering.stringTexture << endl;

//	cout << "sw: " << rendering.stringW << " sh: " << rendering.stringH <<
//		" -- tw: " << rendering.textureW << " th: " << rendering.textureH << endl;

	int vieww = prj->getViewportWidth();
	int viewh = prj->getViewportHeight();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, rendering.stringTexture);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3d center = prj->getViewportCenter();
	float radius = center[2];

	// If radius is set, then use that to determine
	// viewport size so that truncated fisheye works
	// with viewport images as one would expect
	if (radius > 0) {
		vieww = viewh = int(radius * 2);
	}

	// calculations to keep image proportions when scale up to fit view
	float prj_ratio = (float)vieww/viewh;
	float image_ratio = rendering.stringW/rendering.stringH;
	float xbase, ybase;
	if (image_ratio > prj_ratio) {
		xbase = vieww/2;
		ybase = xbase/image_ratio;
	} else {
		ybase = viewh/2;
		xbase = ybase*image_ratio;
	}

	// largest angular size
	float image_scale = rendering.stringW/vieww*180.0;  // ASSUME 180 fov and horizontal aspect

	// Justification - don't like this but it works for now
	if(justify < 0) azimuth += image_scale/2;
	if(justify > 0) azimuth -= image_scale/2;

	//	cout << image_scale << " " << str << endl;

	Mat4d mat;
	prj->set_orthographic_projection();    // 2D coordinate

	Vec3d gridpt;
	Vec3d onscreen, offscreen;
	onscreen[2] = 1;  // flag was projected
	offscreen[2] = 0;  // flag was not projected

	// printf("%f %f\n", altitude, azimuth);

	// altitude = xpos, azimuth = ypos (0 at North), image top towards zenith when rotation = 0

	float image_rotation = 0;

	Vec3d imagev = Mat4d::zrotation(-1*(azimuth-90)*M_PI/180.)
		* Mat4d::xrotation(altitude*M_PI/180.) * Vec3d(0,1,0);

	Vec3d ortho1 = Mat4d::zrotation(-1*(azimuth-90)*M_PI/180.) * Vec3d(1,0,0);
	Vec3d ortho2 = imagev^ortho1;
	
	float textureExtentH = rendering.stringH/rendering.textureH;
	float textureExtentW = rendering.stringW/rendering.textureW;

	vector<Vec3d> meshPoints;  // screen x,y,1 if onscreen

	// TODO: for performance could reduce vertical grid granularity
	// divisions per row, column
	int grid_size = int(image_scale/5);
	if(grid_size < 10 ) grid_size = 10;

	float offset = (float)grid_size/2.;
	int k = 0;

	// Pre-calculate points (more efficient)
	for (int i=0; i<=grid_size; i++) {
		
		for (int j=0; j<=grid_size; j++) {

			if (image_ratio<1) {
				// image height is maximum angular dimension
				gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					Mat4d::rotation( ortho1, image_scale*(j-offset)/(float)grid_size*M_PI/180.) *
					Mat4d::rotation( ortho2, image_scale*image_ratio*(i+k-offset)/(float)grid_size*M_PI/180.) *
					imagev;
				
			} else {
				// image width is maximum angular dimension
				gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					Mat4d::rotation( ortho1, image_scale/image_ratio*(j-offset)/(float)grid_size*M_PI/180.) *
					Mat4d::rotation( ortho2, image_scale*(i+k-offset)/(float)grid_size*M_PI/180.) *
					imagev;
			}
			
			if (prj->project_dome(gridpt, onscreen)) {
				meshPoints.push_back(onscreen);
			} else {
				meshPoints.push_back(offscreen);
			}	   
		}
	}
	
	// Draw
	int index;
	int shiftx = 0;
	int shifty = 0;

	GLfloat current_color[4];
	glGetFloatv(GL_CURRENT_COLOR, current_color);	 

	for (int pass=0; pass<outline*4+1; pass++) {

		if(outline) {
			if(pass < 4 ) {
				glColor3f(0,0,0);
				if(pass<2) shiftx = -1;
				else shiftx = 1;
				if(pass%2) shifty = -1;
				else shifty = 1;
			} else {
				glColor4fv(current_color);
				shiftx = shifty = 0;
			}
		}

		for (int i=0; i<grid_size; i++) {
		
			glBegin(GL_QUAD_STRIP);
			
			for (int j=0; j<=grid_size; j++) {
				
				for (int k=0; k<=1; k++) {
					
					index = j+(i+k)*(grid_size+1);
					//cout << "Draw: " << i << ", " << j << ", " << k << " -- " << index <<endl;
					
					glTexCoord2f((i+k)/(float)grid_size*textureExtentW,(grid_size-j)/(float)grid_size*textureExtentH);
					
					if (meshPoints[index][2]) {
						glVertex3d(meshPoints[index][0]+shiftx, meshPoints[index][1]+shifty, 0);
					} 
				}
			}

			glEnd();
		}
	}

	prj->reset_perspective_projection();

	if(!cache) glDeleteTextures( 1, &rendering.stringTexture);

}
*/

//! Draw text with baseline more or less parallel with horizon
//! justify: -1 left, 0 center, 1 right align (not impemented yet)
void s_font::print_gravity(Projector * prj, float altitude, float azimuth, const string& str,
						   int justify, bool cache, bool outline, float xshift, float yshift) {

	if(str == "") return;

	renderedString_struct rendering;	

	Vec3d startV, screen;
	sphe_to_rect(-azimuth*M_PI/180., altitude*M_PI/180., startV);
	prj->project_dome_fixed(startV, screen);
	float x = screen[0];
	float y = screen[1];

	//cout << str << " : " << cache << endl;

	// Get rendered texture
	if(renderCache[str].textureW == 0) {
		rendering = renderString(str);
		if(cache) renderCache[str] = rendering;
	} else {
		rendering = renderCache[str];
	}

	float textureExtentH = rendering.stringH/rendering.textureH;
	float textureExtentW = rendering.stringW/rendering.textureW;

	Vec3d center = prj->getViewportCenter();
	float radius = center[2];

	float dx = x - center[0];
	float dy = y - center[1];
	float d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if(radius > 0) {
		if (d > radius + rendering.stringH) return;
	} else {
		if(MY_MAX(prj->getViewportWidth(), prj->getViewportHeight() ) > d) return;
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, rendering.stringTexture);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float theta = M_PI + atan2f(dx, dy - 1);
	float psi = (float)getStrLen(str)/(d + 1);  // total angle of rotation

	int steps = int(psi/2);
	if(steps < 10) steps = 10;

	prj->set_orthographic_projection();


	float angle, p, q;

	/*
	glBegin(GL_QUAD_STRIP);

	for (int i=0; i<=steps; i++) {

		angle = theta - i*psi/steps;
		p = sin(angle);
		q = cos(angle);
 
		glTexCoord2f((float)i/steps*textureExtentW,0);
		glVertex2f(center[0]+p*(d-rendering.stringH), center[1]+q*(d-rendering.stringH));

		glTexCoord2f((float)i/steps*textureExtentW,textureExtentH);
		glVertex2f(center[0]+p*d,center[1]+q*d);

	}

	glEnd();
	*/

	vector<Vec2f> meshPoints;  // screen x,y

	// Pre-calculate points (more efficient)
	for (int i=0; i<=steps; i++) {

		angle = theta - i*psi/steps;
		p = sin(angle);
		q = cos(angle);

		meshPoints.push_back(Vec2f(center[0]+p*(d-rendering.stringH), center[1]+q*(d-rendering.stringH)));
		meshPoints.push_back(Vec2f(center[0]+p*d,center[1]+q*d));
	}
	
	int shiftx = 0;
	int shifty = 0;
	GLfloat current_color[4];
	glGetFloatv(GL_CURRENT_COLOR, current_color);	 

	for (int pass=0; pass<outline*4+1; pass++) {

		if(outline) {
			if(pass < 4 ) {
				glColor3f(0,0,0);
				if(pass<2) shiftx = -1;
				else shiftx = 1;
				if(pass%2) shifty = -1;
				else shifty = 1;
			} else {
				glColor4fv(current_color);
				shiftx = shifty = 0;
			}
		}

		glBegin(GL_QUAD_STRIP);

		for (int i=0; i<=steps; i++) {

			glTexCoord2f((float)i/steps*textureExtentW,0);
			glVertex2f(meshPoints[i*2][0]+shiftx, meshPoints[i*2][1]+shifty);

			glTexCoord2f((float)i/steps*textureExtentW,textureExtentH);
			glVertex2f(meshPoints[i*2+1][0]+shiftx, meshPoints[i*2+1][1]+shifty);
		}

		glEnd();
	}

	
	prj->reset_perspective_projection();

	if(!cache) glDeleteTextures( 1, &rendering.stringTexture);

	// TODO justification if needed

}


// Need to escape ampersands for Pango
string s_font::escapeSpecialChars(const string &input) const {

	static const boost::regex pattern ("&");
	static const string replace ("&amp;");
	
	return boost::regex_replace(input, pattern, replace);
}		

