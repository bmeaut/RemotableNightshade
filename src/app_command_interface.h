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

/* This class handles parsing of a simple command syntax for scripting,
   UI components, network commands, etc.
*/

#ifndef _APP_COMMAND_INTERFACE_H_
#define _APP_COMMAND_INTERFACE_H_

#include "core.h"
#include "command_interface.h"
#include "app.h"
#include "utility.h"
#include "script_mgr.h"
#include "audio.h"
#include "external_viewer.h"
using namespace std;

// Predeclaration of the Core class
class Core;
class App;

class AppCommandInterface : CommandInterface
{

public:
	AppCommandInterface(Core * core, App * app);
	virtual ~AppCommandInterface();
	virtual int execute_command(string commandline);
	virtual int execute_command(string command, double arg);
	virtual int execute_command(string command, int arg);
	virtual int execute_command(string command, unsigned long int &wait, bool trusted);
	virtual int set_flag(string name, string value, bool &newval, bool trusted);
	void update(int delta_time);
	void enableAudio();
	void disableAudio();
	string getErrorString();

private:
	Core * stcore;
	App * stapp;
	Audio * audio;  // for audio track from script
	ExternalViewer * ExtViewer; // - external media viewer for scripting
	bool audioDisabled; // - whether audio is disabled temporarily or not, master
	string debug_message;  // for 'execute_command' error details
};


#endif // _APP_COMMAND_INTERFACE_H
