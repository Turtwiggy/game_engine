#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/controller.hpp"

#include <SDL2/SDL_gamecontroller.h>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <algorithm>
#include <format>

namespace game2d {

void
update_ui_controller_system(entt::registry& r)
{
  auto& input = get_first_component<SINGLE_InputComponent>(r);

  ImGui::Begin("Controller");

  if (ImGui::Button("Re-open all controllers"))
    open_controllers(input);

  for (int i = 0; SDL_GameController * c : input.controllers) {
    i++;
    const auto name = SDL_GameControllerName(c);
    ImGui::Text("Controller: %s", name);

    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(c);
    SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);
    ImGui::Text("Id: %i", joystick_id);

    std::string label = std::string("remove-controller##") + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      SDL_Log("%s", std::format("removing controller...").c_str());
      SDL_GameControllerClose(c);
      auto it = std::find(input.controllers.begin(), input.controllers.end(), c);
      input.controllers.erase(it);
    }

    // clang-format off

    ImGui::Text("SDL_CONTROLLER_AXIS_LEFTX %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX));
    ImGui::Text("SDL_CONTROLLER_AXIS_LEFTY %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY));
    ImGui::Text("SDL_CONTROLLER_AXIS_RIGHTX %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX));
    ImGui::Text("SDL_CONTROLLER_AXIS_RIGHTY %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY));
    ImGui::Text("SDL_CONTROLLER_AXIS_TRIGGERLEFT %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    ImGui::Text("SDL_CONTROLLER_AXIS_TRIGGERRIGHT %f", get_axis_01(c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
    ImGui::Text("SDL_CONTROLLER_BUTTON_START %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START));
    ImGui::Text("SDL_CONTROLLER_BUTTON_BACK %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK));
    ImGui::Text("SDL_CONTROLLER_BUTTON_GUIDE %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE));
    ImGui::Text("SDL_CONTROLLER_BUTTON_A %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A));
    ImGui::Text("SDL_CONTROLLER_BUTTON_B %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B));
    ImGui::Text("SDL_CONTROLLER_BUTTON_X %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X));
    ImGui::Text("SDL_CONTROLLER_BUTTON_Y %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y));
    ImGui::Text("SDL_CONTROLLER_BUTTON_LEFTSHOULDER %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
    ImGui::Text("SDL_CONTROLLER_BUTTON_RIGHTSHOULDER %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
    ImGui::Text("SDL_CONTROLLER_BUTTON_DPAD_UP %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP));
    ImGui::Text("SDL_CONTROLLER_BUTTON_DPAD_DOWN %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN));
    ImGui::Text("SDL_CONTROLLER_BUTTON_DPAD_LEFT %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT));
    ImGui::Text("SDL_CONTROLLER_BUTTON_DPAD_RIGHT %i", get_button_held(c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT));

    // clang-format on
  }
  ImGui::End();
};

} // namespace game2d