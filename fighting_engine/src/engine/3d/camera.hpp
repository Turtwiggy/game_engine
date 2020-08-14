#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>

namespace fightingengine {

    // Defines several possible options for camera movement.
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // Default camera values
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;

    // An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
    class Camera
    {
    public:

        // Camera Attributes
        glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 Front;
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Right;
        glm::vec3 WorldUp;
        // Euler Angles

        float Yaw;
        float Pitch;
        // Camera options
        float MovementSpeed;
        float CameraSensitivity;
        float Zoom;

        // Constructor with vectors
        Camera( glm::vec3 position, glm::vec3 up, float yaw = YAW, float pitch = PITCH )
            : Front(glm::vec3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , CameraSensitivity(SENSITIVITY)
            , Zoom(ZOOM)
        {
            Position = position;
            WorldUp = up;
            Yaw = yaw;
            Pitch = pitch;
            update_camera_vectors();
        }

        // Constructor with scalar values
        Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
            : Front(glm::vec3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , CameraSensitivity(SENSITIVITY)
            , Zoom(ZOOM)
        {
            Position = glm::vec3(posX, posY, posZ);
            WorldUp = glm::vec3(upX, upY, upZ);
            Yaw = yaw;
            Pitch = pitch;
            update_camera_vectors();
        }

        glm::mat4 get_view_matrix() const;

        glm::mat4 get_view_projection_matrix(int width, int height) const;

        glm::mat4 get_inverse_projection_view_matrix(int width, int height);

        // Compute the world direction vector based on the given X and Y coordinates in normalized-device space
        glm::vec3 get_eye_ray(float x, float y, float width, float height);

        void update(float delta_time);

        // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void process_mouse_movement(float xoffset, float yoffset, bool constrainPitch = true);

        // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
        void process_mouse_scroll(float yoffset);

    private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void update_camera_vectors();
    };
}
