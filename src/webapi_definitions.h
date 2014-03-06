/*
 * definitions.h
 *
 *  Created on: Mar 4, 2014
 *      Author: dev
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <microhttpd.h>
#include <string>

#define DEFAULT_PORT 8888



enum Command {
	CONSTELLATION_LINES,
	CONSTELLATION_LABELS,
	CONSTELLATION_ART,
	AZIMUTHAL_GRID,
	EQUTORIAL_GRID,
	GROUND,
	CARDINAL_POINTS,
	ATMOSPHERE,
	BODY_LABELS,
	NEBULA_LABELS,
	MOUNT,

	NOT_FOUND,
};

enum State {
	ON,
	OFF,
	TOGGLE
};

struct CommandState {
	Command command;
	State state;
};

#endif /* DEFINITIONS_H_ */
