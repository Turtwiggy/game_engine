
// your header
#include "engine/3d/camera/fly_camera.hpp"

// other library headers
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
//#include "thirdparty/magic_enum.hpp"

namespace fightingengine {

FlyCamera::FlyCamera(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
  : Camera(position, forward, up)
{
  Forward = forward;
  m_WorldUp = Up;
  m_TargetPosition = position;
}

void
FlyCamera::Update(float dt)
{
  Camera::Update(dt);
  // slowly interpolate to target position each frame given some damping factor.
  // this gives smooth camera movement that fades out the closer we are to our
  // target.
  Position =
    glm::lerp(Position, m_TargetPosition, glm::clamp(dt * Damping, 0.0f, 1.0f));
  Yaw = glm::lerp(Yaw, m_TargetYaw, glm::clamp(dt * Damping * 2.0f, 0.0f, 1.0f));
  Pitch = glm::lerp(Pitch, m_TargetPitch, glm::clamp(dt * Damping * 2.0f, 0.0f, 1.0f));

  // calculate new cartesian basis vectors from yaw/pitch pair:
  glm::vec3 front;
  front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  front.y = sin(glm::radians(Pitch));
  front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  Forward = glm::normalize(front);
  Right = glm::normalize(glm::cross(Forward, m_WorldUp));
  Up = glm::cross(Right, Forward);

  // calculate the new view matrix
  UpdateView();
}

void
FlyCamera::InputKey(float dt, CameraMovement direction)
{
  // https://stackoverflow.com/questions/28828957/enum-to-string-in-modern-c11-c14-c17-and-future-c20
  // printf("direction: %s",
  // std::string(magic_enum::enum_name(direction)).c_str());

  float velocity = MovementSpeed * dt;
  if (direction == CameraMovement::FORWARD)
    m_TargetPosition = m_TargetPosition + Forward * velocity;
  else if (direction == CameraMovement::BACKWARD)
    m_TargetPosition = m_TargetPosition - Forward * velocity;
  else if (direction == CameraMovement::LEFT)
    m_TargetPosition = m_TargetPosition - Right * velocity;
  else if (direction == CameraMovement::RIGHT)
    m_TargetPosition = m_TargetPosition + Right * velocity;
  else if (direction == CameraMovement::UP)
    m_TargetPosition = m_TargetPosition + m_WorldUp * velocity;
  else if (direction == CameraMovement::DOWN)
    m_TargetPosition = m_TargetPosition - m_WorldUp * velocity;
}

void
FlyCamera::InputMouse(float deltaX, float deltaY)
{
  float xmovement = deltaX * MouseSensitivty;
  float ymovement = deltaY * MouseSensitivty;

  m_TargetYaw += xmovement;
  m_TargetPitch += (ymovement * -1);

  // prevents calculating the length of the null vector
  if (m_TargetYaw == 0.0f)
    m_TargetYaw = 0.01f;
  if (m_TargetPitch == 0.0f)
    m_TargetPitch = 0.01f;

  // it's not allowed to move the pitch above or below 90 degrees asctime the
  // current world-up vector would break our LookAt calculation.
  if (m_TargetPitch > 89.0f)
    m_TargetPitch = 89.0f;
  if (m_TargetPitch < -89.0f)
    m_TargetPitch = -89.0f;
}

void
FlyCamera::InputScroll(float deltaX, float deltaY)
{
  MovementSpeed = glm::clamp(MovementSpeed + deltaY * 1.0f, 1.0f, 25.0f);
  Damping = glm::clamp(Damping + deltaX * 0.5f, 1.0f, 25.0f);
}

} // fightingengine