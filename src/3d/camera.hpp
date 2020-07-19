#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#pragma warning( push )
#pragma warning( disable : 4244 )

#include "input/input_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
#include <glm/gtx/euler_angles.hpp>
#include <SDL2/SDL.h>

#include <vector>

namespace fightinggame {

    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    struct CameraState {
        bool forward_pressed    = false;
        bool backward_pressed   = false;
        bool left_pressed       = false;
        bool right_pressed      = false;
        bool up_pressed         = false;
        bool down_pressed       = false;
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
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;
        // Euler Angles
        float Yaw;
        float Pitch;
        // Camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;
        CameraState state;

        // Constructor with vectors
        Camera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = YAW,
            float pitch = PITCH)
            : Front(glm::vec3(0.0f, 0.0f, -1.0f))
            , MovementSpeed(SPEED)
            , MouseSensitivity(SENSITIVITY)
            , Zoom(ZOOM)
            , state(CameraState{})
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
            , MouseSensitivity(SENSITIVITY)
            , Zoom(ZOOM)
        {
            Position = glm::vec3(posX, posY, posZ);
            WorldUp = glm::vec3(upX, upY, upZ);
            Yaw = yaw;
            Pitch = pitch;
            update_camera_vectors();
        }

        // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
        glm::mat4 get_view_matrix() const
        {
            return glm::lookAt(Position, Position + Front, Up);
        }

        glm::mat4 get_view_projection_matrix(int width, int height) const
        {
            glm::mat4 projection = glm::perspective(glm::radians(Zoom), (float)width / (float)height, 0.1f, 100.0f);
            glm::mat4 view = get_view_matrix();
            return projection * view;
        }

        void process_users_input(InputManager& input_manager)
        {
            state.forward_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_w);
            state.backward_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_s);
            state.left_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_a);
            state.right_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_d);
            state.up_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_SPACE);
            state.down_pressed = input_manager.get_key_held(SDL_KeyCode::SDLK_LSHIFT);
        }

        void update(float delta_time)
        {
            float velocity = MovementSpeed * delta_time;

            if (state.forward_pressed && state.backward_pressed)
            {
                //do nothing
            }
            else if (state.forward_pressed)
            {
                Position += Front * velocity;
            }
            else if (state.backward_pressed) {
                Position += -(Front * velocity);
            }

            if (state.left_pressed && state.right_pressed)
            {
                //do nothing
            }
            else if (state.left_pressed)
            {
                Position += -(Right * velocity);
            }
            else if (state.right_pressed) {
                Position += Right * velocity;
            }

            if (state.up_pressed && state.down_pressed)
            {
                //do nothing
            }
            else if (state.up_pressed)
            {
                Position += Up * velocity;
            }
            else if (state.down_pressed) {
                Position += -(Up * velocity);
            }
        }

        void process_events(const SDL_Event& e)
        {
            //int mouse_x, mouse_y;
            //SDL_GetMouseState(&mouse_x, &mouse_y);

            switch (e.type)
            {
            case SDL_MOUSEWHEEL:
                process_mouse_scroll(e.wheel.y);
                break;
            case SDL_MOUSEMOTION:
                float xrel = e.motion.xrel;
                float yrel = e.motion.yrel;
                //printf("mousePos x: %f y: %f", xrel, yrel);

                process_mouse_movement(xrel, yrel);
                break;
            }
        }

        // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void process_mouse_movement(float xoffset, float yoffset, GLboolean constrainPitch = true)
        {
            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;

            Yaw += xoffset;
            Yaw = glm::mod(Yaw + xoffset, 360.0f);

            Pitch += (yoffset * -1);

            // Make sure that when pitch is out of bounds, screen doesn't get flipped
            if (constrainPitch)
            {
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }

            // Update Front, Right and Up Vectors using the updated Euler angles
            update_camera_vectors();
        }

        // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
        void process_mouse_scroll(float yoffset)
        {
            Zoom -= (float)yoffset;
            if (Zoom < 1.0f)
                Zoom = 1.0f;
            if (Zoom > 45.0f)
                Zoom = 45.0f;
        }


    private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void update_camera_vectors()
        {
            // calculate the new Front vector
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Front = glm::normalize(front);
            // also re-calculate the Right and Up vector
            Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
            Up = glm::normalize(glm::cross(Right, Front));
        }

    };
}
#pragma warning( pop )

#endif
