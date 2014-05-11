/*
 * mwebapi.h
 *
 *  Created on: Mar 11, 2014
 *      Author: Ákos Pap
 */

#ifndef MWEBAPI_H_
#define MWEBAPI_H_

#include <string>

#include "app.h"
#include "json/json.h"
#include "mongcpp.h"
#include "object_base.h"


using namespace mongoose;
using namespace std;

enum Command {
	CONSTELLATION_LINES,
	CONSTELLATION_LABELS,
	CONSTELLATION_ART,
	CONSTELLATION_BOUNDARIES,
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

typedef map<string, string> QueryParams;

class MWebapi : public MongooseServer {
public:
	MWebapi(App& app);
	virtual ~MWebapi();

	virtual bool handleEvent(ServerHandlingEvent eventCode, MongooseConnection &connection, const MongooseRequest &request, MongooseResponse &response);

	void debug();

protected:
	App& app;
	Core& core;
	AppCommandInterface& commander;

	string remoteScriptsDir;

	bool processFlagRequest(string uri, Json::Value& response);
	bool processExecuteRequest(string uri, const MongooseRequest& request, Json::Value& response);
	bool processRunRequest(string uri, const MongooseRequest& request, Json::Value& response);
	bool processStateRequest(const MongooseRequest& request, Json::Value& response);
	bool processControlRequest(string uri, const MongooseRequest& request, Json::Value& response);
	bool processFetchRequest(string uri, const MongooseRequest& request, Json::Value& response);

	void appendImage(ObjectBase* object, Json::Value& destination);

	Json::Value flagStateToJson();
	Json::Value scriptStateToJson();

	bool writeRemoteScript(stringstream& script, const string& params);

};

#endif /* MWEBAPI_H_ */
