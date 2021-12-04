#include "system.hpp"

// components
#include "modules/editor_camera/components.hpp"

// helpers
#include "modules/editor_camera/helpers.hpp"

// other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

void
game2d::init_editor_camera_system(entt::registry& registry){
  //
};

void
game2d::update_editor_camera_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& view = registry.view<CameraComponent>();
  view.each([&dt](auto& camera) {
    camera.position = glm::lerp(camera.position, camera.target_position, glm::clamp(dt * camera.damping, 0.0f, 1.0f));
    camera.yaw = glm::lerp(camera.yaw, camera.target_yaw, glm::clamp(dt * camera.damping * 2.0f, 0.0f, 1.0f));
    camera.pitch = glm::lerp(camera.pitch, camera.target_pitch, glm::clamp(dt * camera.damping * 2.0f, 0.0f, 1.0f));

    // calculate new cartesian basis vectors from yaw/pitch pair:
    glm::vec3 front;
    front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    front.y = sin(glm::radians(camera.pitch));
    front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));

    camera.axis_forward = glm::normalize(front);
    camera.axis_right = glm::normalize(glm::cross(camera.axis_forward, camera.world_up));
    camera.axis_up = glm::cross(camera.axis_right, camera.axis_forward);

    update_view(camera);
  });
};