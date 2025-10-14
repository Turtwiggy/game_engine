#include "pch.hpp"

#include "quip_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/systems/system_quip/quip_components.hpp"

namespace game2d {

void
request_quip(entt::registry& r, entt::entity e)
{
  create_empty<RequestQuip>(r,
                            RequestQuip{
                              .thing_to_quip = e,
                              .message = "bonk",
                            });
}

} // namespace game2d