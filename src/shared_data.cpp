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

#include "shared_data.h"
#include "app_settings.h"

ISharedData* SharedData::m_instance = NULL;
bool SharedData::m_preventInstance = false;

ISharedData* const SharedData::Instance( void ){
	if( !m_instance  ) {
		InitParser conf;
		AppSettings::Instance()->loadAppSettings( &conf );

		if( conf.get_boolean("main","nscontrol", true) && !m_preventInstance ) {
			try {
				m_instance = new SharedDataC;
			}
			catch( ... ) {
				printf( "Warning: Failed to initialize 'nscontrol' interface or in-memory database.\n" );
				m_instance = new SharedDataStub;
			}
		}
		else
			m_instance = new SharedDataStub;
	}

	return m_instance;
}

void SharedData::Destroy( bool preventInstance ) {
	m_preventInstance = preventInstance;
	delete m_instance;
	m_instance = NULL;
}

void SharedData::Disconnect( void ) {
	if( m_instance )
		m_instance->Disconnect();
}


SharedDataC::SharedDataC( void ) {
	m_sharedMem = new NshadeSharedMemory();
	m_sharedMem->SetClientType( NshadeSharedMemoryConnection::nightshade );
	m_sharedMem->Connect();
	m_sharedMem->Config()->SetRootDir( AppSettings::Instance()->getDataRoot() );

	m_memDB = new dbDatabase(dbDatabase::dbConcurrentUpdate, 32000000);
	if( AppSettings::Instance()->Unix() ) {
		if( !m_memDB->open("NShadeDB", "/dev/shm/NShadeDB.fdb") )
			throw std::exception();
	}
	else {
		if( !m_memDB->open("NShadeDB") )
			throw std::exception();
	}

	m_memDB->commit();
}

SharedDataC::~SharedDataC() {
	if( m_memDB ) {
		m_memDB->close();
		delete m_memDB;
		m_memDB = NULL;
	}
	if( m_sharedMem ) {
		m_sharedMem->Disconnect();
		delete m_sharedMem;
		m_sharedMem = NULL;
	}
}

void SharedDataC::Disconnect( void ) {
	if( m_sharedMem ) {
		m_sharedMem->Disconnect();
		delete m_sharedMem;
		m_sharedMem = NULL;
	}
}

dbDatabase* const SharedDataC::DB( void ) {
	return m_memDB;
}

bool SharedDataC::ReadRT( std::string& data ){
	if( m_sharedMem )
		return m_sharedMem->ReadRT( data );
	else
		return false;
}

void SharedDataC::WriteRT( std::string data ){
	if( m_sharedMem )
		m_sharedMem->WriteRT( data );
}

bool SharedDataC::Read( std::string& data ) {
	if( m_sharedMem )
		return m_sharedMem->Read( data );
	else
		return false;
}

void SharedDataC::Write( std::string data ) {
	m_sharedMem->Write( data );
}

boost::interprocess::offset_ptr<NshadeConf> SharedDataC::Config( void ) {
	if( m_sharedMem )
		return m_sharedMem->Config();
	else
		return NULL;
}

void SharedDataC::CopyStaticData( void ) {
	if( m_sharedMem )
		m_sharedMem->m_readState->CopyStaticData( *m_sharedMem->m_writeState );
}

void SharedDataC::SetCoveLightSystem( const std::string& str ) {
	m_sharedMem->Config()->SetCoveLightSystem( str );
}

void SharedDataC::DataPush( void ) {
	if( m_sharedMem )
		*(m_sharedMem->m_readState) = *(m_sharedMem->m_writeState);
}

void SharedDataC::Observer( const ObserverState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Observer( obj );
}

void SharedDataC::Media( const MediaState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Media( obj );
}

void SharedDataC::Script( const ScriptState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Script( obj );
}

void SharedDataC::References( const ReferenceState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Reference( obj );
}

void SharedDataC::Settings( const SettingsState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Settings( obj );
}

void SharedDataC::Objects( const ObjectsState& obj ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Objects( obj );
}

void SharedDataC::Landscapes( const string& data ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->Landscapes( data );
}

void SharedDataC::SkyLanguages( const string& data ) {
	if( m_sharedMem )
		m_sharedMem->m_writeState->SkyLanguages( data );
}

