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

#ifndef _COMMAND_NSHADE_H_
#define _COMMAND_NSHADE_H_

#include "command_interface.h"
#include <map>
#include <boost/function.hpp>
#include <string>
#include "image_mgr.h"

///////////////////////////////////////////////////////////////////////////////
// Base for all commands //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class NshadeCommandA
{
public:
	virtual ~NshadeCommandA(){};
	virtual int operator()( stringHash_t ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Image Commands /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class LoadImage : public NshadeCommandA
{
public:
	virtual ~LoadImage(){};
	LoadImage( boost::function<int(std::string, std::string, Image::IMAGE_POSITIONING, bool)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<int(std::string, std::string, Image::IMAGE_POSITIONING, bool)> m_command;
};

class DropImage : public NshadeCommandA
{
public:
	virtual ~DropImage(){};
	DropImage( boost::function<int(std::string)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<int(std::string)> m_command;
};

class DropAllImage : public NshadeCommandA
{
public:
	virtual ~DropAllImage(){};
	DropAllImage( boost::function<int(void)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<int(void)> m_command;
};

class TranslateImage : public NshadeCommandA
{
public:
	virtual ~TranslateImage(){};
	TranslateImage( boost::function<void(std::string, float, bool, float, bool, float)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<void(std::string, float, bool, float, bool, float)> m_command;
};

class RotateImage : public NshadeCommandA
{
public:
	virtual ~RotateImage(){};
	RotateImage( boost::function<void(std::string, float, float, bool)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<void(std::string, float, float, bool)> m_command;
};

class ScaleImage : public NshadeCommandA
{
public:
	virtual ~ScaleImage(){};
	ScaleImage( boost::function<void(std::string, float, float)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<void(std::string, float, float)> m_command;
};

class AlphaImage : public NshadeCommandA
{
public:
	virtual ~AlphaImage(){};
	AlphaImage( boost::function<void(std::string, float, float)> cmd ) {
		m_command = cmd;
	}
	int operator()( stringHash_t );
private:
    boost::function<void(std::string, float, float)> m_command;
};


///////////////////////////////////////////////////////////////////////////////
// Command Manager ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class NshadeCommander : public CommandInterface
{
public:
	NshadeCommander();
	virtual ~NshadeCommander();
	int execute_command( std::string );

private:
	ImageMgr& m_imageMgr;
	std::map<std::string, NshadeCommandA*> m_commandMap;
};

#endif
