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

#ifndef _NIGHTSHADE_H_
#define _NIGHTSHADE_H_

#include <config.h>

#if defined( WIN32 ) || defined ( __MWERKS__ ) || defined( _MSC_VER ) || defined( MINGW32 )
#  ifndef WIN32
#     define WIN32
#  endif
#  include <windows.h>
#endif

#include "GLee.h"

#include "SDL.h"
#include "SDL_opengl.h"

#define APP_NAME "Nightshade "VERSION

#define APP_LOWER_NAME "nightshade"

#include "fmath.h"

#define AU 149597870.691
#define MY_MAX(a,b) (((a)>(b))?(a):(b))
#define MY_MIN(a,b) (((a)<(b))?(a):(b))

#include <cassert>

// For desktop use (versus embedded use) uncomment the following
#define DESKTOP 1
#define EDITION "Community"

// For integration with various media (not used on desktop)
// Used for script sources
#define SCRIPT_REMOVEABLE_DISK "dvd"
#define SCRIPT_USB_DISK "usb"
#define SCRIPT_INTERNAL_DISK "internal"

#define SCRIPT_LOCAL_DISK ""

// Planetarium vendors may set and use specific defines here
// Note that EDITION will be printed at startup for clarity
// Goal is to minimize these through configuration options!

//#define LSS 1
//#define NAV 1

#ifdef LSS
    #undef EDITION
    #define EDITION "LSS"
    #undef SCRIPT_REMOVEABLE_DISK
    #undef SCRIPT_USB_DISK
    #undef SCRIPT_INTERNAL_DISK
    #undef SCRIPT_LOCAL_DISK
    #define SCRIPT_REMOVEABLE_DISK "deepsky"
    #define SCRIPT_USB_DISK "planets"
    #define SCRIPT_INTERNAL_DISK "basis"
    #define SCRIPT_LOCAL_DISK "shows/scripts/"
#endif

#ifdef NAV
    #undef EDITION
    #define EDITION "LSS Navigation"
#endif

#endif /*_NIGHTSHADE_H_*/
