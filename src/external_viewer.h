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

#ifndef _EXTERNAL_VIEWER_H_
#define _EXTERNAL_VIEWER_H_

#include <config.h>
#include <string>
#include "shared_data.h"
#include "nightshade.h"

#ifdef OP3

#include <unistd.h>
#include <fcntl.h>
#define EXTERNAL_VIEWER_IPC "/tmp/THOLOS"
#define LIRCM_IPC "/var/tmp/LIRCM-CONTROL"
#endif

#define NO_VALUE -9999.f


class ExternalViewer
{
public:
	ExternalViewer(){};
	ExternalViewer(std::string filename, std::string data_dir, std::string coordinate_system);
	ExternalViewer(std::string filename, std::string data_dir, int coordinate_system);
	virtual ~ExternalViewer();
	void resume();
	void pause();
	void stop();
	void set_alpha(float alpha, float duration);
	void set_scale(float scale, float duration);
	void set_rotation(float rotation, float duration, bool = false);
	void set_location(float xpos, bool deltax, float ypos, bool deltay, float duration);
	void set_clone(int clone);
	void set_viewport(std::string);
	void send_last(void);

private:
	bool send_IPC(const char *command );
	std::string controlScript;

	// Accessed from signal handler
	static bool is_playing;
	static char m_last[101];
};

#endif // _EXTERNAL_VIEWER_H
