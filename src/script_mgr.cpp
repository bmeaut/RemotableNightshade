/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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
#define BOOST_FILESYSTEM_VERSION 2
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/regex.hpp"

#include <iostream>
#include <string>
#include <dirent.h>
#include <cstdio>
#include <set>
#include "shared_data.h"
#include "script_mgr.h"
#include "utility.h"


ScriptMgr::ScriptMgr(AppCommandInterface *command_interface, string _data_dir) : play_paused(false)
{
	commander = command_interface;
	DataDir = _data_dir;
	recording = 0;
	playing = 0;
	record_elapsed_time = 0;
	elapsed_playback_seconds = 0;
	m_incCount = 0;
	lastScript = "";
	lastScriptPath = "";

	// used when scripts are on a CD that needs to be mounted manually
#if defined DESKTOP || defined LSS
	RemoveableScriptDirectory = "";
	RemoveableScriptDirectory2 = "";
#else
	RemoveableScriptDirectory = DataDir + "scripts/" + SCRIPT_REMOVEABLE_DISK + "/";
	RemoveableScriptDirectory2 = DataDir + "scripts/" + SCRIPT_USB_DISK + "/";
#endif
	RemoveableDirectoryMounted = 0;
}

ScriptMgr::~ScriptMgr()
{
}

// path is used for loading script assets
bool ScriptMgr::play_script(string script_file, string script_path)
{
	// load script...

	if (playing) {
		// cancel current script and start next (one script can call another)
		cancel_script();
	}

	set_gui_debug(0);  // Default off until script sets otherwise

	//if (script) delete script;
	script = new Script();

	// if script is on mountable disk, mount that now
	mount_directory( script_file );

	if ( script->load(script_file, script_path) ) {
		m_incCount = 0;
		commander->execute_command("multiplier rate 1");
		playing = 1;

		ScriptState state;
		state.playState = ScriptState::PLAYING;
		SharedData::Instance()->Script( state );

		play_paused = 0;
		elapsed_time = wait_time = 0;
		elapsed_playback_seconds = 0;

		lastScript = script_file;
		lastScriptPath = script_path;

#ifndef DESKTOP
		// touch file to show script is running to external processes
		ofstream flagFile;
		flagFile.open(SCRIPT_IS_PLAYING_FILE);
		flagFile.close();
#endif
		return 1;

	} else {
		cancel_script();
		return 0;
	}
}

void ScriptMgr::cancel_script()
{
	// delete script object...
	if (script) delete script;
	script = NULL;
	// images loaded are deleted from stel_command_interface directly
	playing = 0;
	play_paused = 0;

	ScriptState state;
	state.playState = ScriptState::STOPPED;
	SharedData::Instance()->Script( state );

	// Unmount any removeable media used
	unmount_directories();

#ifndef DESKTOP
	remove(SCRIPT_IS_PLAYING_FILE);
#endif

}


void ScriptMgr::pause_script()
{
	if(!playing)
		return;

	play_paused = 1;

	ScriptState state;
	state.playState = ScriptState::PAUSED;
	SharedData::Instance()->Script( state );

	// need to pause time as well
	commander->execute_command("timerate action pause");
}

void ScriptMgr::resume_script()
{
	if(!playing)
		return;

	play_paused = 0;
	commander->execute_command("timerate action resume");
	commander->execute_command("multiplier rate 1");
	m_incCount = 0;
	ScriptState state;
	state.playState = ScriptState::PLAYING;
	SharedData::Instance()->Script( state );
}

bool ScriptMgr::is_faster() {
	return (m_incCount > 0);
}

void ScriptMgr::faster_script() {
	if( !playing || play_paused )
		return;

	if( ++m_incCount < 3 ) {
		commander->execute_command("multiplier action increment step 2");
		ScriptState state;
		state.playState = ScriptState::FF;
		SharedData::Instance()->Script( state );
	}
	else
		--m_incCount;
}

void ScriptMgr::slower_script() {
	if( !playing || play_paused )
		return;

	if( --m_incCount > -1 )
		commander->execute_command("multiplier action decrement step 2");
	else {
		++m_incCount;
		ScriptState state;
		state.playState = ScriptState::PLAYING;
		SharedData::Instance()->Script( state );
	}
}

