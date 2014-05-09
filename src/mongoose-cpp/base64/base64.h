/*
 * base64.h
 *
 *  Created on: May 8, 2014
 *      Author: dev
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <string>

namespace Base64 {
	std::string base64_encode(char const* , unsigned int len);
	std::string base64_decode(std::string const& s);
}

#endif /* BASE64_H_ */
