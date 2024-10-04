#include "helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/lights/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_stars_shader(entt::registry& r, SINGLE_RendererInfo& ri, bool in_stars_scene, float dt)
{
  ri.stars.bind();

  static glm::vec3 current_pos{ 0, 0, 0 };
  static glm::vec3 target_pos{ 0, 0, 0 };
  static glm::vec2 offset{ 0, 0 };

  const auto camera_e = get_first<OrthographicCamera>(r);
  const bool has_camera = camera_e != entt::null;

  // generate an offset as (0, 0) in the fractal looks weird
  static bool generate_offset = true;
  if (generate_offset) {
    generate_offset = false;
    static engine::RandomState rnd;
    offset.x = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);
    offset.y = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);
    target_pos.x = offset.x;
    target_pos.y = offset.y;
  }

  // starfield to follow player position
  if (has_camera && in_stars_scene) {
    const auto pos = get_position(r, camera_e);
    target_pos.x = pos.x + offset.x;
    target_pos.y = pos.y + offset.y;
  }

  const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
    return b + (a - b) * glm::exp(-decay * dt);
  };
  current_pos.x = exp_decay(current_pos.x, target_pos.x, 32, dt);
  current_pos.y = exp_decay(current_pos.y, target_pos.y, 32, dt);
  // current_pos.y = exp_decay(current_pos.z, target_pos.z, 32, dt);

  ri.stars.set_vec2("player_position", { current_pos.x, current_pos.y });
};

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
  const auto& first_player = get_first<PlayerComponent>(r);
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
    lights[0].luminence = 0.5f;
  }

  // HACK: try adding lights to interesting map features
  /*
  const auto& map_e = get_first<MapComponent>(r);
  {
    if (map_e != entt::null && first_player != entt::null) {
      const auto& map = r.get<MapComponent>(map_e);

      // if player is in the room, light it up
      const auto player_pos = get_position(r, first_player);
      const auto player_gridpos = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);

      const auto rooms = inside_room(r, player_gridpos);
      const auto in_room = rooms.size() > 0;
      const bool inside_spaceship = in_room;

      ri.mix_lighting_and_scene.bind();
      ri.mix_lighting_and_scene.set_bool("inside_spaceship", in_room);

      // increase player brightness outside spaceship.
      if (!inside_spaceship)
        lights[0].luminence = 1.5f;

      // increase player brightness in tunnel (no room lights)
      // if (in_room)
      //   lights[0].luminence = 1.25f;

      if (in_room) {
        int i = 1; // 1 because used 1 light?
        const auto& room_c = r.get<Room>(rooms[0]);
        light_up_room(r, lights, i, room_c, camera_t);
      }
    }
  }
  */

  //
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