/*
 * webapi.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: dev
 */

#include "webapi_webapi.h"

#include <string>

using namespace std;

Command parseURI(const string& uri);
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
	Command c = parseURI(string(url));

	MHD_Response* response;
	int ret;
	unsigned int statusCode;
	if (c == NOT_FOUND) {
		const string errorStr("Unknown command!");
		response = createResponseString(errorStr);
		statusCode = MHD_HTTP_BAD_REQUEST;
	} else {
		switch (c) {
			case CONSTELLATION_DRAWING_ON:
				core.setFlagConstellationLines(true);
				break;
			case CONSTELLATION_DRAWING_OFF:
				core.setFlagConstellationLines(false);
				break;
			case CONSTELLATION_DRAWING_TOGGLE: {
					bool currState = core.getFlagConstellationLines();
					core.setFlagConstellationLines(!currState);
				}
				break;
			default:
				break;
		}

		// TODO prepare a full state report, and send it back
		const string respText("Success!!");
		response = createResponseString(respText);
		statusCode = MHD_HTTP_OK;
	}

	if (NULL != response) {
		ret = MHD_queue_response(connection, statusCode, response);
		MHD_destroy_response(response);
		return ret;
	}

	return MHD_NO;
}


Command parseURI(const string& uri) {
	if (uri.compare("/constellationDrawingOn") == 0) {
		return CONSTELLATION_DRAWING_ON;
	} else if (uri.compare("/constellationDrawingOff") == 0) {
		return CONSTELLATION_DRAWING_OFF;
	} else if (uri.compare("/constellationDrawingToggle") == 0) {
		return CONSTELLATION_DRAWING_TOGGLE;
	}

	return NOT_FOUND;
}

struct MHD_Response* createResponseString(const string& text) {
	return MHD_create_response_from_buffer(
			(unsigned int) text.length(),
			(void*) text.c_str(),
			MHD_RESPMEM_PERSISTENT
	);
}
