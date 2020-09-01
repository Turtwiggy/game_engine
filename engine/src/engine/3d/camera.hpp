#pragma once

#include "engine/3d/ray.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>

namespace fightingengine {

    // Defines several possible options for camera movement.
    enum CameraMovement {
        NONE,
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
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
        //Viewport
        float viewport_height;
        float viewport_width;
        glm::vec3 screen_lower_left_corner;
        float focal_length = 1.0;

        // Constructor with vectors
        Camera ( 
            glm::vec3 position, 
            glm::vec3 up, 
            float viewport_width,
            float viewport_height,
            float yaw = YAW, 
            float pitch = PITCH 
        )
            : Front(glm::vec3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , CameraSensitivity(SENSITIVITY)
            , Zoom(ZOOM)
            , viewport_width(viewport_width)
            , viewport_height(viewport_height)
        {
            Position = position;
            WorldUp = up;
            Yaw = yaw;
            Pitch = pitch;
            update_camera_vectors();

            glm::vec3 horizontal = glm::vec3(viewport_width, 0.0, 0.0);
            glm::vec3 vertical = glm::vec3(0.0, viewport_height, 0.0);
            screen_lower_left_corner = Position - horizontal/2.0f - vertical/2.0f - glm::vec3(0.0, 0.0, focal_length);
        }

        void resize(int width, int height);

        glm::mat4 get_view_matrix() const;
        glm::mat4 get_view_projection_matrix(int width, int height) const;
        glm::mat4 get_inverse_projection_view_matrix(float width, float height);

        //u, v is the pixel value from the bottom left as 0,0
        Ray get_ray(float u, float v) const;

        void update(float delta_time, CameraMovement movement);

        // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void process_mouse_movement(float xoffset, float yoffset, bool constrainPitch = true);

        // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
        void process_mouse_scroll(float yoffset);

    private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void update_camera_vectors();
    };
}
