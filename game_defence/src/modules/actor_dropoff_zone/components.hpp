#pragma once

#include <vector>

namespace game2d {

struct DropoffZoneComponent
{
  float min_seconds_between_items = 5;
  float max_seconds_between_items = 20;
  float timer = 0;

  std::vector<int> valid_items_to_request{ 0, 1, 2, 3, 4 };
  std::vector<int> requested_items;
};

} // namespace game2d