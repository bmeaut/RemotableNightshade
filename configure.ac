dnl Process this file with autoconf to produce a configure script.

AC_INIT(nightshade,11.12.1, support@nightshadesoftware.org)
AC_CONFIG_SRCDIR([src/main.cpp])

AC_PREREQ(2.52)

AM_INIT_AUTOMAKE(1.6)
AM_CONFIG_HEADER(config.h)

AM_GNU_GETTEXT

dnl Check for compilers.
AC_PROG_CC
AC_PROG_CXX

CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall -fpermissive" 

dnl Checks for programs.
AC_ISC_POSIX
AC_PROG_LIBTOOL

# Compilation with STLPort
#AM_PATH_STLPORT

dnl Can't find later libGLU if -lm not set here!?!??!
AC_CHECK_LIB(m,fabs,LIBS="$LIBS -lm",AC_MSG_RESULT(math library not found))
AC_CHECK_LIB(rt, shm_open, LIBS="$LIBS -lrt")

BOOST_REQUIRE([1.42.0])
BOOST_SYSTEM([s])
BOOST_FILESYSTEM([s])
BOOST_REGEX([s])

AC_CHECK_PROG([GRAPHICSXX],[GraphicsMagick++-config],[yes],[no])
if test "$GRAPHICSXX" = "no"; then
  AC_MSG_ERROR([*** GraphicsMagick++ library is missing or GraphicsMagick++-config is not in path.])
fi
CXXFLAGS="$CXXFLAGS `GraphicsMagick++-config --cppflags`" 

dnl Need version of boost with interprocess library. Also need uuid (1.42.0 was the first with uuid).
dnl AX_BOOST_BASE([1.42.0],,AC_MSG_ERROR([*** Boost C++ library version 1.42.0 or greater required.]))

AC_C_BIGENDIAN
AC_HEADER_DIRENT

dnl Check for X11.
AC_PATH_XTRA
LIBS="$LIBS $X_LIBS"
CFLAGS="$CFLAGS $X_CFLAGS"

