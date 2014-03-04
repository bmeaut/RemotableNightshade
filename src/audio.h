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

// manage an audio track

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <config.h>

#include <string>
#include "SDL.h"

#ifdef HAVE_SDL_MIXER_H
	#include "SDL_mixer.h"
#else
	typedef int Mix_Music;
#endif


class AudioPlayer {
public:
	~AudioPlayer();
	static AudioPlayer& Instance( void );
	void set_volume(float _volume);
	void increment_volume();
	void decrement_volume();
private:
	AudioPlayer();
	static void OnFinished();
	static AudioPlayer* m_instance;
	float master_volume;
};

class Audio
{
public:
	Audio(std::string filename, std::string name, long int output_rate, bool fromscript = true);
	virtual ~Audio();
	void play(bool loop);
	void sync();  // reset audio track time offset to elapsed time
	void pause();
	void resume();
	void stop();
	void disable();
    void enable();
    bool drop( bool );
    bool from_script( void );
	std::string get_name() {
		return track_name;
	};
	void update(int delta_time);

private:
	static bool is_disabled;
	bool is_playing;
	bool m_fromscript;
	Mix_Music *track;
	std::string track_name;
	double elapsed_seconds;  // current offset into the track
};

#endif // _AUDIO_H
