/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2007 Digitalis Education Solutions, Inc.
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

// manage an external viewer

#include <iostream>
#include "external_viewer.h"
#include "app_settings.h"
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

bool ExternalViewer::is_playing = false;
char ExternalViewer::m_last[101] = {0};

ExternalViewer::ExternalViewer(std::string filename, std::string data_dir,  std::string coordinate_system)
{
	bool dome = (coordinate_system == "dome");

	send_IPC("r 0 0 0 0 0 0");
	send_IPC("c 0");

	char comm[101];
	sprintf( comm, "p %d", dome ? 1 : 0 );
	send_IPC(comm);

	controlScript = data_dir + "script_external_viewer ";

	// TODO check for load error
	string action = controlScript + "start \"" + filename + "\" &";
	
	// Change forward slash to backslash for windows system call
	if( AppSettings::Instance()->Windows() )
		for( size_t pos = action.find("/"); pos != string::npos; pos = action.find("/") )
			action.replace( pos, 1, "\\" );

//    std::cout << action << "\n";
	system(action.c_str());
	is_playing = 1;
	MediaState state;
	state.playStateVideo = MediaState::PLAYING;
	SharedData::Instance()->Media( state );

	if(!dome)
		set_scale(1.0, 0);
}

// For consistency with image interface
ExternalViewer::ExternalViewer(std::string filename, std::string data_dir, int coordinate_system)
{
	ExternalViewer( filename, data_dir, (coordinate_system == 0) ? "viewport" : "dome" );
}

ExternalViewer::~ExternalViewer()
{
	stop();
	send_IPC("r 0 0 0 0 0 0");
}

void ExternalViewer::resume()
{
	std::string action = controlScript + "resume &";
	system(action.c_str());
	is_playing=1;
	MediaState state;
	state.playStateVideo = MediaState::PLAYING;
	SharedData::Instance()->Media( state );
}

void ExternalViewer::pause()
{
	std::string action = controlScript + "pause &";
	system(action.c_str());
	is_playing=0;
	MediaState state;
	state.playStateVideo = MediaState::PAUSED;
	SharedData::Instance()->Media( state );
}

void ExternalViewer::stop()
{
	std::string action = controlScript + "stop &";
	system(action.c_str());
	is_playing=0;
	MediaState state;
	state.playStateVideo = MediaState::STOPPED;
	SharedData::Instance()->Media( state );
}


void ExternalViewer::set_alpha(float alpha, float duration)
{
	char comm[101];
	sprintf( comm, "r %5.2f %5.2f %5.2f %5.2f %5.2f %5.5f", NO_VALUE,  NO_VALUE,  NO_VALUE,  NO_VALUE,  alpha,  duration*1000.f);
	send_IPC(comm);
}

void ExternalViewer::set_scale(float scale, float duration)
{
	char comm[101];
	sprintf( comm, "r %5.2f %5.2f %5.2f %5.2f %5.2f %5.5f", NO_VALUE,  NO_VALUE,  scale,  NO_VALUE,  NO_VALUE,  duration*1000.f);
	send_IPC(comm);
}

void ExternalViewer::set_rotation(float rotation, float duration, bool shortPath)
{
	char comm[101];
	sprintf( comm, "r %5.2f %5.2f %5.2f %5.2f %5.2f %5.5f", NO_VALUE,  NO_VALUE,  NO_VALUE,  -rotation,  NO_VALUE,  duration*1000.f);
	send_IPC(comm);
}

void ExternalViewer::set_viewport(string coordinate_system) {
	bool dome = (coordinate_system == "dome");
	char comm[101];
	sprintf( comm, "p %d", dome ? 1 : 0 );
	send_IPC(comm);
}

void ExternalViewer::set_location(float xpos, bool deltax, float ypos, bool deltay, float duration)
{
	char comm[101];
	float x = deltax ? xpos : NO_VALUE;
	float y = deltay ? -ypos : NO_VALUE;

	sprintf( comm, "r %5.2f %5.2f %5.2f %5.2f %5.2f %5.5f", x,  y,  NO_VALUE,  NO_VALUE,  NO_VALUE,  duration*1000.f);
	send_IPC(comm);
}

void ExternalViewer::set_clone(int clone)
{
	char comm[101];
	sprintf( comm, "c %d", clone ? 1 : 0 );
	send_IPC(comm);
}


bool ExternalViewer::send_IPC(const char *command)
{
#ifdef EXTERNAL_VIEWER_IPC
	int ipc = open( EXTERNAL_VIEWER_IPC, O_WRONLY | O_NONBLOCK );
	if (ipc!=-1) {
		char output_str[101];
		sprintf(output_str, "%-99s\n", command);
		write(ipc, output_str, 100);
		close(ipc);
		memcpy( m_last, command, sizeof(m_last) );
		return true;
	}
#endif

	return false;
}

void ExternalViewer::send_last(void) {
	if(is_playing)
		send_IPC(m_last);
}

