/*
 * mwebapi.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: dev
 */

#include "mwebapi.h"

#include <string>
#include <time.h>
#include <sstream>
#include <iostream>

#include <boost/algorithm/string/predicate.hpp>

using namespace std;

/* ###########################################################################################################
 * 				String contants
 * ###########################################################################################################
 */

static string HTTP_GET("GET");
static string HTTP_POST("POST");

static string CommandTypeFlag("flag");
static string CommandTypeScript("script");

static string ConstellationLines("constellationLines");
static string ConstellationLabels("constellationLabels");
static string ConstellationArt("constellationArt");
static string AzimuthalGrid("azimuthalGrid");
static string EquatorialGrid("equatorialGrid");
static string Ground("ground");
static string CardinalPoints("cardinalPoints");
static string Atmosphere("atmosphere");
static string BodyLabels("bodyLabels");
static string NebulaLabels("nebulaLabels");
static string Mount("mount");

static string On("on");
static string Off("off");
static string Toggle("toggle");

/* ###########################################################################################################
 * 				Forward declarations
 * ###########################################################################################################
 */

static CommandState parseFlagURI(string uri);
template<class T> inline std::string toString(const T& t);
static string ipToString(long ip);

/* ###########################################################################################################
 * 				MWebapi implementations
 * ###########################################################################################################
 */

MWebapi::MWebapi(Core& core)
			: MongooseServer()
			, core(core) {
	this->setOption("listening_ports", "8888");
}

MWebapi::~MWebapi() {
	// TODO Auto-generated destructor stub
}

template<class T>
inline std::string toString(const T& t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}

static string ipToString(long ip) {
	string res;
	long workIp, a, b, c, d;
	workIp = ip;
	d = workIp % 0x100;
	workIp = workIp >> 8;
	c = workIp % 0x100;
	workIp = workIp >> 8;
	b = workIp % 0x100;
	workIp = workIp >> 8;
	a = workIp;
	res = toString(a) + "." + toString(b) + "." + toString(c) + "." + toString(d);
	return res;
}

const string generateInfoContent(const MongooseRequest &request) {
	string result;
	result = "<h1>Sample Info Page</h1>";
	result += "<br />Request URI: " + request.getUri();
	result += "<br />Your IP: " + ipToString(request.getRemoteIp());

	time_t tim;
	time(&tim);

	result += "<br />Current date & time: " + toString(ctime(&tim));
	result += "<br /><br /><a href=\"/\">Index page</a>";

	return result;
}

void handleInfo(const MongooseRequest &request, MongooseResponse &response) {
	response.setStatus(200);
	response.setConnectionAlive(false);
	response.setCacheDisabled();
	response.setContentType("text/html");
	response.addContent(generateInfoContent(request));
	response.write();
}

bool MWebapi::handleEvent(ServerHandlingEvent eventCode,
		MongooseConnection& connection, const MongooseRequest& request,
		MongooseResponse& response) {
	bool processed = false;

	response.setConnectionAlive(false);
	response.setCacheDisabled();
	response.setContentType("text/html");

	if (eventCode == MG_NEW_REQUEST) {
		string uri(request.getUri().substr(1)); // remove leading '/'
		cout << "Incoming request at " << uri << endl;

		if (boost::equals(request.getRequestMethod(), HTTP_GET)) {
			if (boost::starts_with(uri, CommandTypeFlag)) {
				uri = uri.substr(CommandTypeFlag.length());
				processed = processFlagRequest(uri, response);
			}
		} else if (boost::equals(request.getRequestMethod(), HTTP_POST)) {
			if (boost::starts_with(uri, CommandTypeScript)) {
				uri = uri.substr(CommandTypeScript.length());
				processed = processScriptRequest(uri, request, response);
			}
		}

		response.setStatus(processed ? 200 : 404);
		response.write();

		return true;
	}

	return false;
}

