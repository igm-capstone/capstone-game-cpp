#include "stdafx.h"
#include "jsonUtils.h"

using namespace nlohmann;

json findByName(json::array_t& array, std::string name)
{
	for (json& child : array)
	{
		if (child["name"] == name) {
			return child;
		}
	}

	return json();
}
