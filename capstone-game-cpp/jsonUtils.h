#pragma once

#include "json.h"

using namespace nlohmann;
using namespace std;

using jarr_t = json::array_t;

json findByName(jarr_t& array, string name);