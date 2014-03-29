AM_CFLAGS = @CFLAGS@ @NS_CFLAGS@
AM_CXXFLAGS = @CXXFLAGS@ @NS_CXXFLAGS@
AM_LDFLAGS = @LDFLAGS@ @NS_LDFLAGS@

bin_PROGRAMS = nightshade
nightshade_SOURCES = object.h object.cpp \
    object_type.h object_base.h object_base.cpp \
    constellation.h constellation.cpp constellation_mgr.h constellation_mgr.cpp \
    nebula.h nebula.cpp nebula_mgr.h nebula_mgr.cpp \
    planet.h planet.cpp solarsystem.h solarsystem.cpp \
    hip_star.h hip_star.cpp hip_star_mgr.h hip_star_mgr.cpp \
    main.cpp sdl_facade.h sdl_facade.cpp core.h core.cpp \
    utility.h utility.cpp geodesic_grid.cpp geodesic_grid.h zone_array.cpp zone_array.h zone_data.h vecmath.h \
    sphere_geometry.cpp sphere_geometry.h hip_star_wrapper.cpp hip_star_wrapper.h \
    atmosphere.h atmosphere.cpp grid.h grid.cpp navigator.h navigator.cpp draw.h draw.cpp \
    s_texture.h s_texture.cpp s_font.h s_font.cpp string_array.cpp string_array.h \
    orbit.h solve.h orbit.cpp skylight.h skylight.cpp skybright.h skybright.cpp \
    tone_reproductor.h tone_reproductor.cpp init_parser.h init_parser.cpp s_gui.h s_gui.cpp \
    ui.h ui.cpp ui-lss.hpp ui_conf.cpp ui_tuiconf.cpp projector.h \
    projector.cpp custom_projector.cpp custom_projector.h stereographic_projector.cpp \
    spheric_mirror_projector.cpp stereographic_projector.h spheric_mirror_projector.h \
    spheric_mirror_calculator.cpp spheric_mirror_calculator.h \
    viewport_distorter.cpp viewport_distorter.h \
    observer.h mapping.cpp mapping.h mapping_classes.cpp mapping_classes.h \
	observer.cpp fisheye_projector.h fisheye_projector.cpp landscape.h landscape.cpp \
	callbacks.hpp callback_helpers.hpp s_tui.h s_tui.cpp nightshade.h fmath.h \
	vecmath.h bytes.h meteor.h meteor.cpp meteor_mgr.h meteor_mgr.cpp \
	sky_localizer.h sky_localizer.cpp command_interface.h command_interface.cpp \
	command_nshade.cpp command_nshade.h \
	app_command_interface.h app_command_interface.cpp script_mgr.h script_mgr.cpp script.h \
	script.cpp image_mgr.h image_mgr.cpp image.h image.cpp audio.h audio.cpp \
	loadingbar.h loadingbar.cpp fader.h gettext.h translator.cpp app.cpp \
    shared_data.cpp shared_data.h \
	external_viewer.h external_viewer.cpp GLee.h GLee.c app_settings.h app_settings.cpp \
	signals.cpp signals.h program_object.cpp program_object.h shader.cpp shader.h \
	night_shader.cpp night_shader.h bump_shader.cpp bump_shader.h \
	ring_shader.cpp ring_shader.h ringed_shader.cpp ringed_shader.h named_sockets.cpp named_sockets.h \
    mongoose-cpp/mongoose.c mongoose-cpp/mongoose.h mongoose-cpp/mongcpp.cpp mongoose-cpp/mongcpp.h \
    mongoose-cpp/mwebapi.cpp mongoose-cpp/mwebapi.h \
    mongoose-cpp/json/json.h mongoose-cpp/json/json-forwards.h mongoose-cpp/jsoncpp.cpp

nightshade_CFLAGS = `pkg-config --cflags libmicrohttpd`
nightshade_LDFLAGS = `GraphicsMagick++-config --libs` `pkg-config --libs libmicrohttpd`
nightshade_LDADD = $(top_builddir)/nscontrol/src/libnscontrol.la \
$(BOOST_SYSTEM_LIBS) \
$(BOOST_FILESYSTEM_LIBS) \
$(BOOST_REGEX_LIBS) \
@LIBFASTDB@ \
$(top_builddir)/src/planetsephems/libstellplanet.a \
$(top_builddir)/src/stellastro/libstellastro.a \
$(top_builddir)/src/iniparser/libiniparser.a \
@NS_LIBS@ \
@LIBINTL@

INCLUDES = -I$(top_builddir)/nscontrol/src -I$(top_srcdir)/src/planetsephems -I$(top_srcdir)/src/stellastro -I$(top_srcdir)/src/iniparser -I$(top_srcdir)/src/mongoose-cpp -I$(top_srcdir)/intl

SUBDIRS = planetsephems stellastro iniparser ../nscontrol
noinst_HEADERS = translations.h translator.h app.h
