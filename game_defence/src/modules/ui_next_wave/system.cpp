#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"

// hack below
#include "actors.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/gameover/components.hpp"
#include "modules/scene/components.hpp"

#include <imgui.h>

//
// this whole file should be changed
//

void
game2d::update_ui_next_wave_system(entt::registry& r)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  // todo: do this properly, however that is
  bool do_ui_action = false;
  const auto& controllers = input.controllers;
  if (controllers.size() > 0) {
    auto* c = controllers[0];
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A))
      do_ui_action = true;
  }

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // hack: this should be in a system in fixedupdate() not here
  bool wave_complete = true;
  for (const auto& [entity, hp, team] : r.view<const HealthComponent, const TeamComponent>().each()) {
    if (team.team == AvailableTeams::enemy) {
      wave_complete = false;
      break;
    }
  }

  if (wave_complete) {

    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

    // set button in bottom right
    const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
    const auto& viewport_size = ImVec2(ri.viewport_size_current.x, ri.viewport_size_current.y);
    const auto pos = ImVec2(viewport_pos.x + viewport_size.x, viewport_pos.y + viewport_size.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));

    ImGui::Begin("Wave Complete", NULL, flags);

    ImVec2 button_size = { 100, 100 };
    if (ImGui::Button("Next Wave", button_size) || do_ui_action) {
      auto& wave = get_first_component<SINGLETON_Wave>(r);

      // give the spawner a random position
      const auto& fixed = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
      engine::RandomState rnd;
      rnd.rng.seed(fixed.fixed_tick);

      auto create_rnd_spawner = [&rnd, &r, &ri]() {
        //
        // hack: using screen size is bad, oh well
        // random pos on screen
        const glm::ivec2 half_size = { ri.viewport_size_current.x / 2.0f, ri.viewport_size_current.y / 2.0f };
        int rnd_x = static_cast<int>(engine::rand_det_s(rnd.rng, -half_size.x + 100, half_size.x - 100));
        int rnd_y = static_cast<int>(engine::rand_det_s(rnd.rng, -half_size.y + 100, half_size.y - 100));
        // give the hearth a chance
        if (glm::abs(rnd_x) < 250)
          rnd_x = 250 * glm::sign(rnd_x);
        if (glm::abs(rnd_y) < 250)
          rnd_y = 250 * glm::sign(rnd_y);

        CreateEntityRequest req;
        req.position.x = rnd_x;
        req.position.y = rnd_y;
        req.type = EntityType::actor_spawner;
        r.emplace<CreateEntityRequest>(r.create(), req);
      };

      for (int i = 0; i < wave.wave; i++)
        create_rnd_spawner();

      wave.wave++;
    }
    ImGui::End();
  }
};
