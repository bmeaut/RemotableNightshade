/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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

#include "app.h"
#include "sdl_facade.h"

using namespace s_gui;


SDLFacade::~SDLFacade(){
	SDL_FreeCursor(Cursor);
}


SDL_Surface* const SDLFacade::getScreen() {
	return Screen;
}

void SDLFacade::getResolution( Uint16* const w, Uint16* const h ) const {
	*w = screenW;
	*h = screenH;
}

void SDLFacade::getCurrentRes( Uint16* const w, Uint16* const h ) const {
	*w = 0;
	*h = 0;
	const SDL_VideoInfo* inf = SDL_GetVideoInfo();

	if( inf ) {
		*w = inf->current_w;
		*h = inf->current_h;
	}
}

void SDLFacade::getMaxFullscreenRes( Uint16* const w, Uint16* const h ) const {
	SDL_Rect **modes;
	*w = 0;
	*h = 0;
	/* Get available fullscreen/hardware modes */
	modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	if (modes == (SDL_Rect **)0 || modes == (SDL_Rect **)-1)
		return;
	else {
		for (unsigned int i=0; modes[i]; ++i)
			if(  modes[i]->w * modes[i]->h > *w * *h ) {
				*w = modes[i]->w;
				*h = modes[i]->h;
			}
	}
}

void SDLFacade::createSurface( Uint16 w, Uint16 h, int bppMode, bool fullScreen, string iconFile ) {
	
	Uint32	Vflags;		// Our Video Flags
	Screen = NULL;
	screenW = w;
	screenH = h;
	
	// We want a hardware surface
	Vflags = SDL_HWSURFACE|SDL_OPENGL;//|SDL_DOUBLEBUF;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,1);
	// If fullscreen, set the Flag
	if (fullScreen) Vflags|=SDL_FULLSCREEN;
	
	// Sync refresh rate if desired and available
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	
	// Turn on Full Screen Anti-Aliasing (FSAA)
//	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 ) ;
//	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 2 ) ;

	// Create the SDL screen surface
	Screen = SDL_SetVideoMode(screenW, screenH, bppMode, Vflags);
	if (!Screen) {
		printf("Warning: Couldn't set %dx%d video mode (%s), retrying with stencil size 0\n", w, h, SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
		Screen = SDL_SetVideoMode(screenW, screenH, bppMode, Vflags);
		
		if (!Screen) {
			fprintf(stderr, "Error: Couldn't set %dx%d video mode: %s!\n", w, h, SDL_GetError());
			exit(-1);
		}
	}
	
	// set mouse cursor
	static const char *arrow[] = {
		/* width height num_colors chars_per_pixel */
		"    32    32        3            1",
		/* colors */
		"X c #000000",
		". c #ffffff",
		"  c None",
		/* pixels */
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"                                ",
		"   XXXXXXXX         XXXXXXXX    ",
		"   X......X         X......X    ",
		"   XXXXXXXX         XXXXXXXX    ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"15,17"
	};
	
	Cursor = create_cursor(arrow);
	SDL_SetCursor(Cursor);
	
	// Set the window caption
	SDL_WM_SetCaption(APP_NAME, APP_NAME);
	
	// Set the window icon
	SDL_Surface *icon = SDL_LoadBMP((iconFile).c_str());
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
	
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
}

void SDLFacade::initSDL() {

#ifdef HAVE_SDL_MIXER_H

	// Init the SDL library, the VIDEO subsystem
	// Tony - added timer
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER)<0) {
		// couldn't init audio, so try without
		fprintf(stderr, "Error: unable to open SDL with audio: %s\n", SDL_GetError() );

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER)<0) {
			fprintf(stderr, "Error: unable to open SDL: %s\n", SDL_GetError() );
			exit(-1);
		}
	} else {
		/*
		// initialized with audio enabled
		// TODO: only initi audio if config option allows and script needs
		if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
		{
			printf("Unable to open audio!\n");
		}

		*/
	}
#else
	// SDL_mixer is not available - no audio
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER)<0) {
		fprintf(stderr, "Unable to open SDL: %s\n", SDL_GetError() );
		exit(-1);
	}
#endif

	// Make sure that SDL_Quit will be called in case of exit()
	atexit(SDL_Quit);

	// Might not work TODO check how to handle that
	//SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);

	// Disable key repeat
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(1);
}

string SDLFacade::getVideoModeList(void) const
{
	SDL_Rect **modes;
	int i;
	/* Get available fullscreen/hardware modes */
	modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	/* Check is there are any modes available */
	if (modes == (SDL_Rect **)0) {
		return "No modes available!\n";
	}
	/* Check if our resolution is restricted */
	if (modes == (SDL_Rect **)-1) {
		return "All resolutions available.\n";
	} else {
		/* Print valid modes */
		ostringstream modesstr;
		for (i=0; modes[i]; ++i)
			modesstr << modes[i]->w << "x" << modes[i]->h << endl;
		return modesstr.str();
	}
}

// from an sdl wiki
SDL_Cursor* SDLFacade::create_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;

	i = -1;
	for ( row=0; row<32; ++row ) {
		for ( col=0; col<32; ++col ) {
			if ( col % 8 ) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4+row][col]) {
			case 'X':
				data[i] |= 0x01;
				mask[i] |= 0x01;
				break;
			case '.':
				mask[i] |= 0x01;
				break;
			case ' ':
				break;
			}
		}
	}
	sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

