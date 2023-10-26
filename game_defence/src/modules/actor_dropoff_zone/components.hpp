#pragma once

#include <vector>

namespace game2d {

struct DropoffZoneComponent
{
  float min_seconds_between_items = 10;
  float max_seconds_between_items = 20;
  float timer = 0;

  std::vector<int> valid_items_to_request{ 0, 1, 3, 4 };
  std::vector<int> requested_items;

  int cur_customers = 0;
  int max_customers = 4;
  std::vector<entt::entity> instantiated_customers;

  entt::entity sign;
};

} // namespace game2d