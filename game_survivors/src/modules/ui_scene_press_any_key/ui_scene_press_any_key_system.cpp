#include "pch.hpp"

#include "ui_scene_press_any_key_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/system_scene_pressanykey_move_to_next/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

namespace game2d {
using namespace std::literals;

auto init_text = [](entt::registry& r) {
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  // pos_x is 0 because camera is already at center
  const auto pos = glm::vec2(0, -viewport_size_half.y + ri.viewport_size_render_at.y * (6 / 12.0f));

  WorldspaceTextComponent wst_c;

  wst_c.layout = [](entt::registry& r) {
    ImGuiIO& io = ImGui::GetIO();

    auto my_greenish = hex_to_srgb("#71BBB2");
    auto im_greenish = convert_my_to_im_vec(my_greenish);

    ImGui::PushFont(io.Fonts->Fonts[5]); // size 32
    ImGui::TextColored(im_greenish, "Press the 'Any' Key!");
    ImGui::PopFont();

    ImGui::PushFont(io.Fonts->Fonts[3]); // size 20
    ImGui::TextColored(im_greenish, "*If you cant find it, mash your keyboard or controller.");
    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoNav;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;

  auto header_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(header_e);
  r.emplace<WiggleUpAndDown>(header_e,
                             WiggleUpAndDown{
                               .base_position = pos,
                             });
  set_position(r, header_e, pos);
};

void
update_ui_scene_press_any_key(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_PressAnykeyScene, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  if (ui_c.one_frame_buffer) {
    ui_c.one_frame_buffer = false;
    init_text(r);
    return;
  }
};

} // namespace game2d