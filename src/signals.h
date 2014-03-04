/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * File Author: Trystan A. Larey-Williams
 * Copyright (C) 2010 Digitalis Education Solutions, Inc.
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

#ifndef SIGNALS_H_
#define SIGNALS_H_

#include "nightshade.h"
#include "app.h"
#include "app_settings.h"
#include <map>
#include "signal.h"
#include <nshade_shared_memory.h>

// Must define stubs for missing posix signal definitions on Win32 for compilation
#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	struct sigaction{
		void(*sa_handler)(int);
		int sa_flags;
	};
	struct timevalue {
		int a;
		int b;
	};
	struct itimerval {
		struct timevalue t1;
		struct timevalue t2;
	};
	int setitimer( int, struct itimerval*, struct itimerval* );
	int sigaction(int, const struct sigaction*, struct sigaction* );
	#define SA_RESTART 0
	#define SA_RESETHAND 0
	#define SIGCONT 0 // zero is not a valid sig on Win32 so should just error out
	#define SIGTSTP 0
	#define SIGALRM 0
   #define SIGQUIT 0
	#define ITIMER_REAL 0
#endif


class ISignals {
public:
	static ISignals* Create( App* const );

	virtual void Register( int, void(*)(int) ) = 0;
	virtual void UnRegister( int ) = 0;

	// Predefined handlers for suspend and resume (not supported on Win32
	// but should fail silently)
	static void NSSigTSTP( int );
	static void NSSigCONT( int );
	static void NSSigTERM( int );

protected:
	static App* m_app;
};


class Win32Signals : public ISignals {
	friend class ISignals;

public:
	void Register( int, void(*)(int) );
	void UnRegister( int );
	virtual ~Win32Signals(){};

private:
	Win32Signals(){};
};


class PosixSignals : public ISignals {
	friend class ISignals;

public:
	void Register( int, void(*)(int) );
	void UnRegister( int );
	virtual ~PosixSignals(){};

private:
	PosixSignals(){};
};

#endif /* SIGNALS_H_ */
