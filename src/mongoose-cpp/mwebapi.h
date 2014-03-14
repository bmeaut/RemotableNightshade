/*
 * mwebapi.h
 *
 *  Created on: Mar 11, 2014
 *      Author: dev
 */

#ifndef MWEBAPI_H_
#define MWEBAPI_H_

#include <string>

#include "app.h"
#include "mongcpp.h"


using namespace mongoose;
using namespace std;

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

class MWebapi : public MongooseServer {
public:
	MWebapi(App& app);
	virtual ~MWebapi();

	virtual bool handleEvent(ServerHandlingEvent eventCode, MongooseConnection &connection, const MongooseRequest &request, MongooseResponse &response);

protected:
	App& app;
	Core& core;
	AppCommandInterface& commander;

	bool processFlagRequest(string uri, MongooseResponse& response);
	bool processScriptRequest(string uri, const MongooseRequest& request, MongooseResponse& response);
};

#endif /* MWEBAPI_H_ */
