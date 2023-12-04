#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "imgui/helpers.hpp"
#include "maths/inverse_kinematics.hpp"
#include "modules/ui_inverse_kinematics/components.hpp"

#include "imgui.h"
#include <glm/glm.hpp>

namespace game2d {
using namespace std::literals;

void
update_ui_inverse_kinematics_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  auto& ui = get_first_component<SINGLE_IKLines>(r);

  const glm::vec2 goal = mouse_pos;

  static int n_points = 4;
  static float length = 25;
  static int iterations = 5;
  static IKSolver solver;

  static bool first = true;
  if (first) {
    solver.set_points_and_length(n_points, length);
    first = false;
  }

  std::vector<glm::vec2> points;
  for (int i = 0; i < iterations; i++)
    points = solver.Iterate(points, goal);

  ImGui::Begin("Inverse Kinematics");

  ImGui::Text("Results");
  for (const auto& point : points) {
    glm::ivec2 out = point;
    imgui_draw_ivec2({}, out.x, out.y);
  }

  if (ImGui::Button("Add Point")) {
    n_points = glm::max(n_points + 1, 0);
    solver.set_points_and_length(n_points, length);
  }
  if (ImGui::Button("Remove Point")) {
    n_points = glm::max(n_points - 1, 0);
    solver.set_points_and_length(n_points, length);
  }

  ImGui::InputInt("Iterations", &iterations);

  ImGui::End();

  const int selected = points.size();
  const int starting_size = ui.lines.size();

  // Create UI
  for (int i = points.size(); i > starting_size; i--) {
    const auto new_line = create_gameplay(r, EntityType::empty);
    auto& transform = r.get<TransformComponent>(new_line);
    transform.scale = { 0, 0, 0 };
    ui.lines.push_back(new_line);
  }

  // Destroy UI
  for (auto i = ui.lines.size(); i > selected; i--) {
    const auto idx = i - 1;
    auto entity = ui.lines[idx];
    r.destroy(entity); // fine because no aabb
    ui.lines.erase(ui.lines.begin() + idx);
  }

  // Update State
  for (int i = 0; i < selected - 1; i++) {
    const auto line = generate_line(points[i], points[i + 1], 2);
    auto& transform = r.get<TransformComponent>(ui.lines[i]);
    set_transform_with_line(transform, line);
  }
}

} // namespace game2d