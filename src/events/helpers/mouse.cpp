// header
#include "mouse.hpp"

#include "entt/helpers.hpp"

// other lib headers
#include <imgui.h>

namespace game2d {

bool
get_mouse_lmb_held()
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
  return ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

bool
get_mouse_rmb_held()
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
  return ImGui::IsMouseDown(ImGuiMouseButton_Right);
}

bool
get_mouse_mmb_held()
{
  // return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
}

bool
get_mouse_lmb_press()
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Left);
}

bool
get_mouse_rmb_press()
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Right);
}

bool
get_mouse_mmb_press()
{
  return ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
}

bool
get_mouse_lmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Left);
}

bool
get_mouse_rmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Right);
}

bool
get_mouse_mmb_release()
{
  return ImGui::IsMouseReleased(ImGuiMouseButton_Middle);
}

// Returns mouse pos relative to tl of screen
glm::ivec2
get_mouse_pos()
{
  // int x, y = 0;
  // SDL_GetMouseState(&x, &y);
  // return glm::ivec2(x, y);
  const auto& pos = ImGui::GetIO().MousePos;
  return glm::ivec2(pos.x, pos.y);
}

} // namespace game2d