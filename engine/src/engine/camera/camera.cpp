
// header
#include "engine/camera/camera.hpp"

// other library headers
#ifdef DEBUG
#include "thirdparty/magic_enum.hpp"
#endif
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace fightingengine {

// ---- frustum plane

void
FrustumPlane::SetNormalD(glm::vec3 normal, glm::vec3 point)
{
  Normal = glm::normalize(normal);
  D = -glm::dot(Normal, point);
}

float
FrustumPlane::Distance(glm::vec3 point)
{
  return glm::dot(Normal, point) + D;
}

// ---- camera frustum

void
CameraFrustum::Update(Camera* camera)
{
  float tan = static_cast<float>(2.0 * std::tan(camera->FOV * 0.5));
  float nearHeight = tan * camera->Near;
  float nearWidth = nearHeight * camera->Aspect;
  float farHeight = tan * camera->Far;
  float farWidth = farHeight * camera->Aspect;

  glm::vec3 nearCenter = camera->Position + camera->Forward * camera->Near;
  glm::vec3 farCenter = camera->Position + camera->Forward * camera->Far;

  glm::vec3 v;
  // left plane
  v = (nearCenter - camera->Right * nearWidth * 0.5f) - camera->Position;
  Left.SetNormalD(glm::cross(glm::normalize(v), camera->Up), nearCenter - camera->Right * nearWidth * 0.5f);
  // right plane
  v = (nearCenter + camera->Right * nearWidth * 0.5f) - camera->Position;
  Right.SetNormalD(glm::cross(camera->Up, glm::normalize(v)), nearCenter + camera->Right * nearWidth * 0.5f);
  // top plane
  v = (nearCenter + camera->Up * nearHeight * 0.5f) - camera->Position;
  Top.SetNormalD(glm::cross(glm::normalize(v), camera->Right), nearCenter + camera->Up * nearHeight * 0.5f);
  // bottom plane
  v = (nearCenter - camera->Up * nearHeight * 0.5f) - camera->Position;
  Bottom.SetNormalD(glm::cross(camera->Right, glm::normalize(v)), nearCenter - camera->Up * nearHeight * 0.5f);
  // near plane
  Near.SetNormalD(camera->Forward, nearCenter);
  // far plane
  Far.SetNormalD(-camera->Forward, farCenter);
}

bool
CameraFrustum::Intersect(glm::vec3 point)
{
  for (int i = 0; i < 6; ++i) {
    if (Planes[i].Distance(point) < 0) {
      return false;
    }
  }
  return true;
}

bool
CameraFrustum::Intersect(glm::vec3 point, float radius)
{
  for (int i = 0; i < 6; ++i) {
    if (Planes[i].Distance(point) < -radius) {
      return false;
    }
  }
  return true;
}

bool
CameraFrustum::Intersect(glm::vec3 boxMin, glm::vec3 boxMax)
{
  for (int i = 0; i < 6; ++i) {
    glm::vec3 positive = boxMin;
    if (Planes[i].Normal.x >= 0) {
      positive.x = boxMax.x;
    }
    if (Planes[i].Normal.y >= 0) {
      positive.y = boxMax.y;
    }
    if (Planes[i].Normal.z >= 0) {
      positive.z = boxMax.z;
    }
    if (Planes[i].Distance(positive) < 0) {
      return false;
    }
  }
  return true;
}

// ---- camera

Camera::Camera(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
  : Position(position)
  , Forward(forward)
  , Up(up)
{
  UpdateView();
}

void
Camera::Update(float delta_time)
{
  Frustum.Update(this);
}

void
Camera::SetPerspective(float fov, float aspect, float near, float far)
{
  Perspective = true;
  Projection = glm::perspective(fov, aspect, near, far);
  FOV = fov;
  Aspect = aspect;
  Near = near;
  Far = far;
}

void
Camera::SetOrthographic(float left, float right, float top, float bottom, float near, float far)
{
  Perspective = false;
  Projection = glm::ortho(left, right, top, bottom, near, far);
  Near = near;
  Far = far;
}

void
Camera::UpdateView()
{
  View = glm::lookAt(Position, Position + Forward, Up);
}

float
Camera::FrustumHeightAtDistance(float distance)
{
  if (Perspective) {
    return static_cast<float>(2.0f * distance * tanf(glm::radians(FOV * 0.5)));
  } else {
    return Frustum.Top.D;
  }
}

float
Camera::DistanceAtFrustumHeight(float frustumHeight)
{
  if (Perspective) {
    return frustumHeight * 0.5f / tanf(glm::radians(FOV * 0.5f));
  } else {
    return Frustum.Near.D;
  }
}

glm::mat4
Camera::get_view_projection_matrix() const
{
  glm::mat4 view_projection = Projection * View;
  return view_projection;
}

glm::mat4
Camera::get_inverse_projection_view_matrix() const
{
  return glm::inverse(get_view_projection_matrix());
}

// Compute the world direction vector based on the given X and Y coordinates in
// normalized-device space
glm::vec3
Camera::get_eye_ray(float x, float y) const
{
  glm::vec4 temp(x, y, 0.0f, 1.0f);

  glm::mat4 inverse_projection_view = get_inverse_projection_view_matrix();

  glm::vec4 ray = inverse_projection_view * temp;
  ray /= ray.w;
  ray.x -= Position.x;
  ray.y -= Position.y;
  ray.z -= Position.z;

  return glm::vec3(ray.x, ray.y, ray.z);
}

} // namespace fightingengine
