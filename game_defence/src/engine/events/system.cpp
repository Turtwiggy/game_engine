#include "engine/events/system.hpp"

#include "engine/app/application.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/controller.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <format>
#include <vector>

namespace game2d {

void
init_input_system(entt::registry& r)
{
  SINGLE_InputComponent input;
  input.state = SDL_GetKeyboardState(NULL);
  open_controllers(input);
  destroy_first_and_create<SINGLE_InputComponent>(r, input);
};

void
update_input_system(engine::SINGLE_Application& app, entt::registry& r)
{
  auto& input = get_first_component<SINGLE_InputComponent>(r);
  input.update_since_last_fixed_update = true;

  input.keys_pressed.clear();
  input.keys_released.clear();
  input.button_down.clear();
  input.button_released.clear();

  std::vector<SDL_Event> sdl_events;
  {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      sdl_events.push_back(e);

      // const SDL_EventType type_enum = static_cast<SDL_EventType>(e.type);
      // const std::string type_name = std::string(magic_enum::enum_name(type_enum));
      // SDL_Log("%s", std::format("event... {}", type_name).c_str());

      // Events to quit
      if (e.type == SDL_QUIT)
        app.running = false;

      // https://wiki.libsdl.org/SDL_WindowEvent
      if (e.type == SDL_WINDOWEVENT) {
        switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE: {
            app.running = false;
            break;
          }
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("%s", std::format("Window {} gained keyboard focus", e.window.windowID).c_str());
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("%s", std::format("Window {} lost keyboard focus", e.window.windowID).c_str());
            break;
        }
      }

      // Pass UI the event
      if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        app.imgui.process_event(&e);
        // continue; // Imgui stole the event
      }

      // keyboard specific
      if (e.type == SDL_KEYDOWN)
        process_key_down(input, e.key.keysym.scancode, e.key.repeat);
      if (e.type == SDL_KEYUP)
        process_key_up(input, e.key.keysym.scancode, e.key.repeat);

      // controller
      // these events are not repeating
      // i.e. if you hold it only one event comes through
      if (e.type == SDL_JOYBUTTONDOWN)
        process_button_down(input, e.jbutton.which, static_cast<SDL_GameControllerButton>(e.jbutton.button));
      if (e.type == SDL_JOYBUTTONUP)
        process_button_up(input, e.jbutton.which, static_cast<SDL_GameControllerButton>(e.jbutton.button));

      if (e.type == SDL_JOYHATMOTION) {
        //   const int device = static_cast<int>(evt.jhat.which);
        //   const int value = static_cast<int>(evt.jhat.value);
        // value may be one of the following:
        // SDL_HAT_LEFTUP
        // SDL_HAT_UP
        // SDL_HAT_RIGHTUP
        // SDL_HAT_LEFT
        // SDL_HAT_CENTERED
        // SDL_HAT_RIGHT
        // SDL_HAT_LEFTDOWN
        // SDL_HAT_DOWN
        // SDL_HAT_RIGHTDOWN

        SDL_Log("%s", std::format("TODO: process joyhat button press").c_str());
      }

      if (e.type == SDL_JOYDEVICEADDED) {
        SDL_Log("%s", std::format("controller added").c_str());
        process_controller_added(input);
      }
      if (e.type == SDL_JOYDEVICEREMOVED) {
        SDL_Log("%s", std::format("controller removed").c_str());
        process_controller_removed(input);
      }

      if (e.type == SDL_AUDIODEVICEADDED) {
        SDL_Log("%s", std::format("audio device added. iscapture: {}", e.adevice.iscapture).c_str());
        // game2d::audio::sdl_mixer::process_audio_added(r);
      }
      if (e.type == SDL_AUDIODEVICEREMOVED) {
        SDL_Log("%s", std::format("audio device removed. iscapture: {}", e.adevice.iscapture).c_str());
        // game2d::audio::sdl_mixer::process_audio_removed(r);
      }
    };

  } // finished polling events

#if defined(__EMSCRIPTEN__)
  // emscripten seems to be having issues with mousepos
  int x = 0, y = 0;
  SDL_GetMouseState(&x, &y);
  ImGui::GetIO().MousePos = ImVec2{ float(x), float(y) };
#endif

  process_held_buttons(input);

  {
    InputEvent ie;
    ie.type = InputType::mouse;

    if (get_mouse_lmb_press()) {
      ie.mouse = SDL_BUTTON_LEFT;
      ie.state = InputState::press;
      input.unprocessed_inputs.push_back(ie);
    }
    if (get_mouse_lmb_held()) {
      ie.mouse = SDL_BUTTON_LEFT;
      ie.state = InputState::held;
      input.unprocessed_inputs.push_back(ie);
    }
    if (get_mouse_lmb_release()) {
      ie.mouse = SDL_BUTTON_LEFT;
      ie.state = InputState::release;
      input.unprocessed_inputs.push_back(ie);
    }
    if (get_mouse_rmb_press()) {
      ie.mouse = SDL_BUTTON_RIGHT;
      ie.state = InputState::press;
      input.unprocessed_inputs.push_back(ie);
    }
    if (get_mouse_rmb_held()) {
      ie.mouse = SDL_BUTTON_RIGHT;
      ie.state = InputState::held;
      input.unprocessed_inputs.push_back(ie);
    }
    if (get_mouse_rmb_release()) {
      ie.mouse = SDL_BUTTON_RIGHT;
      ie.state = InputState::release;
      input.unprocessed_inputs.push_back(ie);
    }
  }

  // generate HELD state for controller
  {
    for (SDL_GameController* c : input.controllers) {

      SDL_Joystick* joystick = SDL_GameControllerGetJoystick(c);
      SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);

      const auto generate_held_axis = [&joystick_id](auto& input, auto* c, const SDL_GameControllerAxis& axis) {
        const auto val = get_axis_01(c, axis);
        if (val != 0.0f) {
          InputEvent e;
          e.type = InputType::controller_axis;
          e.joystick_id = joystick_id;
          e.state = InputState::held;
          e.controller_axis = axis;
          e.controller_axis_value_01 = val;
          input.unprocessed_inputs.push_back(e);
        }
      };

      const auto generate_held_input = [&joystick_id](auto& input, auto* c, const SDL_GameControllerButton& button) {
        if (get_button_held(c, button)) {
          InputEvent e;
          e.type = InputType::controller_button;
          e.joystick_id = joystick_id;
          e.state = InputState::held;
          e.controller_button = button;
          input.unprocessed_inputs.push_back(e);
        }
      };

      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);
      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT);
      generate_held_axis(input, c, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT);
      generate_held_input(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    }
  }
};

} // namespace game2d