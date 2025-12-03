#pragma once

#include <list>
#include <fstream>

namespace StringHelper
{
	std::string ToLower(std::string value);

	std::list<std::string> SplitString(std::string value, char separator);

	bool StartsWith(std::string value, std::string start);
}
