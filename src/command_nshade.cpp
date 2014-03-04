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

#include "command_nshade.h"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#define foreach BOOST_FOREACH

using namespace boost;
using namespace std;

NshadeCommander::NshadeCommander() : m_imageMgr(ImageMgr::getImageMgr("media")) {
	m_commandMap["load_image"]       = new class LoadImage( bind(&ImageMgr::load_image, &m_imageMgr, _1, _2, _3, _4) );

	m_commandMap["drop_image"]       = new DropImage( bind(&ImageMgr::drop_image, &m_imageMgr, _1) );

	m_commandMap["drop_all_image"]   = new DropAllImage( bind(&ImageMgr::drop_all_images, &m_imageMgr) );

	m_commandMap["translate_image"]  = new TranslateImage( bind(&Image::set_location,
															   bind(&ImageMgr::get_image, &m_imageMgr, _1),
															   _2, _3, _4, _5, _6) );

	m_commandMap["rotate_image"]     = new RotateImage( bind(&Image::set_rotation,
															bind(&ImageMgr::get_image, &m_imageMgr, _1),
															_2, _3, _4) );

	m_commandMap["scale_image"]      = new ScaleImage( bind(&Image::set_scale,
														   bind(&ImageMgr::get_image, &m_imageMgr, _1),
														   _2, _3) );

	m_commandMap["alpha_image"]      = new AlphaImage( bind(&Image::set_alpha,
														   bind(&ImageMgr::get_image, &m_imageMgr, _1),
														   _2, _3) );
}

NshadeCommander::~NshadeCommander() {
	std::pair<string, NshadeCommandA*> p;
	foreach( p, m_commandMap ) {
		delete p.second;
	}
	m_commandMap.clear();
}

int NshadeCommander::execute_command( string cmd ) {
	string command;
	stringHash_t args;

	// printf( "Command: %s\n", cmd.c_str() );

	if( !parse_command(cmd, command, args) )
		return 0;
	else {
		try {
			map<std::string, NshadeCommandA*>::iterator itr =  m_commandMap.find(command);
			if( itr == m_commandMap.end() )
				return 0;
			else
				return (*itr->second)(args);
		}
		catch( ... ) {
			return 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Commands ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int LoadImage::operator ()( stringHash_t args ) {
	return m_command( args["filename"],
						args["name"],
						static_cast<Image::IMAGE_POSITIONING>(lexical_cast<int>(args["position_type"])),
						true );
}

int DropImage::operator ()( stringHash_t args ) {
	return m_command( args["name"] );
}

int DropAllImage::operator ()( stringHash_t args ) {
	return m_command();
}

int TranslateImage::operator ()( stringHash_t args ) {
	m_command( args["name"],
						lexical_cast<float>(args["xpos"]),
						true,
						lexical_cast<float>(args["ypos"]),
						true,
						lexical_cast<float>(args["duration"]) );
	return 1;
}

int RotateImage::operator ()( stringHash_t args ) {
	m_command( args["name"], lexical_cast<float>(args["rotation"]), lexical_cast<float>(args["duration"]), true );
	return 1;
}

int ScaleImage::operator ()( stringHash_t args ) {
	m_command( args["name"], lexical_cast<float>(args["scale"]), lexical_cast<float>(args["duration"]) );
	return 1;
}

int AlphaImage::operator ()( stringHash_t args ) {
	m_command( args["name"], lexical_cast<float>(args["alpha"]), lexical_cast<float>(args["duration"]) );
	return 1;
}
