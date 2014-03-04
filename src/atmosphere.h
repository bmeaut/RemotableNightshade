/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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

/*
	Class which compute and display the daylight sky color using openGL
	the sky is computed with the Skylight class.
*/

#ifndef _ATMOSTPHERE_H_
#define _ATMOSTPHERE_H_

#include "skylight.h"
#include "vecmath.h"
#include "navigator.h"
#include "tone_reproductor.h"
#include "skybright.h"
#include "fader.h"

using namespace std;

class Projector;

class Atmosphere
{
public:
	Atmosphere();
	virtual ~Atmosphere();
	void compute_color(double JD, Vec3d sunPos, Vec3d moonPos, float moon_phase, ToneReproductor * eye, Projector* prj,
	                   float latitude = 45.f, float altitude = 200.f,
	                   float temperature = 15.f, float relative_humidity = 40.f);
	void draw(Projector* prj, int delta_time);
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! Set fade in/out duration in seconds
	void setFadeDuration(float duration) {
		fader.set_duration((int)(duration*1000.f));
	}
	//! Get fade in/out duration in seconds
	float getFadeDuration() {
		return fader.get_duration()/1000.f;
	}

	//! Define whether to display atmosphere
	void setFlagShow(bool b);

	//! Get whether atmosphere is displayed
	bool getFlagShow() const {
		return fader;
	}

	float get_intensity(void) {
		return atm_intensity;    // tells you actual atm intensity due to eclipses + fader
	}
	float get_fade_intensity(void) {
		return fader.getInterstate();    // let's you know how far faded in or out the atm is (0-1)
	}
	float get_world_adaptation_luminance(void) const {
		return world_adaptation_luminance;
	}
	float get_milkyway_adaptation_luminance(void) const {
		return milkyway_adaptation_luminance;
	}

	// DO NOT SET DIRECTLY, use core->setLightPollutionLimitingMagnitude instead
	void setLightPollutionLuminance(float luminance) {
		lightPollutionLuminance = luminance;    // for determining world adaptation luminance
	}
	float getLightPollutionLuminance() {
		return lightPollutionLuminance;
	}

private:
	Skylight sky;
	Skybright skyb;
	int sky_resolution;
	Vec3f ** tab_sky;	// For Atmosphere calculation
	int startY;			// intern variable used to store the Horizon Y screen value
	float world_adaptation_luminance;
	float milkyway_adaptation_luminance;
	float atm_intensity;
	ParabolicFader fader;
	float lightPollutionLuminance; 	// light pollution simulation, add to svn 20070220
};

#endif // _ATMOSTPHERE_H_
