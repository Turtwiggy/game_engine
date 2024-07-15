#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"

#include "events/helpers/keyboard.hpp"
#include "imgui.h"
#include "maths/maths.hpp"
#include "modules/gameover/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include <SDL_gamecontroller.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

namespace game2d {
using namespace std::literals;

const auto keyboard_key = [](const SDL_Scancode& code) -> InputEvent {
  InputEvent ie;
  ie.type = InputType::keyboard;
  ie.state = InputState::press;
  ie.keyboard = code;
  return ie;
};
const auto controller_button = [](const SDL_GameControllerButton& button) -> InputEvent {
  InputEvent ie;
  ie.type = InputType::controller_button;
  ie.state = InputState::press;
  ie.controller_button = button;
  return ie;
};
const auto controller_axis = [](const SDL_GameControllerAxis& axis) -> InputEvent {
  InputEvent ie;
  ie.type = InputType::controller_axis;
  ie.state = InputState::press;
  ie.controller_axis = axis;
  return ie;
};

const static std::vector<InputEvent> keyboard_combinations = {
  keyboard_key(SDL_SCANCODE_W),
  keyboard_key(SDL_SCANCODE_S),
  keyboard_key(SDL_SCANCODE_A),
  keyboard_key(SDL_SCANCODE_D),
};
const static std::vector<InputEvent> controller_combinations = {
  controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A),
  controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B),
  controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X),
  controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT),
  // controller_button(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT),
};

// store a list of inputs from the player
// the player must enter the right combination in a given time
// or explode

void
check_if_generate_new_combination(entt::registry& r, const int length, engine::RandomState& rnd)
{
  const auto& requests = r.view<GenerateCombinationRequest>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  auto& bamboo_minigame = get_first_component<SINGLE_MinigameBamboo>(r);
  auto& combination = bamboo_minigame.combination;

  // generate new combination...
  combination.clear();
  for (int i = 0; i < length; i++) {
    const int idx = int(engine::rand_det_s(rnd.rng, 0, keyboard_combinations.size()));
    combination.push_back(keyboard_combinations[idx]);
  }
};

void
update_minigame_bamboo_system(entt::registry& r, const float dt)
{
  static engine::RandomState rnd;

  auto& gameover = get_first_component<SINGLETON_GameOver>(r);
  if (gameover.game_is_over)
    return;

  auto& bamboo_minigame = get_first_component<SINGLE_MinigameBamboo>(r);
  auto& buffer = bamboo_minigame.inputs;
  auto& combination = bamboo_minigame.combination;
  auto& last_processed_tick = bamboo_minigame.last_processed_tick;
  bamboo_minigame.time_left -= dt;
  bamboo_minigame.time_left = glm::max(bamboo_minigame.time_left, 0.0f);

  const InputEvent keyboard_submit_key = keyboard_key(SDL_SCANCODE_RETURN);
  const InputEvent controller_submit_key = controller_axis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

  // first combination
  if (combination.size() == 0)
    create_empty<GenerateCombinationRequest>(r);
  check_if_generate_new_combination(r, bamboo_minigame.combination_length, rnd);

  const auto& update_inputs = get_first_component<SINGLETON_InputComponent>(r);

  // Add inputs to buffer.
  for (const auto& input : update_inputs.unprocessed_inputs) {
    if (input.type == InputType::keyboard && input.state == InputState::press) {
      if (input.keyboard != keyboard_submit_key.keyboard) // no submit key
        buffer.push_back(input);
    }
  }

  const auto validate_combination_against_buffer = [&buffer, &combination]() -> bool {
    const auto buffer_size = buffer.size();
    const auto combination_size = combination.size();
    if (buffer_size != combination_size)
      return false;
    for (int i = 0; i < buffer_size; i++) {
      const auto buffer_entry = buffer[i];
      const auto combination_entry = combination[i];

      const bool same_type = buffer_entry.type == combination_entry.type;
      if (!same_type)
        return false;

      const bool same_button = buffer_entry.keyboard == combination_entry.keyboard;
      if (!same_button)
        return false;
    }

    // survived all the checks
    return true;
  };

  static bool combination_correct = false;
  if (get_key_down(update_inputs, keyboard_submit_key.keyboard)) {
    combination_correct |= validate_combination_against_buffer();
    buffer.clear(); // clear your attempt
  }

  // get_axis_01(SDL_GameController *controller, SDL_GameControllerAxis axis)
  // if (trigger_r_press) {
  //   combination_correct |= validate_combination_against_buffer();
  //   buffer.clear(); // clear your attempt
  // }

  // center this window
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints(ImVec2(640, 640 * (9 / 16.0f)), ImVec2(640, 640 * (9 / 16.0f)));

  ImGuiWindowFlags flags = 0;
  // position and sizing
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoResize;
  // visuals
  flags |= ImGuiWindowFlags_NoTitleBar;

  ImGui::Begin("BambooMinigame", NULL, flags);
  ImGui::Text("Input the combination. Press ENTER to submit.");

  ImGui::SeparatorText("Combination");
  for (int i = 0; const auto& key : combination) {
    if (i > 0)
      ImGui::SameLine();
    // const auto* key_name = SDL_GameControllerGetStringForButton(key.controller_button);
    // ImGui::Text("%s ", key_name);
    ImGui::Text("%s ", SDL_GetScancodeName(key.keyboard));
    i++;
  }

  ImGui::SeparatorText("Your Input");

  if (buffer.size() == 0)
    ImGui::Text(" ");

  for (int i = 0; const auto& evt : buffer) {
    if (i > 0)
      ImGui::SameLine();
    if (evt.type == InputType::keyboard)
      ImGui::Text("%s ", SDL_GetScancodeName(evt.keyboard));
    else if (evt.type == InputType::controller_button)
      ImGui::Text("%s ", SDL_GameControllerGetStringForButton(evt.controller_button));
    else
      ImGui::Text("Unknown ");
    i++;
  }

  ImGui::Separator();
  ImGui::Text("Time before detection: %f", bamboo_minigame.time_left);
  if (bamboo_minigame.time_left <= 0.0f) {
    combination_correct = false;
    move_to_scene_start(r, Scene::minigame_bamboo);
  }

  if (combination_correct) {
    ImGui::Text("Your combination was correct!");
    create_empty<GenerateCombinationRequest>(r);
    combination_correct = false;

    if (bamboo_minigame.combination_length == 7) {
      // you win!
      gameover.game_is_over = true;
      gameover.reason = "You solved max combination length";
      gameover.win_condition = true;
      ImGui::End();
      return;
    }

    // increase difficulty
    bamboo_minigame.combination_length++;  // every loop, increase combination by 1
    bamboo_minigame.time_increase += 1.0f; // every loop, you get 1 more second
    bamboo_minigame.time_left = bamboo_minigame.time_left_base;
    bamboo_minigame.time_left += bamboo_minigame.time_increase;
  }

  ImGui::End();
}

} // namespace game2d