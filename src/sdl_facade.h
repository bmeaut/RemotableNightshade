/*
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

/*
 * A wrapper around basic SDL functionality such as initialization, surface creation,
 * and available video mode queries.
 */

#pragma once

#ifdef HAVE_SDL_MIXER_H
#include "SDL_mixer.h"
#endif

#ifdef WIN32
#include "shlobj.h"
#endif

#include "s_gui.h"

class SDLFacade {

public:
	static SDL_Cursor *create_cursor(const char *image[]);
	virtual ~SDLFacade();

	// Must be called prior to any other SDL methods
	void initSDL( void );

	// Creates the rendering target. Must be called prior to any OpenGL functions
	void createSurface(Uint16 w, Uint16 h, int bbpMode, bool fullScreen, string iconFile);

	// Video mode queries
	void getResolution( Uint16* const w, Uint16* const h ) const;
	void getCurrentRes( Uint16* const w, Uint16* const h ) const;
	void getMaxFullscreenRes( Uint16* const w, Uint16* const h ) const;
	string getVideoModeList(void) const;

	// Returns the SDL surface created by createSurface
	SDL_Surface* const getScreen(void);

private:
	SDL_Surface* Screen;  // The Screen
	SDL_Cursor *Cursor;
	Uint16 screenW;
	Uint16 screenH;
};
