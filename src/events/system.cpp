#include "system.hpp"

#include "app/application.hpp"
#include "audio/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <imgui.h>

#include <iostream>
#include <vector>

void
game2d::init_input_system(entt::registry& r)
{
  auto& input = get_first_component<SINGLETON_InputComponent>(r);
  input.state = SDL_GetKeyboardState(NULL);
  open_controllers(input);
};

void
game2d::update_input_system(engine::SINGLETON_Application& app, entt::registry& r)
{
  auto& input = get_first_component<SINGLETON_InputComponent>(r);
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

      // Events to quit
      if (e.type == SDL_QUIT)
        app.running = false;

      // https://wiki.libsdl.org/SDL_WindowEvent
      if (e.type == SDL_WINDOWEVENT) {
        switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            app.running = false;
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus \n", e.window.windowID);
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus \n", e.window.windowID);
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

      // if (evt.type == SDL_JOYHATMOTION)
      //   const int device = static_cast<int>(evt.jhat.which);
      //   const int value = static_cast<int>(evt.jhat.value);
      //   // value may be one of the following:
      //   // SDL_HAT_LEFTUP
      //   // SDL_HAT_UP
      //   // SDL_HAT_RIGHTUP
      //   // SDL_HAT_LEFT
      //   // SDL_HAT_CENTERED
      //   // SDL_HAT_RIGHT
      //   // SDL_HAT_LEFTDOWN
      //   // SDL_HAT_DOWN
      //   // SDL_HAT_RIGHTDOWN

      if (e.type == SDL_JOYDEVICEADDED) {
        std::cout << "controller added" << std::endl;
        process_controller_added(input);
      }
      if (e.type == SDL_JOYDEVICEREMOVED) {
        std::cout << "controller removed" << std::endl;
        process_controller_removed(input);
      }

      // if (e.type == SDL_AUDIODEVICEADDED) {
      //   // std::cout << "audio added" << std::endl;
      //   process_audio_added(r);
      // }
      // if (e.type == SDL_AUDIODEVICEREMOVED) {
      //   // std::cout << "audio removed" << std::endl;
      //   process_audio_removed(r);
      // }
    };

  } // finished polling events

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
    InputEvent ie;
    ie.type = InputType::controller;

    for (int i = 0; SDL_GameController * c : input.controllers) {

      i++;
      SDL_Joystick* joystick = SDL_GameControllerGetJoystick(c);
      SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);

      const auto generate_held_axis = [&joystick_id](auto& input, auto* c, const SDL_GameControllerAxis& axis) {
        const auto val = get_axis_01(c, axis);
        if (val != 0.0f) {
          InputEvent e;
          e.type = InputType::controller;
          e.joystick_id = joystick_id;
          e.joystick_event = JoystickEventType::axis;
          e.state = InputState::held;
          e.controller_axis = axis;
          e.controller_axis_value_01 = val;
          input.unprocessed_inputs.push_back(e);
        }
      };

      const auto generate_held_input = [&joystick_id](auto& input, auto* c, const SDL_GameControllerButton& button) {
        if (get_button_held(c, button)) {
          InputEvent e;
          e.type = InputType::controller;
          e.joystick_id = joystick_id;
          e.joystick_event = JoystickEventType::button;
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