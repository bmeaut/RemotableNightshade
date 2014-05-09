/*
 * mwebapi.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: Ákos Pap
 */

#include "mwebapi.h"

#include <string>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include "script_mgr.h"
#include "nebula.h"

#include "base64/base64.h"

using namespace std;

/* ###########################################################################################################
 * 				String contants
 * ###########################################################################################################
 */

/**
 * Constants for HTTP methods.
 */
static string HTTP_GET("GET");
static string HTTP_POST("POST");

/**
 * Constants for main functionality.
 *
 * Each one corresponds to a "controller". These should be at the beginning of the URI
 * and may be followed by one or more parameter parts.
 */
/** Flag type request, eg. constellation lines visibility. */
static string CommandTypeFlag("flag");
/** Short script execution request (custom button in the App). */
static string CommandTypeExecute("execute");
/** Longer, stored script execution request. */
static string CommandTypeRun("run");
/** Current state request. */
static string CommandTypeGetState("state");
/** Script playback & other control request. */
static string CommandTypeControl("control");
static string CommandTypeFetch("fetch");

/**
 * Constants for flag type requests, corresponding to the manual.
 */
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

/**
 * Constants for script playback control.
 */
static string ControlScriptPlayPause("scriptPlayPause");
static string ControlScriptStop("scriptStop");

/**
 * Constant to reset the whole app state.
 */
static string ControlReset("reset");

static string ControlZoom("zoom");
static string ControlZoomIn("in");
static string ControlZoomOut("out");


static string FetchObjectImage("objectImage");
static string Key_ObjectId("objectId");

/**
 * JSON response field name constants.
 */
static string Response("response");

static string FlagState("flagState");

static string ScriptState("scriptState");
static string IsPlaying("isPlaying");
static string CurrentFile("curentFile");
static string IsFaster("isFaster");
static string IsPaused("isPaused");

static string ObjectImageRoot("objectImage");
static string ObjectImage_Id("objectId");
static string ObjectImage_Name("objectName");
static string ObjectImage_Data("data");


/* ###########################################################################################################
 * 				Forward declarations
 * ###########################################################################################################
 */

static CommandState parseFlagURI(string uri);
static QueryParams parseQuery(const string& query);
static string loadImage(const string& path);


/* ###########################################################################################################
 * 				MWebapi implementations
 * ###########################################################################################################
 */

/**
 * Constructs a server instance.
 *
 * Sets the default script directory, and starts the server on the default port (8888).
 */
MWebapi::MWebapi(App& app)
			: MongooseServer()
			, app(app)
			, core(app.getCore())
			, commander(app.getCommander())
			, remoteScriptsDir(core.getScriptDir() + "/remote/") {
	this->setOption("listening_ports", "8888");

	debug();
}

/**
 * Collects the various flag states, and generates a JSON object containing the states.
 */
Json::Value MWebapi::flagStateToJson() {
	Json::Value root;

	root[ConstellationLines] = 	core.getFlagConstellationLines();
	root[ConstellationLabels] = 	core.getFlagConstellationNames();
	root[ConstellationArt] = 		core.getFlagConstellationArt();
	root[AzimuthalGrid] = 			core.getFlagAzimutalGrid();
	root[EquatorialGrid] = 		core.getFlagEquatorGrid();
	root[Ground] = 				core.getFlagLandscape();
	root[CardinalPoints] = 		core.getFlagCardinalsPoints();
	root[Atmosphere] = 			core.getFlagAtmosphere();
	root[BodyLabels] = 			core.getFlagPlanetsHints();
	root[NebulaLabels] = 			core.getFlagNebulaHints();
	// true: Equatorial, false: Altazimutal
	root[Mount] = 					(core.getFlagConstellationLines() == Core::MOUNT_EQUATORIAL);

	return root;
}

/**
 * Collects the current state of the script runner (playback state, script name, ...)
 * and generates a JSON object from it.
 */
