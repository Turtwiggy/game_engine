#define _CRT_SECURE_NO_WARNINGS

//c++ standard library headers
#include <memory>
#include <vector>
#include <string_view>

//other library headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

//your project headers
#include "engine/core/application.hpp"
#include "engine/core/maths/random.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/triangle.hpp"
#include "engine/3d/renderer/renderer_pbr.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/3d/renderer/renderer_simple.hpp"
#include "engine/scene/background.hpp"
#include "engine/scene/scene.hpp"
#include "engine/scene/scene_node.hpp"
#include "engine/resources/resource_manager.hpp"
#include "engine/tools/profiler.hpp"
using namespace fightingengine;

#include "panels/scene_hierarchy_panel.hpp"
#include "panels/profiler_panel.hpp"
#include "game_state.hpp"
using namespace game_3d;

int main(int argc, char** argv)
{
uint32_t width = 1366;
uint32_t height = 768;
Application app("Fighting Game!", width, height);
app.set_fps_limit(60.0f);   

FlyCamera camera {glm::vec3(0.0f, 0.0f, -1.0f)};
camera.SetPerspective(glm::radians(65.0f), (float)width/(float)height, 0.1f, 100.0f);

GameState state;
RandomState rnd;
Profiler profiler;

//RendererRayTraced renderer(width, height);
//RendererPBR pbr_renderer;
RendererSimple simple_renderer;
RenderCommand::init();

//UI Panels
ProfilerPanel profiler_panel;

// TESTING ---- 

Texture2D octopus_texture = ResourceManager::load_texture("assets/textures/octopus.png", "Octopus");

// ------------

while (app.is_running())
{
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);
    {
        app.frame_begin();
        app.poll();
        float delta_time_s = app.get_delta_time();

        profiler.begin(Profiler::Stage::SdlInput);
        { // ~~ Input Events ~~

            //Settings: Shutdown app
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
                app.shutdown();

            //Settings: Toggle mouse capture
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
                app.get_window().toggle_mouse_capture();

            //Settings: Fullscreen
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f))
            {
                app.get_window().toggle_fullscreen();

                glm::ivec2 screen_size = app.get_window().get_size();
                printf("new screen size: %i %i \n", screen_size.x, screen_size.y);
                RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
            }

            // Input: Camera
            if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
                camera.InputKey(delta_time_s, CameraMovement::FORWARD);
            else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
                camera.InputKey(delta_time_s, CameraMovement::BACKWARD);

            if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
                camera.InputKey(delta_time_s, CameraMovement::LEFT);
            else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
                camera.InputKey(delta_time_s, CameraMovement::RIGHT);
                
            if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_SPACE))
                camera.InputKey(delta_time_s, CameraMovement::UP);
            else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT))
                camera.InputKey(delta_time_s, CameraMovement::DOWN);

            // Input: Mouse
            if (app.get_window().get_mouse_captured())
            {            
                glm::ivec2 rel_mouse = app.get_window().get_relative_mouse_state();
                camera.InputMouse(
                    static_cast<float>(rel_mouse.x), 
                    static_cast<float>(rel_mouse.y)
                );
            }
        }
        profiler.end(Profiler::Stage::SdlInput);

        profiler.begin(Profiler::Stage::GameTick);
        { // ~~ Game State Tick ~~
        
            //THIS IS FOR A FIXED GAME TICK
            //seconds_since_last_game_tick += delta_time_in_seconds;
            //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
            //{
            //    //Fixed update
            //    fixed_tick(SECONDS_PER_FIXED_TICK);
            //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
            //}
        }
        profiler.end(Profiler::Stage::GameTick);

        profiler.begin(Profiler::Stage::Render);
        { // ~~ Rendering ~~
            
            RenderCommand::set_clear_colour(glm::vec4(0.0, 0.482f, 0.655f, 1.0));
            RenderCommand::clear();

            glm::mat4 view_projection =  camera.get_view_projection_matrix();
            
            // //Draw skybox
            // glDepthMask(GL_FALSE);
            // skybox_shader.bind();
            // skybox_shader.set_mat4("view_projection", view_projection);
            // background.Bind();
            // glDrawArrays(GL_TRIANGLES, 0, 36);
            // glDepthMask(GL_TRUE);
            // background.Unbind();

            simple_renderer.update(delta_time_s, camera);

            //state.render(renderer, camera, app.get_window(), timer);
            //default_scene->on_update(delta_time_s);
                        
        }
        profiler.end(Profiler::Stage::Render);

        profiler.begin(Profiler::Stage::GuiLoop);
        { // ~~ GUI ~~

            app.gui_begin();

                // bool demo_window = true;
                // ImGui::ShowDemoWindow(&demo_window);

                // ImGui::Begin("Texture Test");
                // // Using a Child allow to fill all the space of the window.
                // ImGui::BeginChild("GameRender");
                // ImVec2 wsize = ImGui::GetWindowSize();
                // ImGui::Image((ImTextureID)tex.id, ImVec2(wsize.x, wsize.x * 9.0 / 16.0), ImVec2(0, 1), ImVec2(1, 0));
                // ImGui::Text("Yarr Harr I'm an octopus!");
                // ImGui::EndChild();
                // ImGui::End();

                ImGui::Begin("Entities");

                    //Camera
                    ImGui::Text("Camera Pos: %f %f %f", camera.Position.x, camera.Position.y, camera.Position.z );
                    ImGui::Text("Camera Pitch: %f", camera.Pitch);
                    ImGui::Text("Camera Yaw: %f", camera.Yaw);
                    
                ImGui::End();

                ImGui::Begin("Renderer", (bool*)1);

                    ImGui::Text("hi");

                    // if (ImGui::CollapsingHeader("Post-processing"))
                    // {
                    //     ImGui::Checkbox("SSAO", &renderer->GetPostProcessor()->SSAO);
                    //     ImGui::Checkbox("Bloom", &renderer->GetPostProcessor()->Bloom);
                    //     ImGui::Checkbox("Motion Blur", &renderer->GetPostProcessor()->MotionBlur);
                    //     //ImGui::Checkbox("SSR", &renderer->GetPostProcessor()->SSR);
                    //     //ImGui::Checkbox("TXAA", &renderer->GetPostProcessor()->TXAA);
                    //     ImGui::Checkbox("Vignette", &renderer->GetPostProcessor()->Vignette);
                    //     ImGui::Checkbox("Sepia", &renderer->GetPostProcessor()->Sepia);
                    // }

                ImGui::End();

                profiler_panel.draw(app, profiler, delta_time_s);

            app.gui_end();
        }
        profiler.end(Profiler::Stage::GuiLoop);

        profiler.begin(Profiler::Stage::FrameEnd);
        { // ~~ Frame End ~~
            app.frame_end(delta_time_s);
        }
        profiler.end(Profiler::Stage::FrameEnd);
    }
    profiler.end(Profiler::Stage::UpdateLoop);
}
return 0;
}