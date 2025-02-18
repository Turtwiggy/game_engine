#include "system.hpp"

#include "components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/system_screenshake/components.hpp"

namespace game2d {

struct ScreenshakeData
{
  float length = 0.0;
  float strength = 0;
};

const std::unordered_map<ScreenshakeType, ScreenshakeData> screenshake_amounts{
  { ScreenshakeType::EXPLODE, ScreenshakeData{ .length = 0.04f, .strength = 0.005 } },
  { ScreenshakeType::SHOOT, ScreenshakeData{ .length = 0.01f, .strength = 0.002f } },
};

void
update_screenshake_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_ScreenshakeComponent, r, shake_e, shake_c);

  // Debug: screenshake
  {
    auto& input_c = get_first_component<SINGLE_InputComponent>(r);
    if (get_key_down(input_c, SDL_SCANCODE_KP_9))
      create_empty<RequestScreenshakeComponent>(r, RequestScreenshakeComponent{ ScreenshakeType::EXPLODE });
    if (get_key_down(input_c, SDL_SCANCODE_KP_8))
      create_empty<RequestScreenshakeComponent>(r, RequestScreenshakeComponent{ ScreenshakeType::SHOOT });
  }

  const auto view = r.view<const RequestScreenshakeComponent>();
  for (const auto& [e, req] : view.each()) {
    const auto& data = screenshake_amounts.at(req.type);
    shake_c.time_left += data.length;
    shake_c.strength += data.strength;
  }
  r.destroy(view.begin(), view.end()); // done requests

  // do the screenshake
  if (shake_c.time_left > 0.0f) {
    shake_c.time_left -= dt;
    shake_c.time_left = glm::max(shake_c.time_left, 0.0f);
  }

  if (shake_c.time_left <= 0.0f)
    shake_c.strength = 0.0;
};

} // namespace game2d