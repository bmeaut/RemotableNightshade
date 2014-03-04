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

#ifndef NSSTATE_H_
#define NSSTATE_H_

#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <limits>
#include <string>
#include <fastdb/fastdb.h>
#include <boost/lexical_cast.hpp>

typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> CharAlloc;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAlloc> shared_string;

struct ScriptState {
	ScriptState();
	void operator =( const ScriptState& );
	enum PlayState { PLAYING, STOPPED, PAUSED, FF, UNK };
	PlayState playState;
	double volume;
};

struct MediaState {
	MediaState();
	void operator =( const MediaState& );
	enum PlayState { PLAYING, STOPPED, PAUSED, FF, RW, UNK };
	PlayState playStateVideo;
	PlayState playStateAudio;
	double volume;
};

struct ObserverState {
	ObserverState();
	void operator =( const ObserverState& );

	double latitude;
	double longitude;
	double altitude;
	double heading;
	int day;
	int month;
	int year;
	int hour;
	int minute;
	double seconds;
	char home[128];
	char tz[256];
};

struct ReferenceState {
	ReferenceState( bool initToZero = false );
	void operator =( const ReferenceState& );

	// Misc
	short atmosphere;
	short landscape;
	short cardinal_points;
	short moon_scaled;
	short clouds;
	short show_tui_date_time;
	short show_tui_short_obj_info;
	short precession_circle;
	short circumpolar_circle;

	// Grids
	short azimuthal_grid;
	short equatorial_grid;
	short galactic_grid;

	// lines
	short meridian_line;
	short tropic_lines;
	short ecliptic_line;
	short equator_line;

	char selected_landscape[128];
};

class SettingsState {
public:
	SettingsState();
	void operator =( const SettingsState& );

	struct {
		char selected_language[128];
		char sky_language[128];

		// Stars
		float max_mag_star_name;      // [-1.50, 10]
		float star_twinkle_amount; 	  // [0, 1]
		float star_limiting_mag;      // [0, 7]
		short stars;
		float star_mag_scale;         // [0, 30]

		// Effects
		short antialias_lines;
		float line_width;                 // [0.125, 5]
		short light_travel_time;
		float light_pollution_luminance;  // [0, 30]
		short manual_zoom;
		float max_mag_nebula_name;    // [0, 100]
		float milky_way_intensity;    // [0, 100]
		float auto_move_duration;     // [1, 10]
		float zoom_offset;            // [-0.50, 0.50]
		int   meteors_zhr;            // [0, 200000], the upper bound is arbitrary
		float planet_size_limit;      // [0, 25]
		float star_size_limit;        // [0, 25]
		float star_scale;             // [0, 25]
		short point_star;
		short planets;
		float flyto_duration;

		// Colors, [0, 1]
		float const_lines[3];
		float const_names[3];
		float const_art[3];
		float const_bounds[3];
		float cardinal_points[3];
		float planet_names[3];
		float planet_orbits[3];
		float sat_orbits[3];
		float planet_trails[3];
		float meridian_line[3];
		float azimuth_grid[3];
		float equator_grid[3];
		float galactic_grid[3];
		float equator_line[3];
		float ecliptic_line[3];
		float nebula_names[3];
		float nebula_circles[3];
		float precession_circle[3];
		float circumpolar_circle[3];
	} m_state;
};

class ObjectsState {
public:
	ObjectsState();
	void operator =( const ObjectsState& );

	struct {
		short star_labels;
		short planet_labels;
		short planet_orbits;
		short planet_trails;
		short const_labels;
		short const_lines;
		short const_art;
		short const_boundary;
		short dso_labels;
		short manual_zoom;
		char sky_culture[128];
	} m_state;
};

struct TZRecord {
	TZRecord():tzName(NULL){};
	TZRecord( const char* name ):
		tzName(name){
	}

    // Anything that's used in a query should be a string (char*) or you're entering a world
    // of pain. It's all fun and games until you query from a non-C++ source where there's no
    // type information which this DB depends so heavily upon. Just say no.
	const char* tzName;
    TYPE_DESCRIPTOR((KEY(tzName, HASHED|INDEXED)));
};

struct CultureRecord {
	CultureRecord():cultureName(NULL), value(NULL){};
	CultureRecord( const char* name, const char* val ):
		cultureName(name), value(val){
	}

	const char* cultureName;
	const char* value;
    TYPE_DESCRIPTOR((KEY(cultureName, HASHED|INDEXED),
					FIELD(value)));
};

struct ObjectRecord {
	enum OBJECT_TYPE {
		OBJECT_UNINITIALIZED,
		OBJECT_STAR,
		OBJECT_PLANET,
		OBJECT_NEBULA,
		OBJECT_CONSTELLATION,
		OBJECT_TELESCOPE
	};

