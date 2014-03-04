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

#include "loadingbar.h"

LoadingBar::LoadingBar(Projector* _prj, float font_size, const string& font_name, const string&  splash_tex,
                       int screenw, int screenh, const string& extraTextString, float extraTextSize,
                       float extraTextPosx, float extraTextPosy) :
		prj(_prj), width(512), height(512), barwidth(400), barheight(10),
		extraText(VERSION)
{
	splashx = prj->getViewportPosX() + (screenw - width)/2;
	splashy = prj->getViewportPosY() + (screenh - height)/2;
	barx = prj->getViewportPosX() + (screenw - barwidth)/2;
	bary = splashy + 34;
	if(font_name != "") {
		barfont = new s_font(font_size, font_name);
		extraTextFont = new s_font(20, font_name);
	} else {
		barfont = NULL;
		extraTextFont = NULL;
	}
	if (!splash_tex.empty()) splash = new s_texture(splash_tex, TEX_LOAD_TYPE_PNG_ALPHA);
	extraTextPos.set(extraTextPosx, extraTextPosy);
}

LoadingBar::~LoadingBar()
{
	if(barfont) delete barfont;
	if(extraTextFont) delete extraTextFont;
	if (splash) delete splash;
	barfont = NULL;
}

void LoadingBar::Draw(float val)
{
	// percent complete bar only draws in 2d mode
	prj->set_orthographic_projection();

	// Draw the splash screen if available
	if (splash) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);
		glDisable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, splash->getID());

		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);		// Bottom Left
		glVertex3f(splashx, splashy, 0.0f);
		glTexCoord2i(1, 0);		// Bottom Right
		glVertex3f(splashx + width, splashy, 0.0f);
		glTexCoord2i(1, 1);		// Top Right
		glVertex3f(splashx + width, splashy + height, 0.0f);
		glTexCoord2i(0, 1);		// Top Left
		glVertex3f(splashx, splashy + height, 0.0f);
		glEnd();
	}
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	// black out background of text for redraws (so can keep sky unaltered)
	glColor3f(0, 0, 0);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(1, 0);		// Bottom Right
	glVertex3f(splashx + width, bary-70, 0.0f);
	glTexCoord2i(0, 0);		// Bottom Left
	glVertex3f(splashx, bary-70, 0.0f);
	glTexCoord2i(1, 1);		// Top Right
	glVertex3f(splashx + width, bary-5, 0.0f);
	glTexCoord2i(0, 1);		// Top Left
	glVertex3f(splashx, bary-5, 0.0f);
	glEnd();
	glColor3f(0.8, 0.8, 1);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(1, 0);		// Bottom Right
	glVertex3f(barx + barwidth, bary + barheight, 0.0f);
	glTexCoord2i(0, 0);		// Bottom Left
	glVertex3f(barx, bary + barheight, 0.0f);
	glTexCoord2i(1, 1);		// Top Right
	glVertex3f(barx + barwidth, bary, 0.0f);
	glTexCoord2i(0, 1);		// Top Left
	glVertex3f(barx, bary, 0.0f);
	glEnd();
	glColor3f(0.4f, 0.4f, 0.6f);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(1, 0);		// Bottom Right
	glVertex3f(-1 + barx + barwidth * val, bary + barheight - 1, 0.0f);
	glTexCoord2i(0, 0);		// Bottom Left
	glVertex3f(1 + barx, bary + barheight - 1, 0.0f);
	glTexCoord2i(1, 1);		// Top Right
	glVertex3f(-1 + barx + barwidth * val, bary + 1, 0.0f);
	glTexCoord2i(0, 1);		// Top Left
	glVertex3f(1 + barx, bary + 1, 0.0f);
	glEnd();

	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	if(barfont) {
		barfont->print(barx, bary-15, message);
		// LEGAL NOTICES -- THE DISPLAY OF THESE AT STARTUP MAY NOT BE REMOVED as per GPL section 5
		barfont->print(barx-20, bary-40, "<span color='#666699'>Nightshade is free, open source software with a GPLv3 license.</span>");
		barfont->print(barx-20, bary-55, "<span color='#666699'>Nightshade is a registered trademark of Digitalis Education Solutions, Inc.</span>");
	}

	if(extraTextFont) {
		extraTextFont->print(splashx + extraTextPos[0], splashy + extraTextPos[1], extraText);
		extraTextFont->print(splashx + extraTextPos[0], splashy + extraTextPos[1] - 20, EDITION); 
		extraTextFont->print(splashx + extraTextPos[0], splashy + extraTextPos[1] - 40, "Edition"); 
	}
	SDL_GL_SwapBuffers();	// And swap the buffers

	prj->reset_perspective_projection();
}