Json::Value MWebapi::scriptStateToJson() {
	Json::Value root;

	ScriptMgr& sm = app.getScriptManager();

	root[IsPlaying] = sm.is_playing();

	if (sm.is_playing()) {
		root[CurrentFile] = sm.getCurrentScriptName();
		root[IsPaused] = sm.is_paused();
		root[IsFaster] = sm.is_faster();
	}

	return root;
}

void MWebapi::debug() {
	cout << "DBUGGING!!! -------------------------------------------" << endl << endl;

//	ScriptMgr& s = app.getScriptManager();
//	cout << s.get_script_list(remoteScriptsDir) << endl;

	Json::Value root;
	root[FlagState] = flagStateToJson();
	root[ScriptState] = scriptStateToJson();

	cout << "Current state is: " << endl << root << endl << endl;


	cout << endl << "end of DBUGGING!!! -------------------------------------------" << endl;
}

MWebapi::~MWebapi() {
	// TODO Auto-generated destructor stub
}

/**
 * Main entry point. Handles every incoming request, and dispatches them to the
 * appropriate controller.
 *
 * The incoming request's path part must begin with one of the "controller" constants,
 * then zero or more parameter parts may come.
 * Processing these parameter parts is the controller's responsibility.
 *
 * Each controller should return a boolean value of true, if the request handling was successful,
 * and false if not. Additionally, each controller can append objects to the given JSON object,
 * which will be sent back to the client.
 */
bool MWebapi::handleEvent(ServerHandlingEvent eventCode,
		MongooseConnection& connection, const MongooseRequest& request,
		MongooseResponse& response) {
	bool processed = false;

	response.setConnectionAlive(false);
	response.setCacheDisabled();
	response.setContentType("application/json");

	if (eventCode == MG_NEW_REQUEST) {
		string uri(request.getUri().substr(1)); // remove leading '/'
		cout << "Incoming request at " << uri << endl;

		Json::Value responseContent;

		if (boost::equals(request.getRequestMethod(), HTTP_GET)) {
			if (boost::starts_with(uri, CommandTypeGetState)) {
				// state request
				processed = true;
			} else if (boost::starts_with(uri, CommandTypeFlag)) {
				// Flag request
				uri = uri.substr(CommandTypeFlag.length());
				processed = processFlagRequest(uri, responseContent);
			} else if (boost::starts_with(uri, CommandTypeFetch)) {
				// fetch request
				uri = uri.substr(CommandTypeFetch.length());
				processed = processFetchRequest(uri, request, responseContent);
			}
		} else if (boost::equals(request.getRequestMethod(), HTTP_POST)) {
			if (boost::starts_with(uri, CommandTypeExecute)) {
				// short script execution request
				uri = uri.substr(CommandTypeExecute.length());
				processed = processExecuteRequest(uri, request, responseContent);
			} else if (boost::starts_with(uri, CommandTypeRun)) {
				// long script execution request.
				uri = uri.substr(CommandTypeRun.length());
				processed = processRunRequest(uri, request, responseContent);
			} else if (boost::starts_with(uri, CommandTypeControl)) {
				// script state control request
				uri = uri.substr(CommandTypeControl.length());
				processed = processControlRequest(uri, request, responseContent);
			}
		}

		// always return current state
		processStateRequest(request, responseContent);

		response.setStatus(processed ? 200 : 404);

		Json::FastWriter writer;
		response.addContent(writer.write(responseContent));
		response.write();

		return true;
	}

	return false;
}

/**
 * Controller for flag type requests.
 *
 * Parses the URI, decides which flag should be manipulated, and how, then changes it.
 */
bool MWebapi::processFlagRequest(string uri, Json::Value& response) {
	uri = uri.substr(1); // remove leading '/'

	cout << "Requested a flag: " << uri << endl << endl;

	CommandState cs = parseFlagURI(string(uri));

	if (cs.command == NOT_FOUND) {
		response[Response] =  "Unknown command!";
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
		response[Response] =  "Success!!";
	}


	return true;
}

