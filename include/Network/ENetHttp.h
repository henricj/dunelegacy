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

#ifndef ENETHTTP_H
#define ENETHTTP_H

#include <string>
#include <map>

#define PORT_HTTP   80

std::string getDomainFromURL(const std::string& url);

std::string getFilePathFromURL(const std::string& url);

int getPortFromURL(const std::string& url);

std::string percentEncode(const std::string & s);

std::string loadFromHttp(const std::string& url, const std::map<std::string, std::string>& parameters = std::map<std::string, std::string>());

std::string loadFromHttp(const std::string& domain, const std::string& filepath, unsigned short port = PORT_HTTP);


#endif // ENETHTTP_H
