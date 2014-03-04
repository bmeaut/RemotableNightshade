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

#include "script.h"


Script::Script()
{
	input_file = NULL;
	path = "";
}

Script::~Script()
{
	if (input_file)
		delete input_file;  // closes automatically
}

int Script::load(string script_file, string script_path)
{
	input_file = new ifstream(script_file.c_str());

	if (! input_file->is_open()) {
		cout << "Unable to open script " << script_file << endl;
		return 0;
	}
	path = script_path;

	// TODO check first line of file for script identifier... ?

	return 1;
}

int Script::next_command(string &command)
{

	string line;

	while (! input_file->eof() ) {
		getline(*input_file,line);

		if ( line[0] != '#' && line[0] != 0 && line[0] != '\r') {

//			cout << "Line is: " << line << endl;

			command = line;

			return 1;
		}
	}

	return 0;
}


