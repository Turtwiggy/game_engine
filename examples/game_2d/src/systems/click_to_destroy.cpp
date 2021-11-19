// your header
#include "systems/click_to_destroy.hpp"

// components
#include "components/click_to_destroy.hpp"
#include "components/singleton_grid.hpp"
#include "components/singleton_resources.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/grid.hpp"
#include "engine/maths.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>

void
game2d::update_click_to_destroy_system(entt::registry& registry, engine::Application& app)
{
  auto& res = registry.ctx<SINGLETON_ResourceComponent>();
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  const int& GRID_SIZE = registry.ctx<SINGLETON_GridSize>().size_xy;

  // dont process game events if the viewport says so
  if (!ri.viewport_process_events)
    return;

  // std::cout << "set seed to 0" << std::endl;
  // std::string seed = std::string("0");
  // res.rnd.rng.seed(std::seed_seq(seed.begin(), seed.end()));

  // urgh.. x2
  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);
  // adjust pos to keep square visible while mouse is in 4 corners
  glm::vec2 mouse_pos_adjusted_in_worldspace = mouse_pos;
  mouse_pos_adjusted_in_worldspace.x += GRID_SIZE / 2.0f;
  mouse_pos_adjusted_in_worldspace.y += GRID_SIZE / 2.0f;

  const auto& view = registry.view<PositionIntComponent, HealthComponent, ClickToDestroyComponent>();
  view.each([&res, &registry, &app, &mouse_pos_adjusted_in_worldspace, &GRID_SIZE](
              const auto entity, const auto& pos, auto& health, auto& ctd) {
    glm::ivec2 mouse_grid_slot = engine::grid::world_space_to_grid_space(mouse_pos_adjusted_in_worldspace, GRID_SIZE);
    glm::ivec2 player_grid_slot = engine::grid::world_space_to_grid_space(glm::vec2(pos.x, pos.y), GRID_SIZE);

    if (app.get_input().get_mouse_lmb_down()) {
      if (mouse_grid_slot == player_grid_slot) {

        if (app.get_input().get_mouse_lmb_down()) {
          int random = static_cast<int>(engine::rand_det_s(res.rnd.rng, 1, 21));        // 1-20
          int random_armour = static_cast<int>(engine::rand_det_s(res.rnd.rng, 1, 21)); // 1-20
          std::cout << "you rolled a: " << random << " they saved: " << random_armour << std::endl;
          if (random >= random_armour) {
            std::cout << "hit! you clicked: " << static_cast<uint32_t>(entity) << std::endl;
            health.hp -= 1;
            if (health.hp <= 0) {
              std::cout << "delete entity: " << static_cast<uint32_t>(entity) << std::endl;
              registry.destroy(entity);
            }
          }
        }
      }
    }
  });
};
