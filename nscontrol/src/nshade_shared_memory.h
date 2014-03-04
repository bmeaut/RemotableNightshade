/*
 * Copyright (C) 2010 Digitalis Education Solutions, Inc.
 * Author: Trystan Larey-Williams
 * Date: 2-22-2010
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
 * Helper class to manage named shared memory resources wraps read and write
 * calls in mutex for IPC safety
 */

#pragma once

#include "nshade_shared_memory_connection.h"
#include "nshade_state.h"
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/pair.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>


// A single instance of this object is loaded into shared memory to track the number of active
// SharedMemory objects. When an instance of SharedMemory is destroyed it will check the count.
// If zero, the shared memory segment will be cleaned up and all shared data removed.
struct ReferenceCounter {
	ReferenceCounter() { m_ref = 0; };
	unsigned long m_ref;
};

class NshadeSharedMemory {
public:
	NshadeSharedMemory();
	virtual ~NshadeSharedMemory();

	void Connect( void );
	void Disconnect( void );
	bool ReadRT( std::string& data );
	void WriteRT( std::string data );
	bool Read( std::string& data );
	void Write( std::string data );
	boost::interprocess::offset_ptr<NshadeConf> Config( void );
	void SetClientType( NshadeSharedMemoryConnection::Type );

	// For nightshade suspend checking
	void Lock( void );
	void UnLock( void );
	bool TryLock( void );
	// end nightshade suspend checking

	boost::interprocess::offset_ptr<NshadeWriteState> m_writeState;
	boost::interprocess::offset_ptr<NshadeReadState> m_readState;

private:
	static const unsigned int RTQSIZE = 100;

	// Type definitions for IPC friendly linked-list container of connections to shared memory]
	typedef boost::interprocess::pair<boost::uuids::uuid,
			boost::interprocess::offset_ptr<NshadeSharedMemoryConnection> > ConnectionPair;

	typedef boost::interprocess::allocator<ConnectionPair,
			boost::interprocess::managed_shared_memory::segment_manager> ShmAlloc;

	typedef boost::interprocess::map<boost::uuids::uuid,
			boost::interprocess::offset_ptr<NshadeSharedMemoryConnection>, std::less<boost::uuids::uuid>, ShmAlloc> ShmMap;

	// Unique (for each process) pointer to shared memory
	boost::interprocess::managed_shared_memory* m_shm;

	// Pointers to shared objects within shared memory
	boost::interprocess::offset_ptr<NshadeConf> m_nsConf;
	boost::interprocess::offset_ptr<ReferenceCounter> m_refCount;
	boost::interprocess::offset_ptr<ShmMap> m_connections;
    boost::interprocess::offset_ptr<RTQPtr> m_writePtr;
	boost::interprocess::offset_ptr<RTQPtr> m_readPtr;
	boost::interprocess::offset_ptr<RTQGap> m_RTQGap;
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_mutex;
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_readerMutex;
	boost::interprocess::offset_ptr<boost::interprocess::interprocess_mutex> m_writerMutex;

	NshadeSharedMemoryConnection::Type m_clientType;
	boost::uuids::uuid m_clsid;
};