void ScriptMgr::record_script(string script_filename)
{

	// TODO: filename should be selected in a UI window, but until then this works
	if (recording) {
		cout << "Already recording script." << endl;
		return;
	}

	if (script_filename != "") {
		rec_file.open(script_filename.c_str(), fstream::out);
	} else {

		string sdir;
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__)
		if (getenv("USERPROFILE")!=NULL) {
			//for Win XP etc.
			sdir = string(getenv("USERPROFILE")) + "\\My Documents\\";
		} else {
			//for Win 98 etc.
			sdir = "C:\\My Documents\\";
		}
#else
		sdir = string(getenv("HOME")) + "/";
#endif
#ifdef MACOSX
		sdir += "/Desktop/";
#endif

		// add a number to be unique
		char c[3];
		FILE * fp;
		for (int j=0; j<=100; ++j) {
			snprintf(c,3,"%d",j);

			script_filename = sdir + APP_LOWER_NAME + c + ".sts";
			fp = fopen(script_filename.c_str(), "r");
			if (fp == NULL)
				break;
			else
				fclose(fp);
		}

		rec_file.open(script_filename.c_str(), fstream::out);

	}


	if (rec_file.is_open()) {
		recording = 1;
		record_elapsed_time = 0;
		cout << "Now recording actions to file: " << script_filename << endl;
		rec_filename = script_filename;
	} else {
		cout << "Error opening script file for writing: " << script_filename << endl;
		rec_filename = "";
	}
}

void ScriptMgr::record_command(string commandline)
{

	if (recording) {
		// write to file...

		if (record_elapsed_time) {
			rec_file << "wait duration " << record_elapsed_time/1000.f << endl;
			record_elapsed_time = 0;
		}

		rec_file << commandline << endl;

		// For debugging
		cout << commandline << endl;
	}
}

void ScriptMgr::cancel_record_script()
{
	// close file...
	rec_file.close();
	recording = 0;

	cout << "Script recording stopped." << endl;
}

// Allow timer to be reset
void ScriptMgr::reset_timer()
{
	elapsed_time = 0;
}


// runs maximum of one command per update
// note that waits can drift by up to 1/fps seconds
void ScriptMgr::update(int delta_time)
{

	if (recording) record_elapsed_time += delta_time;

	if (playing && !play_paused) {

		elapsed_time += delta_time;  // time elapsed since last command (should have been) executed

		elapsed_playback_seconds += double(delta_time)/1000;

		if (elapsed_time >= wait_time) {
			// now time to run next command


			//      cout << "dt " << delta_time << " et: " << elapsed_time << endl;
			elapsed_time -= wait_time;
			string comd;

			unsigned long int wait;
			if (script->next_command(comd)) {
				commander->execute_command(comd, wait, 0);  // untrusted commands
				wait_time = wait;

			} else {
				// script done
				// cout << "Script completed." << endl;
				commander->execute_command("script action end");
			}

			/* Still bugs to work out 200703

			// * * * DIGITALIS 20070123 Perform all commands until next wait command
			unsigned long int wait=0;
			while(wait == 0 && script && script->next_command(comd)) {
			commander->execute_command(comd, wait, 0);  // untrusted commands
			// cout << comd << endl;
			wait_time = wait;
			}

			if(wait == 0) {
			// script done
			// cout << "Script completed." << endl;
			commander->execute_command("script action end");
			}
			// * * *
			*/

		}
	}

}

// get a list of script files from directory
// in alphabetical order
string ScriptMgr::get_script_list(string directory)
{

	// TODO: This is POSIX specific

	multiset<string> items; 
	multiset<string>::iterator iter; 

	struct dirent *entryp;
	DIR *dp;
	string tmp;

	//  cout << "RemoveableScriptDirectory = " << RemoveableScriptDirectory << endl;

	mount_directory(directory);

	if ((dp = opendir(directory.c_str())) != NULL) {

		// TODO: sort the directory
		while ((entryp = readdir(dp)) != NULL) {
			tmp = entryp->d_name;

			if (tmp.length()>4 && tmp.find(".sts", tmp.length()-4)!=string::npos ) {
				items.insert(tmp + "\n");
				//cout << entryp->d_name << endl;
			}
		}
		closedir(dp);
	} else {
		cout << "Unable to read script directory" << directory << endl;
	}

	unmount_directories();

	string result="";
	for (iter=items.begin(); iter!=items.end(); iter++ ) {
		result += (*iter);
	}

	return result;

}

string ScriptMgr::get_script_path()
{
	if (script) return script->get_path();
	return string(DataDir) + "scripts/";
}


