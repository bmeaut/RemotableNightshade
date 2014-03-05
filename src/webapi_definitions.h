/*
 * definitions.h
 *
 *  Created on: Mar 4, 2014
 *      Author: dev
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <microhttpd.h>

#define DEFAULT_PORT 8888


enum Command {
	CONSTELLATION_DRAWING_ON,
	CONSTELLATION_DRAWING_OFF,
	CONSTELLATION_DRAWING_TOGGLE,
	NOT_FOUND,
};

#endif /* DEFINITIONS_H_ */
