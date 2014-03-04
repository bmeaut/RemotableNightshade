/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Author: Trystan A. Larey-Williams
 * Copyright (C) 2010 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Nightshade is a trademark of Digitalis Education Solutions, Inc.
 * See the TRADEMARKS file for trademark usage requirements.
 *
 */

#include "nshade_state.h"
#include <stdlib.h>

using namespace boost::interprocess;

static const float floatMax = std::numeric_limits<float>::max();
static const double doubleMax = std::numeric_limits<double>::max();
static const short shortMax = std::numeric_limits<short>::max();
static const int intMax = std::numeric_limits<int>::max();

///////////////////////////////////////////////////////////////////////////////
// Observer ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ObserverState::ObserverState() {
	latitude = longitude = altitude = heading = doubleMax;
	year = month = day = hour = minute = intMax;
	seconds = doubleMax;
	memset( home, 0, sizeof(home) );
	memset( tz, 0, sizeof(tz) );
}

void ObserverState::operator =( const ObserverState& obj ) {
	latitude = obj.latitude;
	longitude = obj.longitude;
	altitude = obj.altitude;
	heading = obj.heading;
	year = obj.year;
	month = obj.month;
	day = obj.day;
	hour = obj.hour;
	minute = obj.minute;
	seconds = obj.seconds;
	memcpy( home, obj.home, sizeof(home) );
	memcpy( tz, obj.tz, sizeof(tz) );
}

///////////////////////////////////////////////////////////////////////////////
// Scripts ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ScriptState::ScriptState() {
	volume = doubleMax;
	playState = ScriptState::UNK;
}

void ScriptState::operator =( const ScriptState& obj ) {
	volume = obj.volume;
	playState = obj.playState;
}

///////////////////////////////////////////////////////////////////////////////
// Media //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
MediaState::MediaState() {
	volume = doubleMax;
	playStateVideo = MediaState::UNK;
	playStateAudio = MediaState::UNK;
}

void MediaState::operator =( const MediaState& obj ) {
	volume = obj.volume;
	playStateVideo = obj.playStateVideo;
	playStateAudio = obj.playStateAudio;
}

///////////////////////////////////////////////////////////////////////////////
// References /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ReferenceState::ReferenceState( bool initToZero ) {
	if( initToZero ) {
		atmosphere = 0;
		landscape = 0;
		cardinal_points = 0;
		azimuthal_grid = 0;
		equatorial_grid = 0;
		galactic_grid = 0;
		meridian_line = 0;
		tropic_lines = 0;
		ecliptic_line = 0;
		equator_line = 0;
		moon_scaled = 0;
		clouds = 0;
		precession_circle = 0;
		circumpolar_circle = 0;
		show_tui_date_time = 0;
		show_tui_short_obj_info = 0;
	}
	else {
		atmosphere = -1;
		landscape = -1;
		cardinal_points = -1;
		azimuthal_grid = -1;
		equatorial_grid = -1;
		galactic_grid = -1;
		meridian_line = -1;
		tropic_lines = -1;
		ecliptic_line = -1;
		equator_line = -1;
		moon_scaled = -1;
		clouds = -1;
		precession_circle = -1;
		circumpolar_circle = -1;
		show_tui_date_time = -1;
		show_tui_short_obj_info = -1;
	}
	memset( selected_landscape, 0, sizeof(selected_landscape) );
}

void ReferenceState::operator =( const ReferenceState& obj  ) {
	atmosphere = obj.atmosphere;
	landscape = obj.landscape;
	cardinal_points = obj.cardinal_points;
	azimuthal_grid = obj.azimuthal_grid;
	equatorial_grid = obj.equatorial_grid;
	galactic_grid = obj.galactic_grid;
	meridian_line = obj.meridian_line;
	tropic_lines = obj.tropic_lines;
	ecliptic_line = obj.ecliptic_line;
	equator_line = obj.equator_line;
	moon_scaled = obj.moon_scaled;
	clouds = obj.clouds;
	precession_circle = obj.precession_circle;
	circumpolar_circle = obj.circumpolar_circle;
	show_tui_date_time = obj.show_tui_date_time;
	show_tui_short_obj_info = obj.show_tui_short_obj_info;
	memcpy( selected_landscape, obj.selected_landscape, sizeof(selected_landscape) );
}

