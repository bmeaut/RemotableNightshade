/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
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

// This class handles loading and playing a script of recorded commands


#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <iostream>
#include <fstream>
#include <string>
#include "app_command_interface.h"


class Script
{

public:
	Script();
	~Script();
	int load(string script_file, string script_path);         // open a script file
	int next_command(string &command);    // retreive next command to execute
	string get_path() {
		return path;
	};

private:
	ifstream * input_file;
	string path;

};


#endif // _SCRIPT_H