/**
 * Controller for short script execution requests.
 *
 * Asks the Commander module to execute the incoming commands, line-by-line.
 * Ignores comments (lines starting with a '#')
 *
 * Checks the result of each line, and in case of failure notifies the client
 * about the failing line's number and content.
 *
 * TODO: Can't process "set home_planet xxxxxxxx" type commands, because they fail
 * with a cryptic OpenGL exception. If such command is encountered and it fails,
 * aborts the execution of the current request.
 */
bool MWebapi::processExecuteRequest(string uri, const MongooseRequest& request, Json::Value& response) {

	string query(request.readQueryString());
	stringstream scriptContent(query);

	// DEBUG
//	cout << "Requested a script execution:" << endl;
//	cout << "\t" << query << endl;
	// END DEBUG

	string line;
	int counter = 0;
	bool successful = true;
	string failingLine;
	while (std::getline(scriptContent, line)) {
		counter++;

		// handle comments and empty lines
		if ((line.length() == 0) || (line.length() > 0 && line[0] == '#')) continue;

		cout << "Executing line[" << counter << "]: " << line << endl;
		try {
			successful = commander.execute_command(line);
		} catch (runtime_error& re) {
			cout << "Something went wrong: " << re.what() << endl;
			//cerr << re.what();
			successful = false;
		}
		if (! successful) {
			failingLine = line;
			break;
		}
	}

	if (successful) {
		response[Response] =  (boost::format("Success!! Successfully executed %d lines.") % counter).str();
	} else {
		response[Response] =  (boost::format("Error! Execution failed at line %d: %s") % counter % failingLine).str();
	}

	return true;
}

/**
 * Controller for longer, stored scripts.
 *
 * Requires a filename string as named URL parameter (file), saves the data in the POST payload to
 * a subdir in the default script directory, then makes the ScriptManager play that file.
 *
 * Uses the same engine as the built-in script runner, so their capabilities
 * and limits should be the same.
 */
bool MWebapi::processRunRequest(string uri, const MongooseRequest& request, Json::Value& response) {
	string script(request.readQueryString());
	stringstream scriptContent(script);

	QueryParams params = parseQuery(request.getQueryString());

	// DEBUG
	cout << "Requested a script execution at " << request.getQueryString() << endl;
	cout << "\t" << script << endl;

	// END DEBUG

	string filename;
	QueryParams::const_iterator it;
	if ((it = params.find("file")) != params.end()) {
		filename = it->second;
	} else {
		response[Response] =  "Error! No filename specified!";
		return true;
	}

	if (script.length() == 0) {
		response[Response] = "Error! Script is empty!";
		return true;
	}

	if (writeRemoteScript(scriptContent, filename)) {
		app.getScriptManager().play_script(remoteScriptsDir + params["file"], remoteScriptsDir);
		return "Successfully saved & started to execute the script";
		return true;
	} else {
		response[Response] =  "Error! Failed to save script!";
		return true;
	}


	return true;
}

/**
 * Controller for state request.
 *
 * Assembles the current stare of the Nightshade application.
 */
bool MWebapi::processStateRequest(const MongooseRequest& request, Json::Value& response) {
	response[FlagState] = flagStateToJson();
	response[ScriptState] = scriptStateToJson();

	return true;
}

/**
 * Controller for script playback control request.
 */
bool MWebapi::processControlRequest(string uri, const MongooseRequest& request, Json::Value& response) {
	uri = uri.substr(1); // remove leading '/'

	cout << "Requested a script state change: " << uri << endl << endl;

	ScriptMgr& sm = app.getScriptManager();

	bool success = true;

	if (ControlScriptPlayPause.compare(uri) == 0) {
		if (sm.is_paused()) {
			sm.resume_script();
		} else {
			sm.pause_script();
		}
	} else if (ControlScriptStop.compare(uri) == 0) {
		sm.cancel_script();
	} else if (ControlReset.compare(uri) == 0) {
		cout << "Going to reset!!" << endl << endl;
	} else if (boost::starts_with(uri, ControlZoom)) {

		if (ControlZoomIn.compare(uri.substr(ControlZoom.length() + 1)) == 0) {
			// zoom in
			core.zoomTo(core.getAimFov() - app.getMouseZoom() * core.getAimFov() / 60., 0.2);
		} else {
			// zoom out
			core.zoomTo(core.getAimFov() + app.getMouseZoom() * core.getAimFov() / 60., 0.2);
		}

	} else {
		success = false;
	}

	if (success) {
		response[Response] = "Control action successfully executed: " + uri;
	} else {
		response[Response] = "Failed to execute control action (" + uri + ").";
	}

	return true;
}