///////////////////////////////////////////////////////////////////////////////
// SETTINGS ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SettingsState::SettingsState()
{
	m_state.max_mag_star_name = floatMax;
	m_state.star_twinkle_amount = floatMax;
	m_state.star_limiting_mag = floatMax;
	m_state.antialias_lines = shortMax;
	m_state.line_width = floatMax;
	m_state.light_travel_time = shortMax;
	m_state.light_pollution_luminance = floatMax;
	m_state.manual_zoom = shortMax;
	m_state.stars = shortMax;
	m_state.star_mag_scale = floatMax;
	m_state.max_mag_nebula_name = floatMax;
	m_state.milky_way_intensity = floatMax;
	m_state.auto_move_duration = floatMax;
	m_state.zoom_offset = floatMax;
	m_state.const_lines[0] = floatMax;
	m_state.const_names[0] = floatMax;
	m_state.const_art[0] = floatMax;
	m_state.const_bounds[0] = floatMax;
	m_state.cardinal_points[0] = floatMax;
	m_state.planet_names[0] = floatMax;
	m_state.planet_orbits[0] = floatMax;
	m_state.sat_orbits[0] = floatMax;
	m_state.planet_trails[0] = floatMax;
	m_state.meridian_line[0] = floatMax;
	m_state.azimuth_grid[0] = floatMax;
	m_state.equator_grid[0] = floatMax;
	m_state.galactic_grid[0] = floatMax;
	m_state.equator_line[0] = floatMax;
	m_state.ecliptic_line[0] = floatMax;
	m_state.nebula_names[0] = floatMax;
	m_state.nebula_circles[0] = floatMax;
	m_state.precession_circle[0] = floatMax;
	m_state.circumpolar_circle[0] = floatMax;
	m_state.meteors_zhr = intMax;
	m_state.planet_size_limit = floatMax;
	m_state.star_size_limit = floatMax;
	m_state.star_scale = floatMax;
	m_state.point_star = shortMax;
	m_state.planets = shortMax;
	m_state.flyto_duration = shortMax;

	memset( m_state.selected_language, 0, sizeof(m_state.selected_language) );
	memset( m_state.sky_language, 0, sizeof(m_state.sky_language) );
}

void SettingsState::operator =( const SettingsState& obj ){
	memcpy( &m_state, &obj.m_state, sizeof(m_state) );
}

///////////////////////////////////////////////////////////////////////////////
// Objects ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ObjectsState::ObjectsState() {
	m_state.manual_zoom = shortMax;
	m_state.const_art = shortMax;
	m_state.const_boundary = shortMax;
	m_state.const_labels = shortMax;
	m_state.const_lines = shortMax;
	m_state.dso_labels = shortMax;
	m_state.planet_labels = shortMax;
	m_state.planet_orbits = shortMax;
	m_state.planet_trails = shortMax;
	m_state.star_labels = shortMax;
	memset( m_state.sky_culture, 0, sizeof(m_state.sky_culture) );
}

void ObjectsState::operator =( const ObjectsState& obj ) {
	memcpy( &m_state, &obj.m_state, sizeof(m_state) );
}

///////////////////////////////////////////////////////////////////////////////
// ReadState //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NshadeReadState::NshadeReadState( boost::interprocess::managed_shared_memory* shm ) :
		m_languages(CharAlloc(shm->get_segment_manager())),
		m_landscapes(CharAlloc(shm->get_segment_manager()))
{
	m_readMutex = shm->find_or_construct<interprocess_mutex>("readState_mutex")();
	m_globalMutex = shm->find_or_construct<interprocess_mutex>("globalState_mutex")();
	m_media = shm->find_or_construct<MediaState>("read_media")();
	m_script = shm->find_or_construct<ScriptState>("read_script")();
	m_observer = shm->find_or_construct<ObserverState>("read_observer")();
	m_reference = shm->find_or_construct<ReferenceState>("read_reference")();
	m_settings = shm->find_or_construct<SettingsState>("read_settings")();
	m_objects = shm->find_or_construct<ObjectsState>("read_objects")();
}

