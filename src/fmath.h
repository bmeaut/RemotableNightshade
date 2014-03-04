/*
 * Copyright (C) 2004 Fabien Chereau
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

// Redefine the single precision math functions if not defined
// This must be used in conjunction with the autoconf macro :
// AC_CHECK_FUNCS(sinf cosf tanf asinf acosf atanf expf logf log10f atan2f sqrtf)

#ifndef _FMATH_H_
#define _FMATH_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if !defined(HAVE_POW10) || defined(MACOSX) 
# define pow10(x) pow(10,(x))
#endif

#ifndef WIN32

#ifndef HAVE_ACOSF
# define acosf(x) (float)(acos(x))
#endif
#ifndef HAVE_ASINF
# define asinf(x) (float)(asin(x))
#endif
#ifndef HAVE_ATAN2F
# define atan2f(x, y) (float)(atan2((x),(y)))
#endif
#ifndef HAVE_ATANF
# define atanf(x) (float)(atan(x))
#endif
#ifndef HAVE_COSF
# define cosf(x) (float)(cos(x))
#endif
#ifndef HAVE_EXPF
# define expf(x) (float)(exp(x))
#endif
#ifndef HAVE_POWF
# define powf(x, y) (float)(pow((x),(y)))
#endif
#ifndef HAVE_LOG10F
# define log10f(x) (float)(log10(x))
#endif
#ifndef HAVE_LOGF
# define logf(x) (float)(log(x))
#endif
#ifndef HAVE_SINF
# define sinf(x) (float)(sin(x))
#endif
#ifndef HAVE_SQRTF
# define sqrtf(x) (float)(sqrt(x))
#endif

#endif /* CYGWIN */

#endif /*_FMATH_H_*/