dnl Check for SDL
case "$host" in
*-*-cygwin)
;;
*)
SDL_VERSION=1.2.10
	AM_PATH_SDL($SDL_VERSION,,AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
	NS_CFLAGS="$CFLAGS $SDL_CFLAGS"
	NS_CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
	NS_LIBS="$LIBS $SDL_LIBS"
;;
esac

# Check for libpng
AC_CHECK_LIB(png, png_read_info,[AC_CHECK_HEADER(png.h,NS_LIBS="$NS_LIBS -lpng -lz",)], AC_MSG_ERROR(*** PNG library not found ***), -lz -lm)

# Check for FastDB
AC_CHECK_LIB(fastdb, cli_create,LIBFASTDB="/usr/local/lib/libfastdb.a",AC_MSG_ERROR(*** FastDB library not found. Install libfastdb.a to /usr/local/lib or create a symbolic link. ***))

AC_CHECK_HEADERS(byteswap.h)

OLDCFLAGS="$CFLAGS"
CFLAGS="$NS_CFLAGS"
OLDLIBS="$LIBS"
LIBS="$NS_LIBS"
AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,[AC_CHECK_HEADER(SDL_mixer.h,NS_LIBS="$NS_LIBS -lSDL_mixer",,[#include "SDL.h"])], AC_MSG_WARN(*** SDL_mixer library not found - audio features will be deactivated ***))
AC_CHECK_LIB(SDL_Pango, SDLPango_CreateContext,[AC_CHECK_HEADER(SDL_Pango.h,NS_LIBS="$NS_LIBS -lSDL_Pango",,[#include "SDL.h"])], AC_MSG_ERROR(*** SDL_Pango library not found! ***))
AC_CHECK_HEADERS(byteswap.h SDL_mixer.h)
CFLAGS="$OLDCFLAGS"
LIBS="$OLDLIBS"

if test "$prefix" = "NONE" ; then
   prefix=$ac_default_prefix
fi
datadir=`eval echo "${prefix}/share"`
PKGDATADIR=`eval echo "${datadir}/$PACKAGE"`

AC_DEFINE_UNQUOTED(CONFIG_DATA_DIR, "$PKGDATADIR", [Set the directory where the datas (as textures or star catalogs) are stored.])

localedir=`eval echo "${datadir}/locale"`
AC_DEFINE_UNQUOTED(LOCALEDIR, "$localedir", [Set the directory where the locales (translations) are stored.])

dnl For each host type, set the appropriate options
dnl MACOSX
case "$host" in
*-*-darwin*)
	AC_DEFINE_UNQUOTED(MACOSX,1,[If we are on macosx plateform.])
	NS_LDFLAGS="$NS_LDFLAGS -flat_namespace -undefined suppress -lz "
	NS_CPPFLAGS="$NS_CPPFLAGS -I${prefix}/include -I/opt/local/include -Dmain=SDL_main"
	NS_LIBS="$NS_LIBS -framework OpenGL -lSDLmain -lSDL -lSDL_Pango -lpng"
;;

dnl CYGWIN
*-*-cygwin*)
	AC_DEFINE_UNQUOTED(CYGWIN,1, [If we are on cygwin plateform.])
	NS_LIBS="$NS_LIBS -lopengl32 -lglu32 -lSDLmain -lSDL -mwindows -mthreads "
	NS_CFLAGS="$NS_CFLAGS -I/usr/local/include/SDL -Dmain=SDL_main -DWIN32 -Uunix -mthreads"
	NS_CXXFLAGS="$NS_CXXFLAGS -I/usr/local/include/SDL -Dmain=SDL_main -DWIN32 -Uunix -mthreads"
;;

dnl MINGW32
*-*-mingw32)
	AC_DEFINE_UNQUOTED(MINGW32,1, [If we are on mingw32 plateform.])
	LT_INIT([disable-shared])
	LIBFASTDB="/mingw/lib/libfastdb.a"
	NS_LIBS="$NS_LIBS -lopengl32 -lglu32 `sdl-config --libs` -lws2_32"
	NS_CFLAGS="$NS_CFLAGS `sdl-config --cflags`"
	NS_CXXFLAGS="$NS_CXXFLAGS `sdl-config --cflags` -I/mingw/include/SDL/"
;;

dnl OTHER SYSTEMS
*)
	AC_CHECK_LIB(GL,glEnable)
   	if test "$ac_cv_lib_GL_glEnable" = no ; then
    	AC_CHECK_LIB(MesaGL,glEnable,,AC_MSG_ERROR(GL not found - please install GL or MesaGL))
   	fi

   	AC_CHECK_LIB(GLU,gluLookAt)
   	if test "$ac_cv_lib_GLU_gluLookAt" = no ; then
    	AC_CHECK_LIB(MesaGLU,gluLookAt,,AC_MSG_ERROR(GLU not found - please install GLU or MesaGLU))
   	fi

;;
esac

AC_SUBST(NS_CFLAGS)
AC_SUBST(NS_LIBS)
AC_SUBST(NS_CXXFLAGS)
AC_SUBST(NS_LDFLAGS)
AC_SUBST(NS_CPPFLAGS)
AC_SUBST(LIBFASTDB)

dnl Checks for library functions.
AC_CHECK_FUNCS(toupper strcasecmp strdup sinf cosf tanf asinf acosf atanf expf logf log10f atan2f sqrtf powf modf putenv setlocale tzset strchr pow10 timegm)

AC_CONFIG_FILES([Makefile nightshade.spec src/Makefile src/iniparser/Makefile \
                 src/planetsephems/Makefile src/stellastro/Makefile  data/Makefile \
                 stars/Makefile stars/default/Makefile \
                 doc/Makefile textures/Makefile textures/bodies/Makefile \
                 textures/nebulae/Makefile textures/landscapes/Makefile \
                 textures/constellation-art/Makefile textures/constellation-art/hindu/Makefile \
                 textures/constellation-art/inuit/Makefile textures/color-art/Makefile \
                 textures/color-art/inuit/Makefile data/scripts/Makefile data/sky_cultures/Makefile \
                 data/sky_cultures/aztec/Makefile data/sky_cultures/chinese/Makefile data/sky_cultures/egyptian/Makefile \
                 data/sky_cultures/hindu/Makefile data/sky_cultures/inca/Makefile data/sky_cultures/inuit/Makefile data/sky_cultures/inuit-color/Makefile \
                 data/sky_cultures/lakota/Makefile data/sky_cultures/navajo/Makefile data/sky_cultures/norse/Makefile \
                 data/sky_cultures/polynesian/Makefile data/sky_cultures/western/Makefile data/sky_cultures/western-mod/Makefile \
                 data/sky_cultures/western-color/Makefile data/sky_cultures/western-hevelius/Makefile data/sky_cultures/sami/Makefile \
                 intl/Makefile po/Makefile.in m4/Makefile nscontrol/Makefile nscontrol/src/Makefile])

AC_OUTPUT
