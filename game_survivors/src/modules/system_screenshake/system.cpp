#include "system.hpp"

#include "components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/system_screenshake/components.hpp"

namespace game2d {

struct ScreenshakeData
{
  float length = 0.0;
  float strength = 0;
};

void
update_screenshake_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_ScreenshakeComponent, r, shake_e, shake_c);

#if defined(_DEBUG)
  // Debug: screenshake
  static bool do_screenshake = false;
  imgui_draw_bool("screenshake", do_screenshake);
  if (do_screenshake) {
    do_screenshake = false;
    create_empty<RequestScreenshakeComponent>(r, RequestScreenshakeComponent{ ScreenshakeType::EXPLODE });
  }
#endif

  const std::unordered_map<ScreenshakeType, ScreenshakeData> screenshake_amounts{
    { ScreenshakeType::EXPLODE, ScreenshakeData{ .length = 0.04f, .strength = 0.001f } },
    { ScreenshakeType::SHOOT, ScreenshakeData{ .length = 0.01f, .strength = 0.002f } },
  };

  const auto view = r.view<const RequestScreenshakeComponent>();
  for (const auto& [e, req] : view.each()) {
    const auto& data = screenshake_amounts.at(req.type);
    shake_c.time_left += data.length;
    shake_c.strength += data.strength;
  }
  r.destroy(view.begin(), view.end()); // done requests

  // static glm::vec2 shake{ 1.0f, 1.0f };
  static float time = 0.0f;
  time += dt; // time offset so screenshake looks different

  // do the screenshake
  if (shake_c.time_left > 0.0f) {
    shake_c.time_left -= dt;
    shake_c.time_left = glm::max(shake_c.time_left, 0.0f);
  }

  if (shake_c.time_left <= 0.0f) {
    shake_c.strength = { 0.0f, 0.0f };
  } else {
    shake_c.strength.x = glm::cos(time * 10.0f) * 7.5f;
    shake_c.strength.y = glm::cos(time * 15.0f) * 7.5f;
  }
};

} // namespace game2d