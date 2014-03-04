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

#ifndef SHAREDMEMORYCONNECTION_H_
#define SHAREDMEMORYCONNECTION_H_

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <iostream>
#include "nshade_media.h"

class NshadeSharedMemoryConnection;

struct NshadeCommand {
	unsigned int length;
	static const unsigned short szBuf = 1024;
	char buf[szBuf];
};

 struct RTQueueNode {
	boost::interprocess::offset_ptr<RTQueueNode> next;
	NshadeCommand msg;
};

typedef unsigned int RTQGap;
typedef boost::interprocess::offset_ptr<RTQueueNode> RTQPtr;


class NshadeSharedMemoryConnection {
public:
	NshadeSharedMemoryConnection( boost::interprocess::managed_shared_memory* );
	~NshadeSharedMemoryConnection();

	enum Type {
		nightshade,
		fcgi,
		user
	};

	void Queue( NshadeCommand& );
	NshadeCommand DeQueue( void );
	bool Empty( void );
	void SetOwner( const boost::uuids::uuid );
	const boost::uuids::uuid Owner( void );
	void SetClientType( Type );
	Type ClientType( void );

private:
	typedef boost::interprocess::allocator<NshadeCommand,
		boost::interprocess::managed_shared_memory::segment_manager> QueueAlloc;
	typedef boost::interprocess::deque<NshadeCommand, QueueAlloc> CmdQueue;

	boost::interprocess::offset_ptr<CmdQueue> m_cmdQueue;
	boost::uuids::uuid m_owner;
	Type m_clientType;
};

#endif /* SHAREDMEMORYCONNECTION_H_ */
