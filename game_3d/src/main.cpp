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
#include "engine/maths/random.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/triangle.hpp"
#include "engine/3d/renderer/renderer_pbr.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/3d/renderer/renderer_simple.hpp"
#include "engine/scene/background.hpp"
#include "engine/scene/scene_node.hpp"
#include "engine/resources/resource_manager.hpp"
#include "engine/tools/profiler.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

#include "panels/scene_hierarchy_panel.hpp"
#include "game_state.hpp"
using namespace game_3d;

//imguizmo testing / hack
#include <math.h>
#include <algorithm>
#include "ImGuizmo.h"

void EditTransform(const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
    static bool useSnap = false;
    static float snap[3] = { 1.f, 1.f, 1.f };
    static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
    static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
    static bool boundSizing = false;
    static bool boundSizingSnap = false;

    if (editTransformDecomposition)
    {
        if (ImGui::IsKeyPressed(90))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(69))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(82)) // r Key
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
        ImGui::InputFloat3("Tr", matrixTranslation);
        ImGui::InputFloat3("Rt", matrixRotation);
        ImGui::InputFloat3("Sc", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

        if (mCurrentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                mCurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                mCurrentGizmoMode = ImGuizmo::WORLD;
        }
        if (ImGui::IsKeyPressed(83))
            useSnap = !useSnap;
        ImGui::Checkbox("", &useSnap);
        ImGui::SameLine();

        switch (mCurrentGizmoOperation)
        {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &snap[0]);
            break;
        }
        ImGui::Checkbox("Bound Sizing", &boundSizing);
        if (boundSizing)
        {
            ImGui::PushID(3);
            ImGui::Checkbox("", &boundSizingSnap);
            ImGui::SameLine();
            ImGui::InputFloat3("Snap", boundsSnap);
            ImGui::PopID();
        }
    }
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
}