	ObjectRecord():
		englishName(NULL),
		shortName(NULL),
		nameI18(NULL),
		hpNumber(std::numeric_limits<unsigned int>::max()),
		type(OBJECT_UNINITIALIZED),
		mag(std::numeric_limits<float>::max()),
		distance(std::numeric_limits<float>::max()),
		units(NULL){

		tmp = boost::lexical_cast<std::string>(type);
		strType = tmp.c_str();
	}
	ObjectRecord( const char* en, const char* loc, unsigned int hp, OBJECT_TYPE t, float m, float d ):
		// Star Constructor
		englishName(en),
		shortName(NULL),
		nameI18(loc),
		hpNumber(hp),
		type(t),
		mag(m),
		distance(d),
		units("LY"){

		tmp = boost::lexical_cast<std::string>(type);
		strType = tmp.c_str();
	}
	ObjectRecord( const char* en, const char* loc, OBJECT_TYPE t, float m ):
		// Nebula Constructor
		englishName(en),
		shortName(NULL),
		nameI18(loc),
		hpNumber(std::numeric_limits<unsigned int>::max()),
		type(t),
		mag(m),
		distance(std::numeric_limits<float>::max()),
		units(NULL){

		tmp = boost::lexical_cast<std::string>(type);
		strType = tmp.c_str();
	}
	ObjectRecord( const char* en, const char* loc, const char* abv, OBJECT_TYPE t ):
		// Constellation Constructor
		englishName(en),
		shortName(abv),
		nameI18(loc),
		hpNumber(std::numeric_limits<unsigned int>::max()),
		type(t),
		mag(std::numeric_limits<float>::max()),
		distance(std::numeric_limits<float>::max()),
		units(NULL){

		tmp = boost::lexical_cast<std::string>(type);
		strType = tmp.c_str();
	}
	ObjectRecord( const char* en, const char* loc, OBJECT_TYPE t ):
		// Planet constructor
		englishName(en),
		shortName(NULL),
		nameI18(loc),
		hpNumber(std::numeric_limits<unsigned int>::max()),
		type(t),
		mag(std::numeric_limits<float>::max()),
		distance(std::numeric_limits<float>::max()),
		units("AU"){

		tmp = boost::lexical_cast<std::string>(type);
		strType = tmp.c_str();
	}
    TYPE_DESCRIPTOR((KEY(englishName, HASHED),
					FIELD(shortName),
					KEY(nameI18, HASHED),
					FIELD(hpNumber),
					KEY(type, INDEXED),
					KEY(strType, HASHED),
					FIELD(mag),
					FIELD(distance),
					FIELD(units)));

    // Anything that's used in a query, that is all keys, should be a string (char*) or you're
    // entering a world of pain. It's all fun and games until you query from a non-C++ source
    // where there's no type information which this DB depends so heavily upon. Just say no.
	char const* englishName;
	char const* shortName;
	char const* nameI18;
	char const* strType;
	unsigned int hpNumber;
	unsigned int type;
	float mag;
	float distance;
	char const* units;
	std::string tmp;
};


class NshadeWriteState {
	friend class NshadeReadState;

public:
	NshadeWriteState( boost::interprocess::managed_shared_memory* );
	void Media( const MediaState& );
	void Script( const ScriptState& );
	void Observer( const ObserverState& );
	void Reference( const ReferenceState& );
	void Settings( const SettingsState& );
	void Objects( const ObjectsState& );

	void SkyLanguages( const std::string& );
	void Landscapes( const std::string& );

private:
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_globalMutex;

	boost::interprocess::offset_ptr<MediaState> m_media;
	boost::interprocess::offset_ptr<ScriptState> m_script;
	boost::interprocess::offset_ptr<ObserverState> m_observer;
	boost::interprocess::offset_ptr<ReferenceState> m_reference;
	boost::interprocess::offset_ptr<SettingsState> m_settings;
	boost::interprocess::offset_ptr<ObjectsState> m_objects;

	shared_string m_languages;
	shared_string m_landscapes;
};

class NshadeReadState {
public:
	NshadeReadState( boost::interprocess::managed_shared_memory* );
	void Media( MediaState& );
	void Script( ScriptState& );
	void Observer( ObserverState& );
	void Reference( ReferenceState& );
	void Settings( SettingsState& );
	void Objects( ObjectsState& );

	void operator =( const NshadeWriteState& obj );
	void CopyStaticData( const NshadeWriteState& obj );

	void SkyLanguages( std::string& );
	void Landscapes( std::string& );

private:
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_readMutex;
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_globalMutex;

	boost::interprocess::offset_ptr<MediaState> m_media;
	boost::interprocess::offset_ptr<ScriptState> m_script;
	boost::interprocess::offset_ptr<ObserverState> m_observer;
	boost::interprocess::offset_ptr<ReferenceState> m_reference;
	boost::interprocess::offset_ptr<SettingsState> m_settings;
	boost::interprocess::offset_ptr<ObjectsState> m_objects;

	shared_string m_languages;
	shared_string m_landscapes;
};

#endif
