#pragma once

#include "json.h"

nlohmann::json findByName(nlohmann::json::array_t& array, std::string name);