#include "scene_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"
#include "modules/animations/wiggle/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/scene_brawl_helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include <SDL_scancode.h>
#include <magic_enum.hpp>

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s)
{
  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("going to scene: {}", scene_name).c_str());

  for (const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
    const auto& [e] = ent_tuple;
    if (const auto* p_c = r.try_get<Persistent>(e))
      continue;
    r.destroy(e);
  };

  // box2d to handle it's own cleanup
  emplace_or_replace_physics_world(r);

  // do not use create_persistent here. anything created
  // here should be expected to be removed between scenes
  create_empty<SINGLE_CurrentScene>(r);
  create_empty<SINGLE_EntityBinComponent>(r);
  create_empty<SINGLE_GameStateComponent>(r);
  create_empty<SINGLE_InputComponent>(r);
  create_empty<SINGLE_ScreenshakeComponent>(r);

  // The first and only transform should be the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  r.get<TransformComponent>(camera_e).position = { 0, 0, 0 };
  r.get<TransformComponent>(camera_e).scale = { 0, 0, 0 };

  audio::sdl_mixer::stop_all_audio(r);

  if (s == Scene::splashscreen) {
    create_empty<SINGLE_SplashScreen>(r);

    auto e = create_empty<TransformComponent>(r);
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "STUDIO_LOGO");
    set_size(r, e, { 512, 512 });
    set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::menu) {
    create_empty<SINGLE_MainMenuUI>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01", true });

    // auto e = create_empty<TransformComponent>(r);
    // r.emplace<SpriteComponent>(e);
    // set_sprite(r, e, "STUDIO_TEXT_LOGO");
    // set_size(r, e, { 512, 256 });
    // set_position(r, e, { 0, 0 }); // center

    auto e = create_empty<TransformComponent>(r);
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "GOBLINBRAWL_BACKGROUND");
    set_size(r, e, { 640 * 2, 360 * 2 });
    set_position(r, e, { 0, 0 }); // center
    set_z_index(r, e, ZLayer::BACKGROUND);
  }

  if (s == Scene::brawl) {

    {
      auto e = create_empty<TransformComponent>(r);
      r.emplace<SpriteComponent>(e);
      set_sprite(r, e, "GOBLINBRAWL_BACKGROUND");
      set_size(r, e, { 640 * 2, 360 * 2 });
      set_position(r, e, { 0, 0 }); // center
      set_z_index(r, e, ZLayer::BACKGROUND);
    }

    create_walls(r);

    std::vector<glm::ivec2> positions{
      { 100, -50 },  // tr
      { 100, 50 },   // br
      { -100, 50 },  // bl
      { -100, -50 }, // tl
    };
    std::vector<SDL_Scancode> keys{
      SDL_SCANCODE_Z,
      SDL_SCANCODE_X,
      SDL_SCANCODE_C,
      SDL_SCANCODE_V,
    };
    const std::vector<glm::ivec2> ui_brawl_key_pos{
      { 214, -92 },  // tr
      { 212, 92 },   // br
      { -217, 93 },  // bl
      { -212, -98 }, // tl
    };
    const std::vector<bool> brawler_is_ai{
      false,
      true,
      true,
      true,
    };

    // create brawlers
    for (int i = 0; i < 4; i++) {
      const auto size = glm::ivec2(32, 32);
      const auto pos = positions[i];
      const auto is_ai = brawler_is_ai[i];

      auto e = r.create();
      r.emplace<TagComponent>(e, "brawler");
      r.emplace<SpriteComponent>(e);
      r.emplace<TransformComponent>(e);
      r.emplace<DefaultColour>(e);

      // Add items to physics system?
      PhysicsDescription pdesc;
      pdesc.type = b2_dynamicBody;
      pdesc.size = size;
      pdesc.position = pos;
      pdesc.linear_damping = 10.0f;
      pdesc.angular_damping = 0.0f;
      // on fixture...
      pdesc.is_sensor = false;
      pdesc.density = 1.0f;
      pdesc.friction = 0.3f;
      // pdesc.restitution = 0.0f; // low to encourage intersect
      create_physics_actor(r, e, pdesc);

      set_size(r, e, size);
      set_sprite(r, e, "PERSON_30_6");
      set_position(r, e, pos);

      r.emplace<ActionKey>(e, keys[i]);
      r.emplace<SeparateTransformAndAABB>(e);
      // r.emplace<WiggleUpAndDown>(e);
      r.emplace<HealthComponent>(e, 5, 5);

      if (is_ai)
        r.emplace<BrawlerAI>(e);
    };

    for (int i = 0; i < 4; i++) {
      const auto e = create_empty<TransformComponent>(r);
      // const auto ui_size = glm::vec2{ 64, 64 };
      set_position(r, e, ui_brawl_key_pos[i]);

      WorldspaceTextComponent worldspace_ui;
      // worldspace_ui.size = { ui_size.x, ui_size.y };
      worldspace_ui.flags = ImGuiWindowFlags_NoDecoration;
      worldspace_ui.flags |= ImGuiWindowFlags_NoDocking;
      worldspace_ui.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      worldspace_ui.flags |= ImGuiWindowFlags_NoInputs;
      worldspace_ui.flags |= ImGuiWindowFlags_AlwaysAutoResize;
      worldspace_ui.flags |= ImGuiWindowFlags_NoBackground;

      worldspace_ui.layout = [&r, keys, i]() {
        const char* name = SDL_GetScancodeName(keys[i]);
        const auto win_size = ImGui::GetWindowSize();
        const auto txt_size = ImGui::CalcTextSize(name);
        const auto pos = ImVec2((win_size.x - txt_size.x) * 0.5f, (win_size.y - txt_size.y) * 0.5f);
        ImGui::SetCursorPos(pos);
        ImGui::Text("%s", name);

        const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
        const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
        const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
        ImVec2 tl{ 0.0f, 0.0f };
        ImVec2 br{ 1.0f, 1.0f };
        const auto result = convert_sprite_to_uv(r, "SKULL_AND_BONES");
        std::tie(tl, br) = result;
        const auto icon_size = ImVec2{ 18, 18 };

        ImGui::Image(im_id, icon_size, tl, br);
        ImGui::SameLine();
        ImGui::Text("0");
      };
      r.emplace<WorldspaceTextComponent>(e, worldspace_ui);

      WiggleUpAndDown wiggle;
      wiggle.base_position = ui_brawl_key_pos[i];
      r.emplace<WiggleUpAndDown>(e, wiggle);
    }
  }

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
};

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  audio::sdl_mixer::stop_all_audio(r);

  // scene idea:

  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("additive scene. scene set to: {}", scene_name).c_str());

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d