#pragma once

#include "nlohmann/json.hpp"

#include <string>
#include <unordered_map>

//
// problem statement
//
// When the app launches, load a bunch of on-disk data.
// This on disk-data is a bunch of things and random systems,
// like player preferences (volume, resolutoons),
// and maybe some game data, e.g. amount of gold, units, unlocks
// When the data changes, we want to reflect that change back to disk.
//

struct SINGLE_OnDiskData
{
  std::unordered_map<std::string, nlohmann::json> data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SINGLE_OnDiskData, data);
};
