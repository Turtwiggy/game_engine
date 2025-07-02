#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/renderer/lights/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_lights(entt::registry& r, SINGLE_RendererInfo& ri)
{
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_t = r.get<TransformComponent>(camera_e);

  const int max_lights = 32;
  static std::vector<Light> lights(max_lights);

  // disable lights every frame?
  for (auto& l : lights) {
    l.enabled = false;
    l.luminence = 0.0f;
  }

  // update the first light position to the first player position.
  const auto first_player = get_first<PlayerBoatComponent, TransformComponent>(r);
  if (first_player != entt::null) {
    glm::vec2 hmm = get_position(r, first_player);
    // worldspace to screenspace
    const auto& wh = ri.viewport_size_render_at;
    hmm -= glm::vec2{ camera_t.position.x, camera_t.position.y };
    hmm += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };
    // player light
    lights[0].pos = hmm;
    lights[0].enabled = true;
    lights[0].colour = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[0].luminence = 0.4f;
  }

  // Add lights to top corners of map
  const auto& map_e = get_first<MapComponent>(r);
  const float lum = 0.85f;
  int i = 1; // 1 because used 1 light?

  if (map_e != entt::null) {
    const auto& map_c = r.get<MapComponent>(map_e);

    std::vector<glm::ivec2> map_lights{
      { 0, 0 },                           // tl
      { 0, map_c.ymax - 1 },              // tr
      { map_c.xmax - 1, 0 },              // bl
      { map_c.xmax - 1, map_c.ymax - 1 }, // br
      { map_c.xmax / 2, map_c.ymax / 2 }, // center
    };

    for (const auto& map_l : map_lights) {
      Light& l = lights[i++];
      l.enabled = true;

      // to worldspace
      l.pos = engine::grid::gridspace_to_worldspace_center(map_l, map_c.tilesize);

      // worldspace to screenspace
      const auto& wh = ri.viewport_size_render_at;
      l.pos -= glm::vec2{ camera_t.position.x, camera_t.position.y };
      l.pos += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };

      // white
      l.colour = engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
      l.luminence = lum;
    }
  }

  // update lighting uniforms
  //
  ri.mix_lighting_and_scene.bind();

  for (size_t i = 0; i < lights.size(); i++) {
    const std::string label = "lights["s + std::to_string(i) + "]."s;

    const auto& l = lights[i];
    ri.mix_lighting_and_scene.set_bool(label + "enabled"s, l.enabled);
    if (!l.enabled)
      continue;
    ri.mix_lighting_and_scene.set_vec2(label + "position"s, l.pos);
    ri.mix_lighting_and_scene.set_vec4(label + "colour"s,
                                       { l.colour.r / 255.0f, l.colour.g / 255.0f, l.colour.b / 255.0f, l.colour.a });
    ri.mix_lighting_and_scene.set_float(label + "luminance"s, l.luminence);

    if (i >= max_lights)
      break;
  }
};

} // namespace game2d