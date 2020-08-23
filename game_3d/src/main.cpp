#pragma once

#include "engine/core/application.hpp"
#include "engine/resources/model_manager.hpp"
#include "engine/3d/game_object_3d.hpp"
#include "engine/3d/camera.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/geometry/triangle.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#undef main
//^thanks SDL2

#include <vector>

using namespace fightingengine;

struct GameState
{
    void init(ModelManager& model_manager)
    {
        ////Model: Cornel Box
        //std::shared_ptr cornel_model = model_manager.load_model("assets/models/cornell_box/CornellBox-Original.obj", "cornell_box");
        //FGObject cornel_box = FGObject(cornel_model);

        //Model: Cube
        std::shared_ptr cube_model = model_manager.load_model("assets/models/lizard_wizard/lizard_wizard.obj", "lizard_wizard");

        //Objects
        GameObject3D cube_object = GameObject3D(cube_model);
        GameObject3D cube_object2 = GameObject3D(cube_model);
        cube_object.transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
        cube_object2.transform.Position = glm::vec3(2.0f, 5.0f, -15.0f);
        //Reference to Objects
        cubes.push_back(std::make_shared<GameObject3D>(cube_object));
        cubes.push_back(std::make_shared<GameObject3D>(cube_object2));
    }

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

    void render(RendererRayTraced& renderer, Camera& cam, GameWindow& window, float timer)
    {
        int width, height;
        window.GetSize(width, height);

        //First pass: render all objects
        Shader& shader = renderer.first_geometry_pass(cam, width, height);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
        model = glm::scale(model, glm::vec3(1.0f));
        shader.setMat4("model", model);
        cubes[0]->model->draw(shader, draw_calls);

        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0));
        //model = glm::scale(model, glm::vec3(2.0f));
        //shader.setMat4("model", model);
        //cubes[1]->model->draw(shader, draw_calls);

        //Second pass: pass raytracer triangle information
        std::vector<FETriangle> t;
        t = cubes[0]->model->get_all_triangles_in_meshes();
        //triangles.push_back(cubes[1]->model->get_all_triangles_in_meshes());

        renderer.second_raytrace_pass(cam, width, height, t, timer);
        //Third pass: render scene information to quad
        renderer.third_quad_pass();
    }

    uint32_t draw_calls = 0;
    std::vector<std::shared_ptr<GameObject3D>> cubes;
};

int main(int argc, char** argv)
{
    int width = 1080;
    int height = 720;
    Application app("Fighting Game!", width, height);

    //Camera
    Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ModelManager model_manager;

    GameState state;
    state.init(model_manager);

    RendererRayTraced renderer;
    renderer.init(width, height);

    float timer = 0.0f;
    while (app.is_running())
    {
        app.frame_begin();
        app.poll();
        float delta_time_s = app.get_delta_time();
        timer += delta_time_s;

        //Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        //Toggle mouse capture
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            app.get_window().ToggleMouseCaptured();

        //Fullscreen
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f))
        {
            app.get_window().ToggleFullscreen();
            int width, height;
            app.get_window().GetSize(width, height);
            std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;
            renderer.resize(width, height);
        }

        //Window resized event
        //if (app.get_event().window_resized)
        //    renderer.resize(width, height);

        // ~~ Camera ~~
        if(app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
            camera.update(delta_time_s, CameraMovement::FORWARD);
        else if(app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
            camera.update(delta_time_s, CameraMovement::BACKWARD);
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
            camera.update(delta_time_s, CameraMovement::LEFT);
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
            camera.update(delta_time_s, CameraMovement::RIGHT);
        else
            camera.update(delta_time_s, CameraMovement::NONE);

        // ~~ rendering ~~
        state.render(renderer, camera, app.get_window(), timer);

        // ~~ GUI ~~
        app.gui_begin();

        ImGui::Begin("Hello Window");
        ImGui::Text("Hello World");
        ImGui::End();

        app.gui_end();

        app.frame_end(delta_time_s);
    }

    return 0;
}

// ~~~~~~~~~ Profiling ~~~~~~~~~~~~~~

//// input
//// -----
//profiler.Begin(Profiler::Stage::SdlInput);
//profiler.End(Profiler::Stage::SdlInput);
//
//// Game State Tick
//// ---------------
//{
//    profiler.Begin(Profiler::Stage::GameTick);
//
//    seconds_since_last_game_tick += delta_time_in_seconds;
//
//    //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
//    //{
//    //    //Fixed update
//    //    fixed_tick(SECONDS_PER_FIXED_TICK);
//    //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
//    //}
//
//    profiler.End(Profiler::Stage::GameTick);
//}

//ImGui::Begin("Profiler");
//
//ImGui::Columns(1);
//ImGui::Text("FPS: %f", game.fps_buffer.average());
//
//ImGui::Columns(1);
//
//float draw_time = profiler.GetTime(Profiler::Stage::SdlInput);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::SdlInput].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::GameTick);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::GameTick].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::NewFrame);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::NewFrame].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::MainDraw);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::MainDraw].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::GuiLoop);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::EndFrame);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::EndFrame].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::Sleep);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::Sleep].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::UpdateLoop);
//ImGui::Text(" ~~ %s % fms ~~ ", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (draw_time));
//
//ImGui::End();
