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
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  auto& res = registry.ctx<SINGLETON_ResourceComponent>();

  // dont process game events if the viewport says so
  if (!ri.viewport_process_events) {
    // std::cout << "set seed to 0" << std::endl;
    // std::string seed = std::string("0");
    // res.rnd.rng.seed(std::seed_seq(seed.begin(), seed.end()));
    return;
  }

  const auto& gs = registry.ctx<SINGLETON_GridSize>();
  const int GRID_SIZE = gs.size_xy;

  // urgh.. x2
  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);
  glm::vec2 grid_adjusted_mouse_pos = mouse_pos;
  grid_adjusted_mouse_pos.x += GRID_SIZE / 2.0f;
  grid_adjusted_mouse_pos.y += GRID_SIZE / 2.0f;
  glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(grid_adjusted_mouse_pos, GRID_SIZE);
  glm::ivec2 world_space = grid_slot * GRID_SIZE;

  if (app.get_input().get_mouse_lmb_down()) {
    int random = static_cast<int>(engine::rand_det_s(res.rnd.rng, 1, 21)); // 1-20
    std::cout << "you rolled a: " << random << std::endl;
  }

  const auto& view = registry.view<PositionIntComponent, HealthComponent, ClickToDestroyComponent>();
  view.each([&res, &registry, &app, &grid_adjusted_mouse_pos, &GRID_SIZE](
              const auto entity, const auto& pos, auto& health, auto& ctd) {
    glm::ivec2 mouse_grid_slot = engine::grid::world_space_to_grid_space(grid_adjusted_mouse_pos, GRID_SIZE);
    glm::ivec2 player_grid_slot = engine::grid::world_space_to_grid_space(glm::vec2(pos.x, pos.y), GRID_SIZE);

    if (app.get_input().get_mouse_lmb_down()) {
      if (mouse_grid_slot == player_grid_slot) {
        std::cout << "you clicked: " << static_cast<uint32_t>(entity) << std::endl;
        health.hp -= 1;
        if (health.hp <= 0) {
          std::cout << "delete entity: " << static_cast<uint32_t>(entity) << std::endl;
          registry.destroy(entity);
        }
      }
    }
  });
};
