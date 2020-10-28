#define _CRT_SECURE_NO_WARNINGS
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

//c++ standard library headers
#include <memory>
#include <vector>
#include <string_view>

//other library headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#undef main //thanks SDL2?

//your project headers
#include "engine/core/application.hpp"
#include "engine/3d/renderer/renderer_pbr.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/3d/renderer/renderer_simple.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/triangle.hpp"
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

    FlyCamera camera {glm::vec3(0.0f, 2.0f, 4.0f)};
    camera.SetPerspective(glm::radians(90.0f), (float)width/(float)height, 0.1f, 100.0f);

    //Create world (for now, just spheres)
    //std::vector<Sphere> world = create_world();
    
    //ModelManager model_manager;
    GameState state;
    RandomState rnd;

    //RendererRayTraced renderer(width, height);
    //RendererPBR pbr_renderer;
    RendererSimple simple_renderer;
    RenderCommand::init();

    TextureCube background = ResourceManager::load_texture_cube(
        "assets/skybox/skybox-default/", 
        "default-skybox");
    Shader skybox_shader = Shader()
        .attach_shader("assets/shaders/skybox/skybox.vert", OpenGLShaderTypes::VERTEX)
        .attach_shader("assets/shaders/skybox/skybox.frag", OpenGLShaderTypes::FRAGMENT)
        .build_program();

    Profiler profiler;
    ProfilerPanel profiler_panel;

    //Fix mouse lurch issue - probably should move this in to a class or something
    bool new_grab = false;
    //Testing texture
    Texture2D tex = ResourceManager::load_texture("assets/textures/octopus.png", "Octopus");

    float timer = 0.0f;
    while (app.is_running())
    {
        profiler.new_frame();
        profiler.begin(Profiler::Stage::UpdateLoop);
        app.frame_begin();
        app.poll();

        float delta_time_s = app.get_delta_time();
        timer += delta_time_s;

        { // ~~ Input Events ~~
            profiler.begin(Profiler::Stage::SdlInput);

            //Shutdown app
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
                app.shutdown();

            //Toggle mouse capture
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            {
                new_grab = app.get_window().IsInputGrabbed();

                if(!new_grab)
                {
                    app.get_window().CaptureMouse();
                    new_grab = true;
                }
                else
                {
                    app.get_window().ReleaseMouse();
                    new_grab = false;
                }

                printf("(App) Mouse grabbed? : %d \n", new_grab);
            }

            //Fullscreen
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f))
            {
                if(app.get_window().IsFullscreen())
                    app.get_window().SetFullscreen(false);
                else
                    app.get_window().SetFullscreen(true);

                int width, height;
                app.get_window().GetSize(width, height);
                std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;

                RenderCommand::set_viewport(0, 0, width, height);
            }

            //TODO EVENT: resized window
            // if (app.get_event().window_resized)
            //    renderer.resize(width, height);

            // ~~ Input: Camera ~~
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

            // ~~ Input: Mouse ~~
            if (app.get_window().IsInputGrabbed())
            {                
                int x, y;
                if(new_grab)
                {
                    SDL_GetRelativeMouseState(&x, &y); //throw away this huge jumping value
                    new_grab = false;
                }
                else
                {
                    SDL_GetRelativeMouseState(&x, &y);
                    //printf("relative movement: %i %i \n", x, y);
                    camera.InputMouse((float)x, (float)y);
                }
            }

        profiler.end(Profiler::Stage::SdlInput);
        }

        { // ~~ Game State Tick ~~
            profiler.begin(Profiler::Stage::GameTick);
        
            //THIS IS FOR A FIXED GAME TICK
            //seconds_since_last_game_tick += delta_time_in_seconds;
            //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
            //{
            //    //Fixed update
            //    fixed_tick(SECONDS_PER_FIXED_TICK);
            //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
            //}

            profiler.end(Profiler::Stage::GameTick);
        }

        { // ~~ Rendering ~~
            profiler.begin(Profiler::Stage::Render);
            
            RenderCommand::set_clear_colour(glm::vec4(0.0, 0.482f, 0.655f, 1.0));
            RenderCommand::clear();

            glm::mat4 view_projection =  camera.get_view_projection_matrix();
            
            //Draw skybox
            glDepthMask(GL_FALSE);
            skybox_shader.bind();
            skybox_shader.set_mat4("view_projection", view_projection);
            background.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);

            simple_renderer.update(delta_time_s, camera);

            //state.render(renderer, camera, app.get_window(), timer);
            //default_scene->on_update(delta_time_s);
                        
            profiler.end(Profiler::Stage::Render);
        }

        { // ~~ GUI ~~
            profiler.begin(Profiler::Stage::GuiLoop);

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

            ImGui::Text("Camera Pos: %f %f %f", 
                            camera.Position.x, 
                            camera.Position.y, 
                            camera.Position.z );
            ImGui::Text("Camera Pitch: %f", camera.Pitch);
            ImGui::Text("Camera Yaw: %f", camera.Yaw);

            ImGui::End();

            scene_panel.on_imgui_render();
            profiler_panel.draw(app, profiler, delta_time_s);

            app.gui_end();

            profiler.end(Profiler::Stage::GuiLoop);
        }

        profiler.begin(Profiler::Stage::FrameEnd);

        app.frame_end(delta_time_s);

        profiler.end(Profiler::Stage::FrameEnd);
        profiler.end(Profiler::Stage::UpdateLoop);
    }

    return 0;
}