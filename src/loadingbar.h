/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2005 Fabien Chereau
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

#ifndef _LOADINGBAR_H_
#define _LOADINGBAR_H_

#include "projector.h"
#include "s_font.h"

// Class which is used to display loading bar
class LoadingBar
{
public:
	// Create and initialise
	LoadingBar(Projector* prj, float font_size, const string& font_name, const string&  splash_tex,
	           int screenw, int screenh, const string& extraTextString="", float extraTextSize = 30.f,
	           float extraTextPosx = 0.f, float extraTextPosy = 0.f);
	virtual ~LoadingBar();
	void SetMessage(string m) {
		message=m;
	}
	void Draw(float val);
private:
	string message;
	Projector* prj;
	int splashx, splashy, barx, bary, width, height, barwidth, barheight;
	s_font* barfont;
	s_texture* splash;
	string extraText;
	Vec2f extraTextPos;
	s_font* extraTextFont;
};

#endif //_LOADINGBAR_H_
