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

#ifndef NSCONF_H_
#define NSCONF_H_

#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <string>

class NshadeConf {
public:
	NshadeConf(boost::interprocess::managed_shared_memory*);
	void SetRootDir( std::string );
	std::string GetRootDir( void );
	void SetSearchTypes( std::string );
	std::string GetSearchTypes( void );
	void SetCoveLightSystem( std::string );
	std::string GetCoveLightSystem( void );

private :
	typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> CharAlloc;
	typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAlloc> shared_string;

	shared_string m_rootDir;
	shared_string m_searchTypes;
	shared_string m_coveLightSystem;
};

#endif


