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

#include "nshade_shared_memory.h"

using namespace boost::interprocess;
using namespace std;

NshadeSharedMemory::NshadeSharedMemory() {
	m_clsid = boost::uuids::random_generator()();
	m_shm = new managed_shared_memory( open_or_create, "/nightshadeSM", 4096000 );
    
	const ShmAlloc allocator( m_shm->get_segment_manager() );
	m_connections = m_shm->find_or_construct<ShmMap>(unique_instance)(std::less<boost::uuids::uuid>(), allocator);
	m_mutex       = m_shm->find_or_construct<interprocess_mutex>("global_mutex")();
	m_readerMutex = m_shm->find_or_construct<interprocess_mutex>("reader_mutex")();
	m_writerMutex = m_shm->find_or_construct<interprocess_mutex>("writer_mutex")();
	m_writeState  = m_shm->find_or_construct<NshadeWriteState>(unique_instance)(m_shm);
	m_readState   = m_shm->find_or_construct<NshadeReadState>(unique_instance)(m_shm);
	m_refCount    = m_shm->find_or_construct<ReferenceCounter>(unique_instance)();
	m_nsConf      = m_shm->find_or_construct<NshadeConf>(unique_instance)(m_shm);
	m_readPtr     = m_shm->find_or_construct<RTQPtr>("read_ptr")();
	m_writePtr    = m_shm->find_or_construct<RTQPtr>("write_ptr")();
	m_RTQGap      = m_shm->find_or_construct<RTQGap>(unique_instance)();

	if( m_refCount->m_ref == 0 ) {
		cout << "Nightshade Shared Memory Segment Initialized.\n";

		// Initialize 'fast' circular message buffer
		*m_writePtr = m_shm->construct<RTQueueNode>(anonymous_instance)();
		offset_ptr<RTQueueNode> tmp = *m_writePtr;
		for( unsigned int i = 0; i < RTQSIZE; i++ ) {
			tmp->next = m_shm->construct<RTQueueNode>(anonymous_instance)();
			tmp = tmp->next;
		}
		*m_readPtr = tmp;	     // Initialize reader one node behind writer
		tmp->next = *m_writePtr; // Close the circle
		*m_RTQGap = 1;
	}

	(m_refCount->m_ref)++;
	m_clientType = NshadeSharedMemoryConnection::user;
}

NshadeSharedMemory::~NshadeSharedMemory() {
	if( --(m_refCount->m_ref) == 0 ) {
		// Remove any unreleased connection objects
		for( ShmMap::iterator itr = m_connections->begin(); itr != m_connections->end(); itr++ ) {
			m_shm->destroy_ptr( (itr->second).get() );
		}
		m_connections->clear();

		// Cleanup shared instances
		try {
			m_shm->destroy<interprocess_mutex>("global_mutex");
			m_shm->destroy<interprocess_mutex>("reader_mutex");
			m_shm->destroy<interprocess_mutex>("writer_mutex");
			m_shm->destroy<RTQPtr>("read_ptr");
			m_shm->destroy<RTQPtr>("write_ptr");

			// Explicitly destroying unique instances consistently causes a
			// crash on Fedora15. Since they're essentially singletons across
			// multiple processes, boost may not be expecting an explicit destroy.
			// At any rate, I couldn't find evidence of corruption in shared
			// memory as, at first, I thought this 'crash on exit' may be a sign
			// of that.
		/*	m_shm->destroy<NshadeReadState>(unique_instance);
			m_shm->destroy<NshadeWriteState>(unique_instance);
			m_shm->destroy<RTQGap>(unique_instance);
			m_shm->destroy<NshadeConf>(unique_instance);
			m_shm->destroy<ReferenceCounter>(unique_instance);
			m_shm->destroy<ShmMap>(unique_instance); */
		}
		catch( ... ) {
			printf("Warning: Not all shared memory objects could be destroyed.\n");
		}

		// Tear-down shared memory
		delete m_shm;
		shared_memory_object::remove( "nightshadeSM" );
		cout << "Nightshade Shared Memory Segment Removed.\n";
	}
	else
		delete m_shm;
}

