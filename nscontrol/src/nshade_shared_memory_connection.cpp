/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Author: Trystan A. Larey-Williams
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

#include "nshade_shared_memory_connection.h"

using namespace boost::interprocess;
using namespace boost::uuids;
using namespace std;

NshadeSharedMemoryConnection::NshadeSharedMemoryConnection( managed_shared_memory* const shm ) {
	const QueueAlloc allocator( shm->get_segment_manager() );
	m_cmdQueue = shm->construct<CmdQueue>(anonymous_instance)(allocator);
}

NshadeSharedMemoryConnection::~NshadeSharedMemoryConnection() {
	m_cmdQueue->clear();
}

void NshadeSharedMemoryConnection::SetClientType( NshadeSharedMemoryConnection::Type type ) {
	m_clientType = type;
}

NshadeSharedMemoryConnection::Type NshadeSharedMemoryConnection::ClientType( void ) {
	return m_clientType;
}

void NshadeSharedMemoryConnection::SetOwner( const uuid owner ) {
	m_owner = owner;
}

const uuid NshadeSharedMemoryConnection::Owner( void ) {
	return m_owner;
}

bool NshadeSharedMemoryConnection::Empty( void ) {
	return m_cmdQueue->empty();
}

void NshadeSharedMemoryConnection::Queue( NshadeCommand& cmd ) {
	if(m_cmdQueue->size() > 99) // Limit size of queue
		m_cmdQueue->pop_front();

	m_cmdQueue->push_back( cmd );
}

NshadeCommand NshadeSharedMemoryConnection::DeQueue( void ) {
	NshadeCommand cmd;

	if( !m_cmdQueue->empty() ) {
		cmd = m_cmdQueue->front();
		m_cmdQueue->pop_front();
	}

	return cmd;
}
