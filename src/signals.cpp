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

#include "signals.h"
#include "external_viewer.h"
#include <sys/time.h>

App* ISignals::m_app = NULL;

using namespace std;

#if defined(WIN32) || defined(CYGWIN) || defined(__MINGW32__) || defined(MINGW32)
	int setitimer( int, struct itimerval*, struct itimerval* ){ return 0; }
	int sigaction(int, const struct sigaction*, struct sigaction* ){ return 0; };
#endif

ISignals* ISignals::Create( App* const app ) {
	m_app = app;

	if( AppSettings::Instance()->Windows() )
		return new Win32Signals(); 	// Limited signal support, not posix compliant
	else
		return new PosixSignals();
}

void ISignals::NSSigTERM( int ) {
	// Tell app to stop gracefully
	cout << "Caught SIGTERM, shutting down." << endl;
	if( m_app )
		m_app->quit();
}

void ISignals::NSSigTSTP( int ) {
	try {
		NshadeSharedMemory shm;

		if( shm.TryLock() ) {
			// We got the lock, restore default handler for suspend; release lock just before raising
			struct sigaction restore;
			restore.sa_handler = SIG_DFL;
			restore.sa_flags = SA_RESTART;
			sigaction( SIGTSTP, &restore, NULL );

			// Re-register custom SIGCONT handler
			struct sigaction cont;
			cont.sa_handler = ISignals::NSSigCONT;
			cont.sa_flags = SA_RESTART;
			sigaction( SIGCONT, &cont, NULL );

			shm.UnLock();
			raise( SIGTSTP );
		}
		else { // someone has the lock, check again every 5 ms recursively
			struct itimerval timer = {{0, 0}, {0, 5000}};
			setitimer( ITIMER_REAL, &timer, 0 );

			struct sigaction alarm;
			alarm.sa_handler = ISignals::NSSigTSTP;
			alarm.sa_flags = SA_RESETHAND;
			sigaction( SIGALRM, &alarm, NULL );
		}
	}
	catch( ... ) {
		// may not have access to shared memory, just raise default suspend
		struct sigaction restore;
		restore.sa_handler = SIG_DFL;
		restore.sa_flags = SA_RESTART;
		sigaction( SIGTSTP, &restore, NULL );
		raise( SIGTSTP );
	}
}

void ISignals::NSSigCONT( int ) {
	// Re-register SIGTSTP handler then raise default SIGCONT
	struct sigaction act;
	act.sa_handler = ISignals::NSSigTSTP;
	act.sa_flags = SA_RESTART;
	sigaction( SIGTSTP, &act, NULL );

	// Restore default continue handler
	struct sigaction restore;
	restore.sa_handler = SIG_DFL;
	restore.sa_flags = SA_RESTART;
	sigaction( SIGCONT, &restore, NULL );

	// Notify external viewer that we've resumed from a suspend
	ExternalViewer viewer;
	viewer.send_last();

	// Raise default continue handler
	raise( SIGCONT );
}

// Posix Signal Implementation ////////////////////////////////////////////////
void PosixSignals::Register( int sigid, void(*handler)(int) ) {
	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = SA_RESTART;
	sigaction ( sigid, &act, NULL);
}

void PosixSignals::UnRegister( int sigid ) {
	struct sigaction act;
	act.sa_handler = SIG_DFL;
	act.sa_flags = SA_RESTART;
	sigaction ( sigid, &act, NULL);
}

// Win32 Signal Implementation ////////////////////////////////////////////////
void Win32Signals::Register( int sigid, void(*handler)(int) ) {
	signal( sigid, handler );
}

void Win32Signals::UnRegister( int sigid ) {
	signal( sigid, SIG_DFL );
}