void NshadeReadState::operator=( const NshadeWriteState& obj ) {
	m_readMutex->lock();
	m_globalMutex->lock();
		*m_media = *obj.m_media;
		*m_script = *obj.m_script;
		*m_reference = *obj.m_reference;
		*m_observer = *obj.m_observer;
		*m_settings = *obj.m_settings;
		*m_objects = *obj.m_objects;
	m_globalMutex->unlock();
	m_readMutex->unlock();
}

void NshadeReadState::CopyStaticData( const NshadeWriteState& obj ) {
	m_readMutex->lock();
	m_globalMutex->lock();
		m_languages = obj.m_languages;
		m_landscapes = obj.m_landscapes;
	m_globalMutex->unlock();
	m_readMutex->unlock();
}

void NshadeReadState::Media( MediaState& obj ) {
	m_readMutex->lock();
		obj = *m_media;
	m_readMutex->unlock();
}

void NshadeReadState::Script( ScriptState& obj ) {
	m_readMutex->lock();
		obj = *m_script;
	m_readMutex->unlock();
}

void NshadeReadState::Observer( ObserverState& obj ) {
	m_readMutex->lock();
		obj = *m_observer;
	m_readMutex->unlock();
}

void NshadeReadState::Reference( ReferenceState& obj ) {
	m_readMutex->lock();
		obj = *m_reference;
	m_readMutex->unlock();
}

void NshadeReadState::Settings( SettingsState& obj ) {
	m_readMutex->lock();
		obj = *m_settings;
	m_readMutex->unlock();
}

void NshadeReadState::Objects( ObjectsState& obj ) {
	m_readMutex->lock();
		obj = *m_objects;
	m_readMutex->unlock();
}

void NshadeReadState::SkyLanguages( std::string& langs ) {
	m_readMutex->lock();
		langs = m_languages.c_str();
	m_readMutex->unlock();
}

