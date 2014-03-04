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

#ifndef _SCRIPT_MGR_H_
#define _SCRIPT_MGR_H_

#include <iostream>
#include <fstream>
#include "app_command_interface.h"
#include "script.h"

// flag file for other processes
#define SCRIPT_IS_PLAYING_FILE "/var/tmp/st_script_playing"

// predeclaration
class AppCommandInterface;
class Script;

class ScriptMgr
{

public:
	ScriptMgr(AppCommandInterface * command_interface, string _data_dir);
	~ScriptMgr();
	bool play_script(string script_file, string script_path);
	bool play_startup_script();
	bool play_demo_script();
	void cancel_script();  // stop playing current script
	void pause_script();
	void faster_script();
	void slower_script();
	void resume_script();  // start playing paused script
	void record_script(string script_filename);  // begin recording user interactions
	void record_command(string commandline);     // record a command (if recording)
	void cancel_record_script();  // stop recording user interactions
	bool is_playing() {
		return playing;
	};     // is a script playing?
	bool is_paused() {
		return play_paused;
	};     // is a script paused?
	bool is_recording() {
		return recording;
	};    // is a script being recorded?
	bool is_faster( void );
	void reset_timer();
	void update(int delta_time);  // execute commands in running script
	string get_script_list(string directory);  // get list of scripts in a directory
	string get_script_path();
	string get_record_filename() {
		return rec_filename;    // file record is writing to
	}
	void set_allow_ui(bool _aui) {
		allow_ui = _aui;
	}
	bool get_allow_ui() {
		return allow_ui;
	}
	void set_gui_debug(bool _gdebug) {
		gui_debug = _gdebug;    // Should script errors be shown onscreen?
	}
	bool get_gui_debug() {
		return gui_debug;
	}
	bool replay_last_script();

	bool play_script_by_number(int number);

	string get_script_by_number(string directory, int number);

	double get_script_elapsed_seconds();

private:

	bool mount_directory(string path);
	void unmount_directories();

	AppCommandInterface * commander;  // for executing script commands
	Script * script; // currently loaded script
	double elapsed_playback_seconds;  // seconds since current script playback began (in script time)
	unsigned long int elapsed_time;  // ms since last script command executed
	unsigned long int wait_time;     // ms until next script command should be executed
	unsigned long int record_elapsed_time;  // ms since last command recorded
	bool recording;  // is a script being recorded?
	bool playing;    // is a script playing?  (could be paused)
	bool play_paused;// is script playback paused?
	fstream rec_file;
	string rec_filename;
	string RemoveableScriptDirectory;

// - usb
	string RemoveableScriptDirectory2;

	bool RemoveableDirectoryMounted;
	string DataDir;
	bool allow_ui;    // Allow user interface to function during scripts
	// (except for time related keys which control script playback)
	bool gui_debug;

	string lastScript, lastScriptPath;

	int m_incCount;
};


#endif // _SCRIPT_MGR_H
