#pragma once

//your project headers
#include "engine/core/application.hpp"
#include "engine/3d/camera.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/renderer/render_command.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/geometry/triangle.hpp"

namespace game_3d{

struct GameState
{
    void render (
        RendererRayTraced& renderer, 
        const Camera& cam, 
        GameWindow& window, 
        float timer)
    {
        int width, height;
        window.GetSize(width, height);

        //First pass: render all objects
        //RenderCommand::set_clear_colour(glm::vec4(1.0, 0.0, 0.0, 1.0));
        Shader& shader = renderer.first_geometry_pass(cam, width, height);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
        model = glm::scale(model, glm::vec3(1.0f));
        shader.setMat4("model", model);
      
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0));
        //model = glm::scale(model, glm::vec3(2.0f));
        //shader.setMat4("model", model);
        //cubes[1]->model->draw(shader, draw_calls);

        //Second pass: pass raytracer triangle information
        std::vector<FETriangle> t;
        //t = cubes[0]->model->get_all_triangles_in_meshes();
        renderer.second_raytrace_pass(cam, static_cast<float>(width), static_cast<float>(height), t, timer);

        //Third pass: render scene information to quad
        renderer.third_quad_pass();
    }

    uint32_t draw_calls = 0;
    //std::vector<std::shared_ptr<GameObject3D>> cubes;
};

std::vector<Sphere> create_world()
{
    //ground sphere
    Sphere s4;
    s4.position = glm::vec3(0.0f, -100.5f, -1.0f);
    s4.radius = 100.0f;
    s4.mat.material_type = MATERIAL_DIFFUSE;
    s4.mat.albedo_colour = glm::vec3(0.8f, 0.8f, 0.0f);
    //main sphere
    Sphere s1;
    s1.position = glm::vec3(0.0f, 0.0f, -1.0f);
    s1.radius = 0.5f;
    s1.mat.material_type = MATERIAL_DIFFUSE;
    s1.mat.albedo_colour = glm::vec3(0.7f, 0.3f, 0.3f);
    //left sphere
    Sphere s2;
    s2.position = glm::vec3(-1.0f, 0.0f, -1.0f);
    s2.radius = 0.5f;
    s2.mat.material_type = MATERIAL_METAL;
    s2.mat.albedo_colour = glm::vec3(0.8f, 0.8f, 0.8f);
    s2.mat.metal_fuzz = 0.3f;
    //right sphere
    Sphere s3;
    s3.position = glm::vec3(1.0f, 0.0f, -1.0f);
    s3.radius = 0.5f;
    s3.mat.material_type = MATERIAL_METAL;
    s3.mat.albedo_colour = glm::vec3(0.8f, 0.6f, 0.2f);
    s3.mat.metal_fuzz = 0.8f;
    //World
    std::vector<Sphere> spheres;
    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
    spheres.push_back(s4);

    return spheres;
}

} //namespace game_3d




//Old code

// void init(ModelManager& model_manager)
//{
    ////Model: Cornel Box
    //std::shared_ptr cornel_model = model_manager.load_model("assets/models/cornell_box/CornellBox-Original.obj", "cornell_box");
    //FGObject cornel_box = FGObject(cornel_model);
    //Model: Cube
    //std::shared_ptr cube_model = model_manager.load_model("assets/models/cornell_box/CornellBox-Original.obj", "CornellBox");
    //Objects
    // GameObject3D cube_object = GameObject3D(cube_model);
    // GameObject3D cube_object2 = GameObject3D(cube_model);
    // cube_object.transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
    // cube_object2.transform.Position = glm::vec3(2.0f, 5.0f, -15.0f);
    // //Reference to Objects
    // cubes.push_back(std::make_shared<GameObject3D>(cube_object));
    // cubes.push_back(std::make_shared<GameObject3D>(cube_object2));
//}

//void Game::tick(float delta_time_in_seconds, GameState& state, float timer, InputManager& input_manager, Camera& camera)
//{
//printf("ticking state, delta_time: %f \n", delta_time_in_seconds);
//std::shared_ptr<FGObject> cube0 = state.cubes[0];
//cube0->transform.Position.x = 5.0f + glm::sin(timer);
//cube0->transform.Position.y = 0.0f;
//cube0->transform.Position.z = 5.0f + glm::cos(timer);
//const float pi = 3.14;
//const float frequency = 0.3f; // Frequency in Hz
//float bouncy_val = 0.5 * (1.0 + sin(2.0 * pi * frequency * timer));
//cube0->transform.Scale.x = glm::max(0.3f, bouncy_val);
//cube0->transform.Scale.y = 1.0f;
//cube0->transform.Scale.z = glm::max(0.3f, bouncy_val);
////printf("cube pos: %f %f %f", state.cube_pos.x, state.cube_pos.y, state.cube_pos.z);
////printf("lerp sin_val: %f x: %f z: %f \n ", bouncy_val);
////Player Cube
//std::shared_ptr<FGObject> player_cube = state.player;
//if (input_manager.get_key_held(SDLK_UP))
//{
//    player_cube->transform.Position.z += 1.0f * delta_time_in_seconds;
//}
//if (input_manager.get_key_held(SDLK_DOWN))
//{
//    player_cube->transform.Position.z -= 1.0f * delta_time_in_seconds;
//}
//if (input_manager.get_key_held(SDLK_LEFT))
//{
//    player_cube->transform.Position.x -= 1.0f * delta_time_in_seconds;
//}
//if (input_manager.get_key_held(SDLK_RIGHT))
//{
//    player_cube->transform.Position.x += 1.0f * delta_time_in_seconds;
//}