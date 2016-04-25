#include "stdafx.h"
#include "jsonUtils.h"

json findByName(jarr_t& array, string name)
{
	for (json& child : array)
	{
		if (child["name"] == name) {
			return child;
		}
	}

	return json();
}