int main(int argc, char** argv)
{
    uint32_t width = 1280;
    uint32_t height = 720;
    Application app("Fighting Game!", width, height);
    app.set_fps_limit(60.0f);   
    //app.get_window().set_fullscreen(true);
    //app.remove_fps_limit();

    // ---- Game 

    FlyCamera camera {glm::vec3(0.0f, 0.0f, 3.0f)};
    camera.SetPerspective(glm::radians(65.0f), (float)width/(float)height, 0.1f, 100.0f);

    RandomState rnd;
    Profiler profiler;
    ProfilerPanel profiler_panel;
    RendererSimple simple_renderer( rnd );

    // ImGuiszmo::SetOrthographic(false);
    // ImGuizmo::ViewManipulate()

    // ---- random cubes

    int desired_cubes = 35;

    std::vector<glm::vec3> cube_pos;
    glm::vec3 rand_pos { 0.0f, 0.0f, 0.0f};
    for(int i = 0; i < desired_cubes; i++)
    {
        rand_pos.x = rand_det_s(rnd.rng, -10.0f, 10.0f);
        rand_pos.y = rand_det_s(rnd.rng, -10.0f, 10.0f);
        rand_pos.z = rand_det_s(rnd.rng, -10.0f, 10.0f);
        cube_pos.push_back( rand_pos );
    }

    // ---- App

    while (app.is_running())
    {
        profiler.new_frame();       
        profiler.begin(Profiler::Stage::UpdateLoop);

        app.frame_begin();
        
        float delta_time_s = app.get_delta_time();

        profiler.begin(Profiler::Stage::SdlInput);
        app.poll(); //input events

        //Settings: Shutdown app
        if ( app.get_input().get_key_down( SDL_KeyCode::SDLK_END ) )
            app.shutdown();

        //Settings: Toggle mouse capture
        if ( app.get_input().get_key_down( SDL_KeyCode::SDLK_m ) )
            app.get_window().toggle_mouse_capture();

        //Settings: Fullscreen
        if ( app.get_input().get_key_down( SDL_KeyCode::SDLK_f ) )
        {
            app.get_window().toggle_fullscreen();

            glm::ivec2 screen_size = app.get_window().get_size();
            RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
        }

        // Input: Camera

        if (app.get_input().get_key_held( SDL_Scancode::SDL_SCANCODE_W ) )
            camera.InputKey( delta_time_s, CameraMovement::FORWARD );
        else if ( app.get_input().get_key_held( SDL_Scancode::SDL_SCANCODE_S ) )
            camera.InputKey( delta_time_s, CameraMovement::BACKWARD );

        if ( app.get_input().get_key_held( SDL_Scancode::SDL_SCANCODE_A ) )
            camera.InputKey( delta_time_s, CameraMovement::LEFT );
        else if ( app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D ) )
            camera.InputKey( delta_time_s, CameraMovement::RIGHT );
            
        if ( app.get_input().get_key_held( SDL_Scancode::SDL_SCANCODE_SPACE ) )
            camera.InputKey( delta_time_s, CameraMovement::UP );
        else if ( app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT ) )
            camera.InputKey( delta_time_s, CameraMovement::DOWN );

        // Input: Mouse
        if ( app.get_window().get_mouse_captured() )
        {            
            glm::ivec2 rel_mouse = app.get_window().get_relative_mouse_position();
            camera.InputMouse(
                static_cast<float>( rel_mouse.x ) , 
                static_cast<float>( rel_mouse.y ) );
        } 
        //Mouse not captured by window
        else
        {
            //if mouse is clicked
            if( app.get_input().get_mouse_lmb_held() )
            {
                //This is the lazy implementation of object picking!
                //our ray origin is the camera position
                glm::vec3 ray_origin = camera.Position;

                //step 1: get mouse pos
                //range [0:width, height:0]
                glm::ivec2 abs_mouse = app.get_window().get_mouse_position();

                //printf("mouse x: %i y: %i \n", abs_mouse.x, abs_mouse.y);

                //step 2: convert mouse position in to 3d normalized device coordinates
                //range [-1:1, -1:1, -1:1]
                float x = (2.0f * abs_mouse.x) / width - 1.0f;
                float y = 1.0f - (2.0f * abs_mouse.y) /  height;
                glm::vec2 normalized_device_coords {x, y};

                //step 3: convert normalized device coords to homogenous clip coords
                //range [-1:1, -1:1, -1:1, -1:1]
                //note: ray points in the -z direction
                glm::vec4 ray_clip {normalized_device_coords.x, normalized_device_coords.y, -1.0, 1.0};

                //Convert homogenous clip coords to  eye camera coordinates
                //range [-x:x, -y:y, -z:z, -w:w]
                glm::vec4 eye_ray = glm::inverse(camera.Projection) * ray_clip;
                eye_ray.z = -1.0f;
                eye_ray.w = 0.0f;

                //step 5: convert eye camera coordinates to world coordinates
                //range [-x:x, -y:y, -z:z, -w:w]
                glm::vec3 ray_world = glm::vec3(glm::inverse(camera.View) * eye_ray);
                ray_world = glm::normalize(ray_world);

                printf("ray x: %f y: %f z: %f \n", ray_world.x, ray_world.y, ray_world.z);
            }
        }

        profiler.end(Profiler::Stage::SdlInput);
        profiler.begin(Profiler::Stage::GameTick);

        // ---- Game State Tick 
        
        //THIS IS FOR A FIXED GAME TICK
        //seconds_since_last_game_tick += delta_time_in_seconds;
        //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
        //{
        //    //Fixed update
        //    fixed_tick(SECONDS_PER_FIXED_TICK);
        //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
        //}
        
        profiler.end(Profiler::Stage::GameTick);
        profiler.begin(Profiler::Stage::Render);
                
        // ---- Rendering 

        simple_renderer.update( 
            delta_time_s, 
            camera,
            rnd, 
            cube_pos,
            app.get_window().get_size() );                        

        profiler.end(Profiler::Stage::Render);
        profiler.begin(Profiler::Stage::GuiLoop);

        // ---- GUI 

        app.gui_begin();

            // bool demo_window = true;
            // ImGui::ShowDemoWindow(&demo_window);

            ImGui::Begin("Depth Texture");

                // Using a Child allow to fill all the space of the window.
                ImGui::BeginChild("Depth Texture");
                ImVec2 wsize = ImGui::GetWindowSize();
                ImGui::Image((ImTextureID)simple_renderer.depthmap_, ImVec2(wsize.x, wsize.y), ImVec2(0, 1), ImVec2(1, 0));
                //ImGui::Text("Depth Texture being rendererd");
                ImGui::EndChild();

            ImGui::End();

            ImGui::Begin("Camera");

                //Camera
                ImGui::Text("Camera Pos: %f %f %f", camera.Position.x, camera.Position.y, camera.Position.z );
                ImGui::Text("Camera Pitch: %f", camera.Pitch);
                ImGui::Text("Camera Yaw: %f", camera.Yaw);
                
            ImGui::End();

            ImGui::Begin("Renderer", (bool*)1);

                ImGui::Text("Draw calls: %i", simple_renderer.get_draw_calls());

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

            profiler_panel.draw( app, profiler, delta_time_s );

        app.gui_end();

        profiler.end(Profiler::Stage::GuiLoop);
        profiler.begin(Profiler::Stage::FrameEnd);

        app.frame_end(delta_time_s);

        profiler.end(Profiler::Stage::FrameEnd);
        profiler.end(Profiler::Stage::UpdateLoop);
    }

    return 0;
}