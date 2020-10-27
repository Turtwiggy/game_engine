#pragma once

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/3d/camera/camera.hpp"

namespace fightingengine {

class FlyCamera : public Camera
{
public:
    float Yaw             =  -90.0f;
    float Pitch           =  0.0f;

    float MovementSpeed   =  2.5f;
    float MouseSensitivty =  0.1f;
    float Damping         =  5.0f;
private:
    glm::vec3 m_TargetPosition;
    glm::vec3 m_WorldUp;
    float m_TargetYaw     = 0.0f;
    float m_TargetPitch   = 0.0f;

public:
    FlyCamera(
      glm::vec3 position
    , glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f)
    , glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f) );

    virtual void Update(float dt);

    virtual void InputKey(float dt, CameraMovement direction);
    virtual void InputMouse(float deltaX, float deltaY);
    virtual void InputScroll(float deltaX, float deltaY);
};

} //fightingengine