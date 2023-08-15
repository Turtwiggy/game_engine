#include "system.hpp"

#include "app/application.hpp"
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
game2d::init_input_system(SINGLETON_InputComponent& input, entt::registry& r)
{
  input.state = SDL_GetKeyboardState(NULL);
  open_controllers(input);
};

void
game2d::update_input_system(engine::SINGLETON_Application& app, SINGLETON_InputComponent& input)
{
  input.keys_pressed.clear();
  input.keys_released.clear();
  input.sdl_events.clear();

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    input.sdl_events.push_back(e);

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
    // Note: should move imgui manager to a System...
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
      app.imgui.process_event(&e);
      // continue; // Imgui stole the event
    }

    // keyboard specific
    if (e.type == SDL_KEYDOWN)
      process_key_down(input, e.key.keysym.scancode, e.key.repeat);
    if (e.type == SDL_KEYUP)
      process_key_up(input, e.key.keysym.scancode, e.key.repeat);

    // controller specific
    // if (e.type == SDL_JOYHATMOTION)
    //   process_controller_dpad(e.jhat);

    //   process_controller_button_down(input, e.jbutton);
    if (e.type == SDL_JOYBUTTONDOWN)
      std::cout << "down " << static_cast<int>(e.jbutton.button) << std::endl;

    //   process_controller_button_up(input, e.jbutton);
    if (e.type == SDL_JOYBUTTONUP)
      std::cout << "up " << static_cast<int>(e.jbutton.button) << std::endl;

    if (e.type == SDL_JOYDEVICEADDED)
      process_controller_added(input);
    if (e.type == SDL_JOYDEVICEREMOVED)
      process_controller_removed(input);

  }; // finished polling events

  InputEvent ie;

  ie.type = InputType::keyboard;

  //
  // capture inputs incase FixedUpdate() wants to know about them
  //
  for (const SDL_Event& evt : input.sdl_events) {
    if (evt.type == SDL_KEYDOWN && get_key_down(input, evt.key.keysym.scancode)) {
      ie.key = static_cast<uint32_t>(evt.key.keysym.scancode);
      ie.state = InputState::press;
      input.unprocessed_inputs.push_back(ie);
    }
    if (evt.type == SDL_KEYUP && get_key_up(input, evt.key.keysym.scancode)) {
      ie.key = static_cast<uint32_t>(evt.key.keysym.scancode);
      ie.state = InputState::release;
      input.unprocessed_inputs.push_back(ie);
    }
    if (evt.type == SDL_KEYDOWN && get_key_held(input, evt.key.keysym.scancode)) {
      ie.key = static_cast<uint32_t>(evt.key.keysym.scancode);
      ie.state = InputState::held;
      input.unprocessed_inputs.push_back(ie);
    }
  }

  ie.type = InputType::mouse;

  if (get_mouse_lmb_press()) {
    ie.key = SDL_BUTTON_LEFT;
    ie.state = InputState::press;
    input.unprocessed_inputs.push_back(ie);
  }
  if (get_mouse_lmb_release()) {
    ie.key = SDL_BUTTON_LEFT;
    ie.state = InputState::release;
    input.unprocessed_inputs.push_back(ie);
  }
  if (get_mouse_rmb_press()) {
    ie.key = SDL_BUTTON_RIGHT;
    ie.state = InputState::press;
    input.unprocessed_inputs.push_back(ie);
  }
  if (get_mouse_rmb_release()) {
    ie.key = SDL_BUTTON_RIGHT;
    ie.state = InputState::release;
    input.unprocessed_inputs.push_back(ie);
  }

  ie.type = InputType::controller;

  // float dx = get_axis_01()
};