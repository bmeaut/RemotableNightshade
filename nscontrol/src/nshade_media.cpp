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

#include "nshade_media.h"

using namespace boost::interprocess;

NshadeConf::NshadeConf(boost::interprocess::managed_shared_memory* shm) :
		m_rootDir(CharAlloc(shm->get_segment_manager())),
		m_searchTypes(CharAlloc(shm->get_segment_manager())),
		m_coveLightSystem(CharAlloc(shm->get_segment_manager())){
}

void NshadeConf::SetRootDir( std::string clientDir ) {
	m_rootDir = clientDir.c_str();
}

std::string NshadeConf::GetRootDir( void ) {
	return m_rootDir.c_str();
}

void NshadeConf::SetSearchTypes( std::string searchTypes ) {
	m_searchTypes = searchTypes.c_str();
}

std::string NshadeConf::GetSearchTypes( void ) {
	return m_searchTypes.c_str();
}

void NshadeConf::SetCoveLightSystem( std::string searchTypes ) {
	m_coveLightSystem = searchTypes.c_str();
}

std::string NshadeConf::GetCoveLightSystem( void ) {
	return m_coveLightSystem.c_str();
}