bool MWebapi::processFetchRequest(string uri, const MongooseRequest& request, Json::Value& response) {
	uri = uri.substr(1); // remove leading '/'

	if (boost::starts_with(uri, FetchObjectImage)) {
		if (request.getQueryString().length() > 0) {
			// parameterized request
			QueryParams params = parseQuery(request.getQueryString());
			string objectId = params[Key_ObjectId];

			Json::Value oiRoot;
			oiRoot[ObjectImage_Id] = objectId;

			if (boost::starts_with(objectId, "M") || boost::starts_with(objectId, "NGC")) {

			}

			Nebula* nebula = core.getNebulaManager().searchNebula(objectId, false);
			if (nebula == NULL) {
				oiRoot[ObjectImage_Name] = "Not found";
				oiRoot[ObjectImage_Data] = "";
			} else {
				string filePath = nebula->getTexture().getFileName();

				oiRoot[ObjectImage_Name] = nebula->getEnglishName() + "--|--" + nebula->getNameI18n();
				oiRoot[ObjectImage_Data] = loadImage(
						core.getDataRoot() + "/textures/" + filePath);

			}
			response[ObjectImageRoot] = oiRoot;
		} else {
			// general request
		}

		return true;
	}
	return false;
}

void MWebapi::appendNebulaImage(const string& objectId, Json::Value& response) {

}

/**
 * Writes the given stringstream's content to a file with the given filename.
 */
bool MWebapi::writeRemoteScript(stringstream& script, const string& filename) {
	ofstream os;
	os.open(string(remoteScriptsDir + filename).c_str(), ios::out | ios::trunc);

	if (! os.bad()) {
		string line;
		while (std::getline(script, line)) {
			os << line << endl;
		}
		os.close();
		return true;
	}

	return false;
}


/* ###########################################################################################################
 * 				Utility functions (visible only in the current translation unit)
 * ###########################################################################################################
 */

/**
 * Parses the query params part of a request URI, and returns a map of them.
 */
static QueryParams parseQuery(const string& query) {
	static string PAIR_DELIMITER("&");
	static string KEY_DELIMITER("=");
	QueryParams ret;

	if (query.length() == 0) return ret;

	std::vector<string> pairs;
	boost::split(pairs, query, boost::is_any_of(PAIR_DELIMITER));

	for( std::vector<string>::const_iterator i = pairs.begin(); i != pairs.end(); ++i) {
		std::vector<std::string> entry;
		boost::split(entry, *i, boost::is_any_of(KEY_DELIMITER));
		ret[entry[0]] = entry[1];
	}

	for (QueryParams::const_iterator it = ret.begin(); it != ret.end(); ++it) {
		std::cout << it->first << " " << it->second << "\n";
	}

	return ret;
}

static string loadImage(const string& path) {

	ifstream in;

	cout << "opening file " << path << endl;

	in.open(path.c_str(), ios::in|ios::binary);

	if (in.is_open()) {
		// get length of file:
		in.seekg (0, in.end);
		int length = in.tellg();
		in.seekg (0, in.beg);

		cout << "File length: " << length << " bytes" << endl;

		// allocate memory:
		char * buffer = new char [length];

		// read data as a block:
		in.read(buffer,length);

		in.close();

		return Base64::base64_encode(buffer, length);

	}

	cout << "Can't open file!" << endl;

	return "";
}

/**
 * Parses a flag type request URI, and determines which flag should be changed and how.
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