// look for a script called "startup.sts"
bool ScriptMgr::play_startup_script()
{
	// first try on removeable directory
	if (RemoveableScriptDirectory2 !="" &&
	        play_script(RemoveableScriptDirectory2 + "scripts/startup.sts",
	                    RemoveableScriptDirectory2 + "scripts/")) {
		return 1;

	} else if (RemoveableScriptDirectory !="" &&
	           play_script(RemoveableScriptDirectory + "scripts/startup.sts",
	                       RemoveableScriptDirectory + "scripts/")) {
		return 1;
	} else {

		// try in application tree
		return play_script(DataDir + "scripts/startup.sts", DataDir + "scripts/");
	}
}

// look for a script called "demo.sts"
bool ScriptMgr::play_demo_script()
{

	return play_script(DataDir + "scripts/demo.sts", DataDir + "scripts/");

}

bool ScriptMgr::replay_last_script()
{

	return play_script( lastScript, lastScriptPath );

}

// look for a script starting with digits to run
bool ScriptMgr::play_script_by_number(int number)
{
	string file;

	// Try usb, internal, local, dvd in order

	if (RemoveableScriptDirectory2 !="") {
		file = get_script_by_number(RemoveableScriptDirectory2 + "scripts/", number);

		if(file != "") return play_script(RemoveableScriptDirectory2 + "scripts/" + file,
										  RemoveableScriptDirectory2 + "scripts/");
	}

	// try internal scripts
	file = get_script_by_number(DataDir + "internal-scripts/", number);

	if(file != "" ) return play_script(DataDir + "internal-scripts/" + file, 
									   DataDir + "internal-scripts/");

	// try in local scripts
	file = get_script_by_number(DataDir + "scripts/", number);
	
	if(file != "" ) return play_script(DataDir + "scripts/" + file, DataDir + "scripts/");

	if (RemoveableScriptDirectory !="") {
		file = get_script_by_number(RemoveableScriptDirectory + "scripts/", number);
		
		if(file != "") return play_script(RemoveableScriptDirectory + "scripts/" + file,
										  RemoveableScriptDirectory + "scripts/");
	}

	return 0;
}


// return first matching script in directory
string ScriptMgr::get_script_by_number(string directory, int number)
{

	mount_directory(directory);

	try {
		boost::filesystem::path p(directory);

		string digits;
		if(number < 10) digits = string("0") + Utility::intToString(number);
		else digits = Utility::intToString(number);
		
		boost::regex e(string("^") + digits + ".*\\.sts$");
		
		boost::filesystem::directory_iterator dir_iter(p), dir_end;
		string filename;
		
		for(;dir_iter != dir_end; ++dir_iter) {
			filename=dir_iter->leaf();
			// cout<<filename<<endl;
			
			if (boost::regex_search(filename,e)) {
				//cout<<"MatchFound:"<<filename<<endl;
				return filename;
			}
		}
	}
	catch( ... ) {
		// cerr << "Could not find script by number in directory " << directory << endl;
		return "";
	}

	unmount_directories();

	return "";
}

// Determine if path is on a removeable media device
// and mount if needed
// returns true if path matched
bool ScriptMgr::mount_directory(string path)
{

	bool match = false;

	if (RemoveableScriptDirectory != "" &&
		path.compare(0,RemoveableScriptDirectory.length(), RemoveableScriptDirectory) ==0) {

		std::system( ( DataDir + "script_mount_script_cd" ).c_str() );
		//cout << "MOUNT CD to read script\n";

		match = RemoveableDirectoryMounted = 1;

	} else if (RemoveableScriptDirectory2 != "" &&
			   path.compare(0,RemoveableScriptDirectory2.length(), RemoveableScriptDirectory2) == 0 ) {

		std::system( ( DataDir + "script_mount_script_usb" ).c_str() );
		//cout << "MOUNT USB to read script\n";

		match = RemoveableDirectoryMounted = 1;
	}

	return match;

}

// Unmounts all removeable directories, if any mounted
void ScriptMgr::unmount_directories()
{
	if (RemoveableDirectoryMounted) {
		std::system( ( DataDir + "script_unmount_script_disk " ).c_str() );
		// cout << "UNMOUNT DISK\n";
		RemoveableDirectoryMounted = 0;
	}
}

// Return seconds elapsed in the current script playback
double ScriptMgr::get_script_elapsed_seconds()
{
	if (playing) return elapsed_playback_seconds; 
	else return 0;
}
