/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Network/ENetHttp.h>

#include <Network/ENetHelper.h>

#include <string.h>

#include <stdexcept>
#include <algorithm>
#include <stdio.h>
#include <enet/enet.h>

std::string getDomainFromURL(const std::string& url) {
	size_t domainStart = 0;

	if(url.substr(0,7) == "http://") {
		domainStart += 7;
	}

	size_t domainEnd = url.find_first_of(":/", domainStart);

	return url.substr(domainStart, domainEnd-domainStart);
}

std::string getFilePathFromURL(const std::string& url) {
	size_t domainStart = 0;

	if(url.substr(0,7) == "http://") {
		domainStart += 7;
	}

	size_t domainEnd = url.find_first_of('/', domainStart);

	return url.substr(domainEnd, std::string::npos);
}

int getPortFromURL(const std::string& url) {
	size_t domainStart = 0;

	if(url.substr(0,7) == "http://") {
		domainStart += 7;
	}

	size_t domainEnd = url.find_first_of(":/", domainStart);

	if(domainEnd == std::string::npos) {
		return 0;
	}

	if(url.at(domainEnd) == ':') {
		int port = strtol(&url[domainEnd+1], NULL, 10);

		if(port <= 0) {
			return -1;
		}

		return port;
	} else {
		return 0;
	}
}

std::string percentEncode(const std::string & s) {
	const std::string unreservedCharacters = "-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_~"; // see RFC 3986

	std::string result;
	std::string::const_iterator iter;
	for(iter = s.begin(); iter != s.end(); ++iter) {
		if(unreservedCharacters.find_first_of(*iter) == std::string::npos) {
			// percent encode
			char tmp[4];
			sprintf(tmp, "%%%.2X", (unsigned char) *iter);
			result += tmp;
		} else {
			// copy unmodifed
			result += *iter;
		}
	}

    return result;
}


std::string loadFromHttp(const std::string& url, const std::map<std::string, std::string>& parameters) {

	std::string domain = getDomainFromURL(url);

	std::string filepath = getFilePathFromURL(url);

	int port = getPortFromURL(url);

	if(port < 0 || port > 65535) {
		throw std::runtime_error("Invalid port number");
	}

	if(port == 0) {
		port = PORT_HTTP;
	}

	std::map<std::string, std::string>::const_iterator iter;

	for(iter = parameters.begin(); iter != parameters.end(); ++iter) {

		if(filepath.find_first_of('?') == std::string::npos) {
			// first parameter
			filepath += "?";
		} else {
			filepath += "&";
		}

		filepath += percentEncode(iter->first) + "=" + percentEncode(iter->second);
	}

	return loadFromHttp(domain, filepath, (unsigned short) port);
}

std::string loadFromHttp(const std::string& domain, const std::string& filepath, unsigned short port) {
	ENetAddress address;
	if(enet_address_set_host(&address, domain.c_str()) < 0) {
		throw std::runtime_error("Cannot resolve '" + domain + "'");
	}

	address.port = port;

	ENetSocket httpSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
	if(httpSocket == ENET_SOCKET_NULL) {
		throw std::runtime_error("Unable to create socket");
	}

	if(enet_socket_connect(httpSocket, &address) < 0) {
		throw std::runtime_error("Unable to connect to '" + domain + "'");
	}


	const std::string newline = "\x0D\x0A";
	const std::string doubleNewline = newline + newline;
	std::string request = "GET " + filepath + " HTTP/1.0" + newline + "Host: " + domain + doubleNewline;

	ENetBuffer sendBuffer;
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	sendBuffer.data = (void*) request.c_str();
	sendBuffer.dataLength = request.size();

	if(enet_socket_send(httpSocket, NULL, &sendBuffer, 1) < 0) {
		throw std::runtime_error("Error while sending HTTP request to '" + domain + "'");
	}

	std::string result;

	char resultBuffer[1024];

	while(true) {

		ENetBuffer receiveBuffer;
		memset(&receiveBuffer, 0, sizeof(sendBuffer));
		receiveBuffer.data = resultBuffer;
		receiveBuffer.dataLength = sizeof(resultBuffer);

		int receiveLength = enet_socket_receive(httpSocket, NULL, &receiveBuffer, 1);

		if(receiveLength < 0) {
			throw std::runtime_error("Error while receiving from '" + domain + "'");
		}

		result.append(resultBuffer, receiveLength);

		if((size_t) receiveLength < sizeof(resultBuffer)) {
			break;
		}

	}

	enet_socket_destroy(httpSocket);

	if(result.substr(9,3) != "200") {
		throw std::runtime_error("Server Error: Received status code '" + result.substr(9,3) + "' from " + domain + ": " + result.substr(0, result.find(newline)));
	}

	size_t contentStart = result.find(doubleNewline);

	std::string content = result.substr(contentStart + doubleNewline.length());

	return content;
}