bool MWebapi::processFlagRequest(string uri, MongooseResponse& response) {
	uri = uri.substr(1); // remove leading '/'

	cout << "Requested a flag: " << uri << endl << endl;

	CommandState cs = parseFlagURI(string(uri));

	if (cs.command == NOT_FOUND) {
		response.addContent("Unknown command!");
	} else {
		switch (cs.command) {
			case CONSTELLATION_LINES:
				switch (cs.state) {
					case ON:
						core.setFlagConstellationLines(true);
						break;
					case OFF:
						core.setFlagConstellationLines(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagConstellationLines();
							core.setFlagConstellationLines(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case CONSTELLATION_LABELS:
				switch (cs.state) {
					case ON:
						core.setFlagConstellationNames(true);
						break;
					case OFF:
						core.setFlagConstellationNames(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagConstellationNames();
							core.setFlagConstellationNames(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case CONSTELLATION_ART:
				switch (cs.state) {
					case ON:
						core.setFlagConstellationArt(true);
						break;
					case OFF:
						core.setFlagConstellationArt(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagConstellationArt();
							core.setFlagConstellationArt(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case AZIMUTHAL_GRID:
				switch (cs.state) {
					case ON:
						core.setFlagAzimutalGrid(true);
						break;
					case OFF:
						core.setFlagAzimutalGrid(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagAzimutalGrid();
							core.setFlagAzimutalGrid(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case EQUTORIAL_GRID:
				switch (cs.state) {
					case ON:
						core.setFlagEquatorGrid(true);
						break;
					case OFF:
						core.setFlagEquatorGrid(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagEquatorGrid();
							core.setFlagEquatorGrid(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case GROUND:
				switch (cs.state) {
					case ON:
						core.setFlagLandscape(true);
						break;
					case OFF:
						core.setFlagLandscape(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagLandscape();
							core.setFlagLandscape(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case CARDINAL_POINTS:
				switch (cs.state) {
					case ON:
						core.setFlagCardinalsPoints(true);
						break;
					case OFF:
						core.setFlagCardinalsPoints(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagCardinalsPoints();
							core.setFlagCardinalsPoints(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case ATMOSPHERE:
				switch (cs.state) {
					case ON:
						core.setFlagAtmosphere(true);
						break;
					case OFF:
						core.setFlagAtmosphere(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagAtmosphere();
							core.setFlagAtmosphere(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case BODY_LABELS:
				switch (cs.state) {
					case ON:
						core.setFlagPlanetsHints(true);
						break;
					case OFF:
						core.setFlagPlanetsHints(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagPlanetsHints();
							core.setFlagPlanetsHints(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case NEBULA_LABELS:
				switch (cs.state) {
					case ON:
						core.setFlagNebulaHints(true);
						break;
					case OFF:
						core.setFlagNebulaHints(false);
						break;
					case TOGGLE: {
							bool currState = core.getFlagNebulaHints();
							core.setFlagNebulaHints(!currState);
						}
						break;
					default:
						break;
				}
				break;
			case MOUNT:
				switch (cs.state) {
					case ON:
						core.setMountMode(Core::MOUNT_EQUATORIAL);
						break;
					case OFF:
						core.setMountMode(Core::MOUNT_ALTAZIMUTAL);
						break;
					case TOGGLE: {
							Core::MOUNT_MODE currState = core.getMountMode();
							core.setMountMode(currState == Core::MOUNT_ALTAZIMUTAL
																		? Core::MOUNT_EQUATORIAL
																		: Core::MOUNT_ALTAZIMUTAL);
						}
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}

	response.addContent("Success!!");

	return true;
}

bool MWebapi::processScriptRequest(string uri, const MongooseRequest& request,
		MongooseResponse& response) {

	cout << "Requested a script execution:" << endl;
	cout << "\t" << request.readQueryString() << endl;

	response.addContent("Success!!");

	return true;
}

/* ###########################################################################################################
 * 				Utility functions (visible only in the current translation unit)
 * ###########################################################################################################
 */

static CommandState parseFlagURI(string uri) {
	CommandState ret;

	if (boost::starts_with(uri, ConstellationLines)) {
		ret.command = CONSTELLATION_LINES;
		uri = uri.substr(ConstellationLines.length() + 1);
	} else if (boost::starts_with(uri, ConstellationLabels)) {
		ret.command = CONSTELLATION_LABELS;
		uri = uri.substr(ConstellationLabels.length() + 1);
	} else if (boost::starts_with(uri, ConstellationArt)) {
		ret.command = CONSTELLATION_ART;
		uri = uri.substr(ConstellationArt.length() + 1);
	} else if (boost::starts_with(uri, AzimuthalGrid)) {
		ret.command = AZIMUTHAL_GRID;
		uri = uri.substr(AzimuthalGrid.length() + 1);
	} else if (boost::starts_with(uri, EquatorialGrid)) {
		ret.command = EQUTORIAL_GRID;
		uri = uri.substr(EquatorialGrid.length() + 1);
	} else if (boost::starts_with(uri, Ground)) {
		ret.command = GROUND;
		uri = uri.substr(Ground.length() + 1);
	} else if (boost::starts_with(uri, CardinalPoints)) {
		ret.command = CARDINAL_POINTS;
		uri = uri.substr(CardinalPoints.length() + 1);
	} else if (boost::starts_with(uri, Atmosphere)) {
		ret.command = ATMOSPHERE;
		uri = uri.substr(Atmosphere.length() + 1);
	} else if (boost::starts_with(uri, BodyLabels)) {
		ret.command = BODY_LABELS;
		uri = uri.substr(BodyLabels.length() + 1);
	} else if (boost::starts_with(uri, NebulaLabels)) {
		ret.command = NEBULA_LABELS;
		uri = uri.substr(NebulaLabels.length() + 1);
	} else if (boost::starts_with(uri, Mount)) {
		ret.command = MOUNT;
		uri = uri.substr(Mount.length() + 1);
	} else {
		ret.command = NOT_FOUND;
		return ret;
	}

	if (boost::equals(uri, On)) {
		ret.state = ON;
	} else if (boost::equals(uri, Off)) {
		ret.state = OFF;
	} else if (boost::equals(uri, Toggle)) {
		ret.state = TOGGLE;
	}

	return ret;
}
