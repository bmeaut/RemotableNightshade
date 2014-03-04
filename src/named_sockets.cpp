/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

///////////////////////////////////////////////////////////////////////////////
/// Provides a way to share a set of sockets connections throughout the app
/// Implemented to support integration with external cove lighting systems
///////////////////////////////////////////////////////////////////////////////

#include "named_sockets.h"
#include <iostream>
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	int close( int fd ) {
		return closesocket( fd );
	}
	int inet_aton( char* pszAddrString, void* pAddrBuf ) {
		// NOOP, no direct equivalent supported under current MinGW build
		// environment. Need winsock2.
		return 0;
	}
	void getifaddrs( void* ) {
		// Not implemented
	}
	void freeifaddrs( void* ) {
		// Not implemented
	}
	void inet_ntop( int, void*, char*, int ) {
		// Not implemented
	}
#else
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

using namespace std;

NamedSockets* NamedSockets::m_instance = NULL;

NamedSockets::NamedSockets() {
	m_lock = SDL_CreateMutex();
}

NamedSockets::~NamedSockets() {
	SDL_mutexP(m_lock);
		for(map<string, NamedSocket>::const_iterator pos = m_sockets.begin(); pos != m_sockets.end(); ++pos) {
			cout << "Closing connection to " << pos->first << endl;
			close(pos->second.sd);
		}
	SDL_mutexV(m_lock);

	SDL_DestroyMutex(m_lock);
}

NamedSockets& NamedSockets::Instance() {
	if( !m_instance )
		m_instance = new NamedSockets;

	return *m_instance;
}

void NamedSockets::Clear() {
	delete m_instance;
	m_instance = NULL;
}

void NamedSockets::Create( const string& name, const string& ip, unsigned short port ) {
	SDL_mutexP(m_lock);
		m_name = name;
		if( m_sockets[name].sd )
			Destroy( name );
		m_sockets[name].sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in* ptr = reinterpret_cast<sockaddr_in*>(&m_sockets[name].addr);
		memset(ptr, 0, sizeof(sockaddr));
		ptr->sin_family = AF_INET;
		inet_aton( ip.c_str(), &(ptr->sin_addr) );
		ptr->sin_port = htons( port );
	SDL_mutexV(m_lock);

	SDL_CreateThread(&NamedSockets::ConnectThread, this);
}

void NamedSockets::CreateOnCurrentSubnet( const string& name, const string& lsB, unsigned int port ) {
	struct ifaddrs* ptr = NULL;
	getifaddrs( &ptr );
	for( struct ifaddrs* itr = ptr; itr != NULL; itr = itr->ifa_next ) {
		if( ((struct sockaddr_in *)itr->ifa_addr)->sin_family != AF_INET || string(itr->ifa_name) == "lo" )
			continue;

        char buf[INET_ADDRSTRLEN+1];
        memset( buf, 0, INET_ADDRSTRLEN+1 );
        inet_ntop(AF_INET, (void *)&((struct sockaddr_in *)itr->ifa_addr)->sin_addr, buf, INET_ADDRSTRLEN);

        string addr(buf);
        addr = addr.erase( addr.rfind( "." ) + 1 );
        addr += lsB;
        Create( name, addr, port );
	}

	if( ptr )
		freeifaddrs( ptr );
}

bool NamedSockets::Connected( const std::string& name ) {
	SDL_mutexP(m_lock);
		bool connected = m_sockets[name].connected;
	SDL_mutexV(m_lock);
	return connected;
}

int NamedSockets::ConnectThread( void* context ) {
	NamedSockets* obj = reinterpret_cast<NamedSockets*>(context);
	if( !obj )
		return( 0 );

	SDL_mutexP(obj->m_lock);
		cout << "Attempting connection to " << obj->m_name << endl;
		if( connect(obj->m_sockets[obj->m_name].sd,
					&(obj->m_sockets[obj->m_name].addr),
					sizeof(obj->m_sockets[obj->m_name].addr)) == 0 ) {
			obj->m_sockets[obj->m_name].connected = true;
			obj->m_connectCb();
			cout << "Connection to remote host succeeded" << endl;
		}
		else
			cout << "Connection to " << obj->m_name << " failed" << endl;
	SDL_mutexV(obj->m_lock);

	return( 0 );
}

void NamedSockets::Destroy( const string& name ) {
	SDL_mutexP(m_lock);
		if( m_sockets[name].sd ) {
			close( m_sockets[name].sd );
			m_sockets[name].sd = 0;
		}
	SDL_mutexV(m_lock);
}

void NamedSockets::Send( const string& name, const string& data ) {
	SDL_mutexP(m_lock);
		string local( data );
		if( m_sockets[name].sd ) {
			if( send( m_sockets[name].sd, local.c_str(), local.length(), 0) == -1 )
				cout << "Data send over socket " << name << " failed" << endl;
		}
	SDL_mutexV(m_lock);
}
