/*
 * webapi.h
 *
 *  Created on: Mar 4, 2014
 *      Author: dev
 */

#ifndef WEBAPI_H_
#define WEBAPI_H_

#include <microhttpd.h>
#include <webapi_definitions.h>
#include "core.h"

#include <string>


class Webapi {
public:
	Webapi(Core& core);
	virtual ~Webapi();

	bool startListening();

	int getPort() const { return port; }
	void setPort(int port) { this->port = port; }

	int	answer_to_connection (void *cls, struct MHD_Connection *connection,
							  const char *url, const char *method,
							  const char *version, const char *upload_data,
							  size_t *upload_data_size, void **con_cls);

private:
	Core& core;

	int port;
	unsigned int daemonStartFlag;

	struct MHD_Daemon* daemon;


};

#endif /* WEBAPI_H_ */
