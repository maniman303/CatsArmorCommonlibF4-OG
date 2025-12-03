#include "StringHelper.h"

namespace StringHelper
{
	std::string ToLower(std::string value)
	{
		size_t length = value.length();
		for (size_t x = 0; x < length; x++) {
			value[x] = (char)tolower(value[x]);
		}

		return value;
	}

	std::list<std::string> SplitString(std::string value, char separator)
	{
		std::list<std::string> result;

		int startPos = 0;
		int length = (int)value.length();

		for (int i = 0; i < length; i++) {
			if (value[i] != separator) {
				continue;
			}

			result.push_back(value.substr(startPos, i - startPos));
			startPos = i + 1;
		}

		if (length - startPos > 0) {
			result.push_back(value.substr(startPos, length - startPos));
		}

		return result;
	}

	bool StartsWith(std::string value, std::string start)
	{
		auto valueLength = value.length();
		auto startLength = start.length();

		if (valueLength < startLength) {
			return false;
		}

		for (size_t i = 0; i < startLength; i++) {
			if (value[i] != start[i]) {
				return false;
			}
		}

		return true;
	}
}
