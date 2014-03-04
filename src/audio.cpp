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

// manage an audio track (SDL mixer music track)

#include <iostream>
#include "audio.h"
#include "nightshade.h"
#include "translator.h"
#include "shared_data.h"
#include <sstream>

// Define stub functions to link against if we don't have SDL_MIXER
#ifndef HAVE_SDL_MIXER_H
	static int MIX_MAX_VOLUME = 0;
	static int MIX_DEFAULT_FORMAT = 0;
	int Mix_OpenAudio( int, int, int, int ){return 0;}
	Mix_Music* Mix_LoadMUS( const char* ){return NULL;}
	int Mix_HaltMusic(void){return 0;}
	void Mix_CloseAudio(void){}
	int Mix_PlayMusic(Mix_Music*, int){return 0;}
	int Mix_SetMusicPosition(double){return 0;}
	void Mix_ResumeMusic(void){}
	void Mix_PauseMusic(void){}
	int Mix_VolumeMusic(int){return 0;}
	void Mix_HookMusicFinished(void (*music_finished)()){};
	void Mix_FreeMusic( Mix_Music* ){};
#endif


AudioPlayer* AudioPlayer::m_instance = NULL;
bool Audio::is_disabled = false;

AudioPlayer::AudioPlayer(){
	master_volume = 1.0;

	MediaState state;
	state.volume = master_volume;
	SharedData::Instance()->Media( state );

	ScriptState sstate;
	sstate.volume = master_volume;
	SharedData::Instance()->Script( sstate );

	Mix_HookMusicFinished(AudioPlayer::OnFinished);
}

AudioPlayer& AudioPlayer::Instance(void){
	if( !m_instance ) {
		m_instance = new AudioPlayer();
	}
	return *m_instance;
}

void AudioPlayer::OnFinished(void){
	MediaState state;
	state.playStateAudio = MediaState::STOPPED;
	SharedData::Instance()->Media( state );
}

// _volume should be between 0 and 1
void AudioPlayer::set_volume(float _volume)
{
	if (_volume >= 0 && _volume <= 1) {
		master_volume = _volume;

		MediaState state;
		state.volume = master_volume;
		SharedData::Instance()->Media( state );

		ScriptState sstate;
		sstate.volume = master_volume;
		SharedData::Instance()->Script( sstate );

#ifdef DESKTOP
		Mix_VolumeMusic( int(MIX_MAX_VOLUME * master_volume) );
#else
		std::ostringstream oss;
		oss << "amixer set PCM " << int(0.5 + master_volume*100) << "% >> /dev/null";
		std::string comm = oss.str();
		system(comm.c_str());
#endif

	}
}

void AudioPlayer::increment_volume()
{
	master_volume += 0.02f;

	if (master_volume > 1) master_volume = 1;

	MediaState state;
	state.volume = master_volume;
	SharedData::Instance()->Media( state );

	ScriptState sstate;
	sstate.volume = master_volume;
	SharedData::Instance()->Script( sstate );

#ifdef DESKTOP
	Mix_VolumeMusic(int(MIX_MAX_VOLUME*master_volume));
#else
	system("amixer set PCM 2%+ >> /dev/null");
#endif

}

void AudioPlayer::decrement_volume()
{
	master_volume -= 0.02f;

	if (master_volume < 0) master_volume = 0;

	MediaState state;
	state.volume = master_volume;
	SharedData::Instance()->Media( state );

	ScriptState sstate;
	sstate.volume = master_volume;
	SharedData::Instance()->Script( sstate );

#ifdef DESKTOP
	Mix_VolumeMusic(int(MIX_MAX_VOLUME*master_volume));
#else
	system("amixer set PCM 2%- unmute >> /dev/null");
#endif

}


Audio::Audio(std::string filename, std::string name, long int output_rate, bool fromscript) : m_fromscript(fromscript)
{
	if (output_rate < 1000)
		output_rate = 22050;

	// initialize audio
	if (Mix_OpenAudio(output_rate, MIX_DEFAULT_FORMAT, 2, 2048)) {
		cout << "Unable to open audio output!" << endl;
		track = NULL;
		return;
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME);

	track = Mix_LoadMUS(filename.c_str());
	if (track == NULL) {
		is_playing = 0;
		std::cout << "Could not load audio file " << filename << endl;
	} else is_playing = 1;

	if (is_disabled) disable();

	track_name = name;
}

Audio::~Audio()
{
	if(track) {
		is_playing=0;
		Mix_HaltMusic(); // stop playing
		Mix_FreeMusic(track);  // free memory
	}

	MediaState state;
	state.playStateAudio = MediaState::STOPPED;
	SharedData::Instance()->Media( state );

	// stop audio use
	Mix_CloseAudio();
}

// used when ffwd/resume playing a script with audio
void Audio::disable()
{
	if (is_playing) Mix_PauseMusic();
	is_disabled = 1;
}


bool Audio::drop( bool fromScript ) {
	if( m_fromscript == fromScript ) {
		delete this;
		return true;
	}
	return false;
}

void Audio::enable()
{
    if( !is_disabled )
    	return;

	is_disabled=0;
	sync();
}

bool Audio::from_script( void ) {
	return m_fromscript;
}

void Audio::play(bool loop)
{

	// TODO check for load errors
	if (track) {
		is_playing = 1;
		elapsed_seconds = 0;
		//cout << "Play Mid\n" << endl;
		MediaState state;
		state.playStateAudio = MediaState::PLAYING;
		SharedData::Instance()->Media( state );

		if (loop)
			Mix_PlayMusic(track, -1);
		else
			Mix_PlayMusic(track, 0);

		if (is_disabled) disable();
	}
}

// used solely to track elapsed seconds of play
void Audio::update(int delta_time)
{

	if (track && is_playing) elapsed_seconds += delta_time/1000.f;

}

// sychronize with elapsed time
// no longer starts playback if paused or disabled
void Audio::sync()
{
	if (track==NULL) return;

	if (is_playing && !is_disabled) {
		Mix_SetMusicPosition(elapsed_seconds);  // ISSUE doesn't work for all audio formats
		Mix_ResumeMusic();
	}
}

void Audio::pause()
{
	if (is_playing) {
		Mix_PauseMusic();
		MediaState state;
		state.playStateAudio = MediaState::PAUSED;
		SharedData::Instance()->Media( state );
	}
	is_playing=0;
}

void Audio::resume()
{
	is_playing=1;
	MediaState state;
	state.playStateAudio = MediaState::PLAYING;
	SharedData::Instance()->Media( state );
	sync();
}

void Audio::stop()
{
	Mix_HaltMusic();
	MediaState state;
	state.playStateAudio = MediaState::STOPPED;
	SharedData::Instance()->Media( state );
	is_playing=0;
}
