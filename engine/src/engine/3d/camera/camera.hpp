#pragma once

// c++ standard library headers
#include <vector>

// other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fightingengine {

class Camera;

/*
    Data container for the 3D plane equation variables in Cartesian space. A
   plane equation can be defined by its Normal (perpendicular to the plane) and
   a distance value D obtained from any point on the plane itself (projected
   onto the normal vector).
*/
class FrustumPlane
{
public:
  glm::vec3 Normal;
  float D;

  void SetNormalD(glm::vec3 normal, glm::vec3 point);

  float Distance(glm::vec3 point);
};

/*
    Container object managing all 6 camera frustum planes as calculated from any
   Camera object. The CameraFrustum allows to quickly determine (using simple
   collision primitives like point, sphere, box) whether an object is within the
   frustum (or crossing the frustum's edge(s)). This gives us the option to
   significantly reduce draw calls for objects that aren't visible anyways. Note
   that the frustum needs to be re-calculated every frame.
*/
class CameraFrustum
{
public:
  // This bit of c++ wizardy means a CameraFrustum can be accessed by
  // using frustum.Left etc
  union
  {
    FrustumPlane Planes[6];
    struct
    {
      FrustumPlane Left;
      FrustumPlane Right;
      FrustumPlane Top;
      FrustumPlane Bottom;
      FrustumPlane Near;
      FrustumPlane Far;
    };
  };

public:
  CameraFrustum() = default;

  void Update(Camera* camera);

  bool Intersect(glm::vec3 point);
  bool Intersect(glm::vec3 point, float radius);
  bool Intersect(glm::vec3 boxMin, glm::vec3 boxMax);
};

// Defines several possible options for camera movement.
enum class CameraMovement
{
  NONE,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
  glm::mat4 Projection;
  glm::mat4 View;

  glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 Forward = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);

  float FOV;
  float Aspect;
  float Near;
  float Far;
  bool Perspective;

  CameraFrustum Frustum;

private:
public:
  Camera() = default;
  Camera(glm::vec3 position, glm::vec3 forward, glm::vec3 up);

  void Update(float dt);

  void SetPerspective(float fov, float aspect, float near, float far);
  void SetOrthographic(float left,
                       float right,
                       float top,
                       float bottom,
                       float near,
                       float far);

  void UpdateView();

  float FrustumHeightAtDistance(float distance);
  float DistanceAtFrustumHeight(float frustumHeight);

  glm::mat4 get_view_projection_matrix() const;
  glm::mat4 get_inverse_projection_view_matrix() const;
  glm::vec3 get_eye_ray(float x, float y) const;

  // u, v is the pixel value from the bottom left as 0,0
  // Ray get_ray(glm::vec3 lower_left, float u, float v, float viewport_width,
  // float viewport_height) const;
};

} // namespace fightingengine
