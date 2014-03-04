/*
 * Author: Trystan Larey-Williams
 * Copyright (C) 2010 Digitalis Education Solutions, Inc.
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

#ifndef __SHARED_DATA_H__
#define __SHARED_DATA_H__

#include <nshade_shared_memory.h>
#include <fastdb/fastdb.h>

class ISharedData {
public:
	virtual ~ISharedData(){};
	virtual bool ReadRT( std::string& data ) = 0;
	virtual void WriteRT( std::string data ) = 0;
	virtual bool Read( std::string& data ) = 0;
	virtual void Write( std::string data ) = 0;
	virtual void Observer( const ObserverState& ) = 0;
	virtual void Script( const ScriptState& ) = 0;
	virtual void Media( const MediaState& ) = 0;
	virtual void References( const ReferenceState& ) = 0;
	virtual void Settings( const SettingsState& ) = 0;
	virtual void Objects( const ObjectsState& ) = 0;
	virtual void Landscapes( const std::string& ) = 0;
	virtual void SkyLanguages( const std::string& ) = 0;
	virtual void SetCoveLightSystem( const std::string& ) = 0;
	virtual void CopyStaticData( void ) = 0;
	virtual void DataPush( void ) = 0;
	virtual void Disconnect( void ) = 0;
	virtual dbDatabase* const DB( void ) = 0;
	virtual boost::interprocess::offset_ptr<NshadeConf> Config( void ) = 0;
};

class SharedDataC : public ISharedData {
	// Allow SharedData to construct me
	friend class SharedData;

public:
	bool ReadRT( std::string& data );
	void WriteRT( std::string data );
	bool Read( std::string& data );
    void Write( std::string data );
	void Observer( const ObserverState& );
    void Script( const ScriptState& );
	void Media( const MediaState& );
	void References( const ReferenceState& );
	void Settings( const SettingsState& );
	void Objects( const ObjectsState& );
	void Landscapes( const std::string& );
	void SkyLanguages( const std::string& );
	void SetCoveLightSystem( const std::string& );
	void CopyStaticData( void );
	void DataPush( void );
	void Disconnect( void );
	dbDatabase* const DB( void );
	boost::interprocess::offset_ptr<NshadeConf> Config( void );

private:
	SharedDataC();
	virtual ~SharedDataC();
	dbDatabase* m_memDB;
	NshadeSharedMemory* m_sharedMem;
};

class SharedDataStub : public ISharedData {
	// Allow SharedData to construct me
	friend class SharedData;

public:
	bool ReadRT( std::string& data ){/* NOOP */ return false; };
	void WriteRT( std::string data ){/* NOOP */};
	bool Read( std::string& data ){/* NOOP */ return false; };
    void Write( std::string data ){/* NOOP */};
	void Observer( const ObserverState& ){/* NOOP */};
	void Script( const ScriptState& ){/* NOOP */};
	void Media( const MediaState& ){/* NOOP */};
	void References( const ReferenceState& ){/* NOOP */};
	void Settings( const SettingsState& ){/* NOOP */};
	void Objects( const ObjectsState& ){/* NOOP */};
	void Landscapes( const std::string& ){/* NOOP */}
	void SkyLanguages( const std::string& ){/* NOOP */};
	void CopyStaticData( void ){/* NOOP */};
	void SetCoveLightSystem( const std::string& ){/* NOOP */};
	void DataPush( void ){/* NOOP */};
	void Disconnect( void ){/* NOOP */};
	dbDatabase* const DB( void ){ return NULL; };
	boost::interprocess::offset_ptr<NshadeConf> Config( void ){/* NOOP */ return NULL; };

private:
	SharedDataStub(){};
};

class SharedData {
public:
	static ISharedData* const Instance( void );
	static void Destroy( bool = false );
	static void Disconnect();

private:
	static ISharedData* m_instance;

	// Used to prevent anyone from obtaining an instance during an update
	static bool m_preventInstance;
};

#endif


