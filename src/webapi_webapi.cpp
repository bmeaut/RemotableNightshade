/*
 * webapi.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: dev
 */

#include "webapi_webapi.h"

#include <string>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;

CommandState parseURI(string uri);
struct MHD_Response* createResponseString(const string& text);

Webapi::Webapi(Core& core)
			: core(core)
			, port(DEFAULT_PORT)
			, daemonStartFlag(MHD_USE_SELECT_INTERNALLY)
			, daemon(NULL) {


}

Webapi::~Webapi() {
	if (daemon != NULL) {
		MHD_stop_daemon(daemon);
	}
}

/**
 * It's a hack! See http://stackoverflow.com/a/15599707
 * It's necessary, because the MHD_start_daemon function doesn't
 * take Webapi::answer_to_connection() as ahc parameter.
 */

Webapi* pWebapi = NULL;
static int webapi_adapter(void* cls, struct MHD_Connection* connection, const char* url,  const char* method,
								 const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls) {
	return pWebapi->answer_to_connection(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
}

bool Webapi::startListening() {
	pWebapi = this;
	daemon = MHD_start_daemon(daemonStartFlag, port,
								NULL, // AcceptPolicyCallback
								NULL, // ^^ params
								&webapi_adapter, // AccessHandlerCallback (function, that processes a request)
								NULL, // ^^ params
								MHD_OPTION_END);

	if (daemon != NULL) {
		printf("daemon is listening on port %d!\n", port);
	} else {
		printf("daemon is NULL. Something went wrong. :(\n");
	}
	return (daemon != NULL);
}

int Webapi::answer_to_connection(void* cls, 			// params
								 struct MHD_Connection* connection,
								 const char* url, 		// requested URI
								 const char* method,    // GET or POST (usually)
								 const char* version,
								 const char* upload_data,
								 size_t* upload_data_size,
								 void** con_cls) {
	CommandState cs = parseURI(string(url));

	MHD_Response* response;
	int ret;
	unsigned int statusCode;

	if (cs.command == NOT_FOUND) {
		const string errorStr("Unknown command!");
		response = createResponseString(errorStr);
		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
		statusCode = MHD_HTTP_BAD_REQUEST;
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

		// TODO prepare a full state report, and send it back
		const string respText("Success!!");

		response = createResponseString(respText);
		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
		statusCode = MHD_HTTP_OK;
	}

	if (NULL != response) {
		ret = MHD_queue_response(connection, statusCode, response);
		MHD_destroy_response(response);
		return ret;
	}

	return MHD_NO;
}


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

CommandState parseURI(string uri) {
	CommandState ret;

	// first character should be a '/'
	uri = uri.substr(1);

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
		ret. state = TOGGLE;
	}

	return ret;
}

struct MHD_Response* createResponseString(const string& text) {
	return MHD_create_response_from_buffer(
			strlen(text.c_str()),
			(void*) text.c_str(),
			MHD_RESPMEM_MUST_COPY
	);
}
