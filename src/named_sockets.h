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

#pragma once

#include <map>
#include <string>
#include <boost/function.hpp>
#include "SDL_thread.h"

#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	#include <winsock.h>
	#define INET_ADDRSTRLEN 128
	struct ifaddrs {
		int sin_family;
		ifaddrs* ifa_next;
		char* ifa_name;
		struct sockaddr_in* ifa_addr;
	};
	int close( int fd );
	int inet_aton( char* pszAddrString, void* pAddrBuf );
	void getifaddrs( void* );
	void freeifaddrs( void* );
	void inet_ntop( int, void*, char*, int );
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <ifaddrs.h>
#endif

struct NamedSocket {
	NamedSocket() { connected = false; sd = 0; }
	int sd;
	struct sockaddr addr;
	bool connected;
};

class NamedSockets {
public:
	static NamedSockets& Instance();
	static void Clear();
	void Create( const std::string&, const std::string&, unsigned short );
	void CreateOnCurrentSubnet( const std::string&, const std::string&, unsigned int );
	void SetConnectCallback( boost::function<void(void)> cb ){ m_connectCb = cb; }

	bool Connected( const std::string& );
	void Send( const std::string&, const std::string& );
	void Destroy( const std::string& );

private:
	NamedSockets();
	~NamedSockets();
	SDL_mutex* m_lock;
	std::string m_name;
	boost::function<void(void)> m_connectCb;
	std::map<std::string, NamedSocket> m_sockets;
	static NamedSockets* m_instance;
	static int ConnectThread( void* );
};