void NshadeSharedMemory::Connect( void ) {
	m_mutex->lock();
	if( m_connections->find(m_clsid) == m_connections->end() ) {
		offset_ptr<NshadeSharedMemoryConnection> obj = m_shm->construct<NshadeSharedMemoryConnection>(anonymous_instance)( m_shm );
		obj->SetOwner( m_clsid );
		obj->SetClientType( m_clientType );
		(*m_connections)[m_clsid] = obj;
	}
	m_mutex->unlock();
}

void NshadeSharedMemory::Disconnect( void ) {
	m_mutex->lock();
	ShmMap::iterator itr = m_connections->find(m_clsid);
	if( itr != m_connections->end() ) {
		m_shm->destroy_ptr( (itr->second).get() );
		m_connections->erase( m_clsid );
	}
	m_mutex->unlock();
}

boost::interprocess::offset_ptr<NshadeConf> NshadeSharedMemory::Config( void ) {
	return m_nsConf;
}

void NshadeSharedMemory::SetClientType( NshadeSharedMemoryConnection::Type type ) {
	m_clientType = type;
}

bool NshadeSharedMemory::TryLock() {
	return m_mutex->try_lock();
}

void NshadeSharedMemory::Lock() {
	m_mutex->lock();
}

void NshadeSharedMemory::UnLock() {
	m_mutex->unlock();
}

bool NshadeSharedMemory::Read( std::string& data ) {
	bool have_data = false;
//	cerr << "Available Shared Memory: " << m_shm->get_segment_manager()->get_free_memory() << endl;

	m_mutex->lock();
	try {
		ShmMap::iterator itr = m_connections->find(m_clsid);
		if( itr !=  m_connections->end() && !itr->second->Empty() ) {
			have_data = true;
			NshadeCommand cmd = itr->second->DeQueue();
			data = std::string( cmd.buf, cmd.length );
		}
	}
	catch( std::exception& e ) {
		m_mutex->unlock();
		cerr << e.what() << endl;
		cerr << "Available Shared Memory: " << m_shm->get_segment_manager()->get_free_memory() << endl;
		throw e;
	}

	m_mutex->unlock();
	return have_data;
}

void NshadeSharedMemory::Write( std::string data ) {
	m_mutex->lock();
	try {
		NshadeCommand cmd;
		unsigned int len = data.length();
		if( len >= NshadeCommand::szBuf )
			len = NshadeCommand::szBuf - 1;

		memcpy( (void*)cmd.buf, (void*)data.c_str(), len );
		cmd.length = len;

		for( ShmMap::iterator itr = m_connections->begin(); itr != m_connections->end(); itr++ ) {
			if( itr->second->Owner() !=  m_clsid && itr->second->ClientType() != m_clientType ) {
				itr->second->Queue( cmd );
			}
		}
	}
	catch( std::exception& e ) {
		m_mutex->unlock();
		cerr << e.what() << endl;
		cerr << "Available Shared Memory: " << m_shm->get_segment_manager()->get_free_memory() << endl;
		throw e;
	}

	m_mutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Experimental performance oriented RW methods. Do not currently have much
// effect on performance in practice and more effort is needed to integrate and
// encapsulate their data structures.
///////////////////////////////////////////////////////////////////////////////

bool NshadeSharedMemory::ReadRT( std::string& data ) {
	m_readerMutex->lock();

	m_mutex->lock();
	bool empty = *m_RTQGap == 1;
	m_mutex->unlock();

	if( empty ) {
		m_readerMutex->unlock();
		return false;
	}

	*m_readPtr = (*m_readPtr)->next;
	data = std::string( (*m_readPtr)->msg.buf, (*m_readPtr)->msg.length );

	m_mutex->lock();
		--(*m_RTQGap);
	m_mutex->unlock();

	m_readerMutex->unlock();
	return true;
}

void NshadeSharedMemory::WriteRT( std::string data ) {
	m_writerMutex->lock();

	m_mutex->lock();
	bool full = *m_RTQGap == (RTQSIZE - 1);
	m_mutex->unlock();

	if( full ) {
		m_writerMutex->unlock();
		return;
	}

	unsigned int len = data.length();
	if( len >= NshadeCommand::szBuf )
		len = NshadeCommand::szBuf - 1;
	memcpy( (void*)(*m_writePtr)->msg.buf, (void*)data.c_str(), len );
	(*m_writePtr)->msg.length = len;
	*m_writePtr = (*m_writePtr)->next;

	m_mutex->lock();
		++(*m_RTQGap);
	m_mutex->unlock();

	m_writerMutex->unlock();
}

