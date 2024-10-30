#include "initiative_system.hpp"
#include "engine/entt/helpers.hpp"
#include "initiative_components.hpp"

namespace game2d {

void
update_initiative_system(entt::registry& r)
{
  const auto init_e = get_first<SINGLE_Initiative>(r);
  if (init_e == entt::null)
    create_empty<SINGLE_Initiative>(r);
  auto& init_c = get_first_component<SINGLE_Initiative>(r);

  const auto& view = r.view<InitiativeComponent>();

  std::vector<entt::entity> initiative_list{ view.begin(), view.end() };
  const auto pred = [&view](const entt::entity a, const entt::entity b) {
    const auto a_c = view.get<InitiativeComponent>(a).initiative;
    const auto b_c = view.get<InitiativeComponent>(b).initiative;
    return (a_c < b_c) || ((a_c == b_c) && a < b); // if same initiative, use entity id
  };
  std::sort(initiative_list.begin(), initiative_list.end(), pred);

  init_c.order = initiative_list;
}

} // namespace game2d