void NshadeReadState::Landscapes( std::string& lnds ) {
	m_readMutex->lock();
		lnds = m_landscapes.c_str();
	m_readMutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// WriteState /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NshadeWriteState::NshadeWriteState( boost::interprocess::managed_shared_memory* shm ) :
		m_languages(CharAlloc(shm->get_segment_manager())),
		m_landscapes(CharAlloc(shm->get_segment_manager()))
{
	m_globalMutex = shm->find_or_construct<interprocess_mutex>("globalState_mutex")();
	m_media = shm->find_or_construct<MediaState>("write_media")();
	m_script = shm->find_or_construct<ScriptState>("write_script")();
	m_observer = shm->find_or_construct<ObserverState>("write_observer")();
	m_reference = shm->find_or_construct<ReferenceState>("write_reference")( true );
	m_settings = shm->find_or_construct<SettingsState>("write_settings")();
	m_objects = shm->find_or_construct<ObjectsState>("write_objects")();
}

void NshadeWriteState::Media( const MediaState& obj ) {
	m_globalMutex->lock();
	if( obj.volume != doubleMax )
		m_media->volume = obj.volume;
	if( obj.playStateVideo != MediaState::UNK )
		m_media->playStateVideo = obj.playStateVideo;
	if( obj.playStateAudio != MediaState::UNK )
		m_media->playStateAudio = obj.playStateAudio;
	m_globalMutex->unlock();
}

void NshadeWriteState::Script( const ScriptState& obj ) {
	m_globalMutex->lock();
		if( obj.volume != doubleMax )
			m_script->volume = obj.volume;
		if( obj.playState != ScriptState::UNK )
			m_script->playState = obj.playState;
	m_globalMutex->unlock();
}

void NshadeWriteState::Reference( const ReferenceState& obj ) {
	m_globalMutex->lock();
		if( obj.atmosphere > -1 )
			m_reference->atmosphere = obj.atmosphere;
		if( obj.landscape > -1 )
			m_reference->landscape = obj.landscape;
		if( obj.cardinal_points > -1 )
			m_reference->cardinal_points = obj.cardinal_points;
		if( obj.azimuthal_grid > -1 )
			m_reference->azimuthal_grid = obj.azimuthal_grid;
		if( obj.equatorial_grid > -1 )
			m_reference->equatorial_grid = obj.equatorial_grid;
		if( obj.galactic_grid > -1 )
			m_reference->galactic_grid = obj.galactic_grid;
		if( obj.meridian_line > -1 )
			m_reference->meridian_line = obj.meridian_line;
		if( obj.tropic_lines > -1 )
			m_reference->tropic_lines = obj.tropic_lines;
		if( obj.ecliptic_line > -1 )
			m_reference->ecliptic_line = obj.ecliptic_line;
		if( obj.equator_line > -1 )
			m_reference->equator_line = obj.equator_line;
		if( obj.moon_scaled > -1 )
			m_reference->moon_scaled = obj.moon_scaled;
		if( obj.clouds > -1 )
			m_reference->clouds = obj.clouds;
		if( obj.precession_circle > -1 )
			m_reference->precession_circle = obj.precession_circle;
		if( obj.circumpolar_circle > -1 )
			m_reference->circumpolar_circle = obj.circumpolar_circle;
		if( obj.show_tui_date_time > -1 )
			m_reference->show_tui_date_time = obj.show_tui_date_time;
		if( obj.show_tui_short_obj_info > -1 )
			m_reference->show_tui_short_obj_info = obj.show_tui_short_obj_info;
		if( obj.selected_landscape[0] )
			memcpy( m_reference->selected_landscape, obj.selected_landscape, sizeof(m_reference->selected_landscape) );
	m_globalMutex->unlock();
}

void NshadeWriteState::Observer( const ObserverState& obj ) {
	m_globalMutex->lock();
		if( obj.altitude != doubleMax )
			m_observer->altitude = obj.altitude;
		if( obj.heading != doubleMax )
			m_observer->heading = obj.heading;
		if( obj.latitude != doubleMax )
			m_observer->latitude = obj.latitude;
		if( obj.longitude != doubleMax )
			m_observer->longitude = obj.longitude;
		if( obj.year != intMax )
			m_observer->year = obj.year;
		if( obj.month != intMax )
			m_observer->month = obj.month;
		if( obj.day != intMax )
			m_observer->day = obj.day;
		if( obj.hour != intMax )
			m_observer->hour = obj.hour;
		if( obj.minute != intMax )
			m_observer->minute = obj.minute;
		if( obj.seconds != doubleMax )
			m_observer->seconds = obj.seconds;
		if( obj.home[0] )
			memcpy( m_observer->home, obj.home, sizeof(m_observer->home) );
		if( obj.tz[0] )
			memcpy( m_observer->tz, obj.tz, sizeof(m_observer->tz) );
	m_globalMutex->unlock();
}

void NshadeWriteState::Settings( const SettingsState& obj ) {
	m_globalMutex->lock();

	if( obj.m_state.selected_language[0] )
		memcpy( m_settings->m_state.selected_language, obj.m_state.selected_language,
				sizeof( m_settings->m_state.selected_language));
	if( obj.m_state.sky_language[0] )
		memcpy( m_settings->m_state.sky_language, obj.m_state.sky_language,
				sizeof(m_settings->m_state.sky_language) );
	if( obj.m_state.max_mag_star_name != floatMax )
		m_settings->m_state.max_mag_star_name = obj.m_state.max_mag_star_name;
	if( obj.m_state.star_twinkle_amount != floatMax )
		m_settings->m_state.star_twinkle_amount = obj.m_state.star_twinkle_amount;
	if( obj.m_state.star_limiting_mag != floatMax )
		m_settings->m_state.star_limiting_mag = obj.m_state.star_limiting_mag;
	if( obj.m_state.antialias_lines != shortMax )
		m_settings->m_state.antialias_lines = obj.m_state.antialias_lines;
	if( obj.m_state.line_width != floatMax )
		m_settings->m_state.line_width = obj.m_state.line_width;
	if( obj.m_state.light_travel_time != shortMax )
		m_settings->m_state.light_travel_time = obj.m_state.light_travel_time;
	if( obj.m_state.light_pollution_luminance != floatMax )
		m_settings->m_state.light_pollution_luminance = obj.m_state.light_pollution_luminance;
	if( obj.m_state.manual_zoom != shortMax )
		m_settings->m_state.manual_zoom = obj.m_state.manual_zoom;
	if( obj.m_state.max_mag_nebula_name != floatMax )
		m_settings->m_state.max_mag_nebula_name = obj.m_state.max_mag_nebula_name;
	if( obj.m_state.milky_way_intensity != floatMax )
		m_settings->m_state.milky_way_intensity = obj.m_state.milky_way_intensity;
	if( obj.m_state.auto_move_duration != floatMax )
		m_settings->m_state.auto_move_duration = obj.m_state.auto_move_duration;
	if( obj.m_state.zoom_offset != floatMax )
		m_settings->m_state.zoom_offset = obj.m_state.zoom_offset;
	if( obj.m_state.const_lines[0] != floatMax )
		memcpy( m_settings->m_state.const_lines, obj.m_state.const_lines, sizeof(m_settings->m_state.const_lines));
	if( obj.m_state.const_names[0] != floatMax )
		memcpy( m_settings->m_state.const_names, obj.m_state.const_names, sizeof(m_settings->m_state.const_names));
	if( obj.m_state.const_art[0] != floatMax )
		memcpy( m_settings->m_state.const_art, obj.m_state.const_art, sizeof(m_settings->m_state.const_art));
	if( obj.m_state.const_bounds[0] != floatMax )
		memcpy( m_settings->m_state.const_bounds, obj.m_state.const_bounds, sizeof(m_settings->m_state.const_bounds));
	if( obj.m_state.cardinal_points[0] != floatMax )
		memcpy( m_settings->m_state.cardinal_points, obj.m_state.cardinal_points, sizeof(m_settings->m_state.cardinal_points));
	if( obj.m_state.planet_names[0] != floatMax )
		memcpy( m_settings->m_state.planet_names, obj.m_state.planet_names, sizeof(m_settings->m_state.planet_names));
	if( obj.m_state.planet_orbits[0] != floatMax )
		memcpy( m_settings->m_state.planet_orbits, obj.m_state.planet_orbits, sizeof(m_settings->m_state.planet_orbits));
	if( obj.m_state.sat_orbits[0] != floatMax )
		memcpy( m_settings->m_state.sat_orbits, obj.m_state.sat_orbits, sizeof(m_settings->m_state.sat_orbits));
	if( obj.m_state.planet_trails[0] != floatMax )
		memcpy( m_settings->m_state.planet_trails, obj.m_state.planet_trails, sizeof(m_settings->m_state.planet_trails));
	if( obj.m_state.meridian_line[0] != floatMax )
		memcpy( m_settings->m_state.meridian_line, obj.m_state.meridian_line, sizeof(m_settings->m_state.meridian_line));
	if( obj.m_state.azimuth_grid[0] != floatMax )
		memcpy( m_settings->m_state.azimuth_grid, obj.m_state.azimuth_grid, sizeof(m_settings->m_state.azimuth_grid));
	if( obj.m_state.equator_grid[0] != floatMax )
		memcpy( m_settings->m_state.equator_grid, obj.m_state.equator_grid, sizeof(m_settings->m_state.equator_grid));
	if( obj.m_state.galactic_grid[0] != floatMax )
		memcpy( m_settings->m_state.galactic_grid, obj.m_state.galactic_grid, sizeof(m_settings->m_state.galactic_grid));
	if( obj.m_state.equator_line[0] != floatMax )
		memcpy( m_settings->m_state.equator_line, obj.m_state.equator_line, sizeof(m_settings->m_state.equator_line));
	if( obj.m_state.ecliptic_line[0] != floatMax )
		memcpy( m_settings->m_state.ecliptic_line, obj.m_state.ecliptic_line, sizeof(m_settings->m_state.ecliptic_line));
	if( obj.m_state.nebula_names[0] != floatMax )
		memcpy( m_settings->m_state.nebula_names, obj.m_state.nebula_names, sizeof(m_settings->m_state.nebula_names));
	if( obj.m_state.nebula_circles[0] != floatMax )
		memcpy( m_settings->m_state.nebula_circles, obj.m_state.nebula_circles, sizeof(m_settings->m_state.nebula_circles));
	if( obj.m_state.precession_circle[0] != floatMax )
		memcpy( m_settings->m_state.precession_circle, obj.m_state.precession_circle, sizeof(m_settings->m_state.precession_circle));
	if( obj.m_state.circumpolar_circle[0] != floatMax )
		memcpy( m_settings->m_state.circumpolar_circle, obj.m_state.circumpolar_circle, sizeof(m_settings->m_state.circumpolar_circle));
	if( obj.m_state.meteors_zhr != intMax )
		m_settings->m_state.meteors_zhr = obj.m_state.meteors_zhr;
	if( obj.m_state.stars != shortMax )
		m_settings->m_state.stars = obj.m_state.stars;
	if( obj.m_state.star_mag_scale != floatMax )
		m_settings->m_state.star_mag_scale = obj.m_state.star_mag_scale;
	if( obj.m_state.planet_size_limit != floatMax )
		m_settings->m_state.planet_size_limit = obj.m_state.planet_size_limit;
	if( obj.m_state.star_size_limit != floatMax )
		m_settings->m_state.star_size_limit = obj.m_state.star_size_limit;
	if( obj.m_state.star_scale != floatMax )
		m_settings->m_state.star_scale = obj.m_state.star_scale;
	if( obj.m_state.point_star != shortMax )
		m_settings->m_state.point_star = obj.m_state.point_star;
	if( obj.m_state.planets != shortMax )
		m_settings->m_state.planets = obj.m_state.planets;
	if( obj.m_state.flyto_duration != shortMax )
		m_settings->m_state.flyto_duration = obj.m_state.flyto_duration;

	m_globalMutex->unlock();
}

void NshadeWriteState::SkyLanguages( const std::string& langs ) {
	m_globalMutex->lock();
		m_languages = langs.c_str();
	m_globalMutex->unlock();
}

void NshadeWriteState::Landscapes( const std::string& lnds ) {
	m_globalMutex->lock();
		m_landscapes = lnds.c_str();
	m_globalMutex->unlock();
}

void NshadeWriteState::Objects( const ObjectsState& obj ) {
	m_globalMutex->lock();
		if( obj.m_state.manual_zoom != shortMax )
			m_objects->m_state.manual_zoom = obj.m_state.manual_zoom;
		if( obj.m_state.const_art != shortMax )
			m_objects->m_state.const_art = obj.m_state.const_art;
		if( obj.m_state.const_boundary != shortMax )
			m_objects->m_state.const_boundary = obj.m_state.const_boundary;
		if( obj.m_state.const_labels != shortMax )
			m_objects->m_state.const_labels = obj.m_state.const_labels;
		if( obj.m_state.const_lines != shortMax )
			m_objects->m_state.const_lines = obj.m_state.const_lines;
		if( obj.m_state.dso_labels != shortMax )
			m_objects->m_state.dso_labels = obj.m_state.dso_labels;
		if( obj.m_state.planet_labels != shortMax )
			m_objects->m_state.planet_labels = obj.m_state.planet_labels;
		if( obj.m_state.planet_orbits != shortMax )
			m_objects->m_state.planet_orbits = obj.m_state.planet_orbits;
		if( obj.m_state.planet_trails != shortMax )
			m_objects->m_state.planet_trails = obj.m_state.planet_trails;
		if( obj.m_state.star_labels != shortMax )
			m_objects->m_state.star_labels = obj.m_state.star_labels;
		if( obj.m_state.sky_culture[0] )
			memcpy( m_objects->m_state.sky_culture, obj.m_state.sky_culture, sizeof(m_objects->m_state.sky_culture) );
	m_globalMutex->unlock();
}

