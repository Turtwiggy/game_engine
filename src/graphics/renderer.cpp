#include "graphics/renderer.h"

#include "graphics/render_command.h"
#include "window/game_window.h"
#include "3d/fg_texture.hpp"
#include "util/util_functions.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif
#include "stb_image.h"
#include "imgui.h"
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

#include <vector>
#include <memory>

namespace fightinggame
{
    struct ComputeShaderVertex {
        glm::vec4 pos;
        glm::vec4 nml;
        glm::vec4 tex;
    };
    struct ComputeShaderTriangle {
        ComputeShaderVertex p1;
        ComputeShaderVertex p2;
        ComputeShaderVertex p3;
    };

    struct RenderData
    {
        //textures
        unsigned int wood_texture;
        unsigned int second_texture;

        //Geometry pass
        unsigned int g_buffer;
        unsigned int  g_position, g_normal, g_albedo_spec;
        Shader geometry_shader;

        //RayTracing
        unsigned int ray_fbo;
        unsigned int out_texture;
        Shader compute_shader;
        unsigned int compute_shader_workgroup_x;
        unsigned int compute_shader_workgroup_y;
        int compute_normal_binding;
        int compute_out_tex_binding;
        Shader quad_shader;

        bool refresh_ssbo = false;
        unsigned int ssbo;
        unsigned int ssbo_binding;

        unsigned int max_triangles = 100;
        unsigned int set_triangles = 0;
        std::vector<ComputeShaderTriangle> triangles;

        Renderer::Statistics stats;
    };
    static RenderData s_Data;

    void Renderer::init_renderer(int screen_width, int screen_height)
    {
        // load textures
        // -------------
        s_Data.wood_texture = TextureFromFile("BambooWall_1K_albedo.jpg", "assets/textures/Bamboo", false);
        s_Data.second_texture = TextureFromFile("container.jpg", "assets/textures", false);

        // A quad shader to render the full-screen quad VAO with the framebuffer as texture
        // --------------------------------------------------------------------------------
        Shader quad_shader = Shader()
                                .attach_shader("assets/shaders/raytraced/example.vert", GL_VERTEX_SHADER)
                                .attach_shader("assets/shaders/raytraced/example.frag", GL_FRAGMENT_SHADER)
                                .build_program();
        quad_shader.use();
        quad_shader.setInt("tex", 0);
        s_Data.quad_shader = quad_shader;
        quad_shader.unbind();

        Shader geometry_shader = Shader()
                                .attach_shader("assets/shaders/raytraced/geometry.vert", GL_VERTEX_SHADER)
                                .attach_shader("assets/shaders/raytraced/geometry.frag", GL_FRAGMENT_SHADER)
                                .build_program();
        geometry_shader.use();
        s_Data.geometry_shader = geometry_shader;
        geometry_shader.unbind();

        // configure g-buffer for intial render pass
        // -----------------------------------------
        unsigned int gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        unsigned int gPosition, gNormal, gAlbedoSpec;
        {
            // position color buffer
            glGenTextures(1, &gPosition);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
            // normal color buffer
            glGenTextures(1, &gNormal);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
            // color + specular color buffer
            glGenTextures(1, &gAlbedoSpec);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
            // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
            unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, attachments);
            // create and attach depth buffer (renderbuffer)
            unsigned int rboDepth;
            glGenRenderbuffers(1, &rboDepth);
            glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
            // finally check if framebuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        s_Data.g_buffer = gBuffer;
        s_Data.g_position = gPosition;
        s_Data.g_normal = gNormal;
        s_Data.g_albedo_spec = gAlbedoSpec;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Ray tracing texture that compute shader writes to
        // -------------------------------------------------
        unsigned int rayFBO;
        glGenFramebuffers(1, &rayFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, rayFBO);
        unsigned int rayTexture;
        {
            // position color buffer
            glGenTextures(1, &rayTexture);
            glBindTexture(GL_TEXTURE_2D, rayTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rayTexture, 0);
            // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
            unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, attachments);
            // finally check if framebuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        s_Data.ray_fbo = rayFBO;
        s_Data.out_texture = rayTexture;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Ray tracing compute shader
        // ----------------------------
        Shader compute_shader = Shader()
            //.attach_shader("assets/shaders/raytraced/compute/random.glsl", GL_COMPUTE_SHADER)
            .attach_shader("assets/shaders/raytraced/compute/raytraced.glsl", GL_COMPUTE_SHADER)
            .build_program();
        compute_shader.use();
        //CHECK_OPENGL_ERROR(0);

        int workgroup_size[3];
        glGetProgramiv(compute_shader.ID, GL_COMPUTE_WORK_GROUP_SIZE, workgroup_size);
        unsigned int work_group_size_x = workgroup_size[0];
        unsigned int work_group_size_y = workgroup_size[1];
        unsigned int work_group_size_z = workgroup_size[2];
        //link data
        s_Data.compute_shader = compute_shader;
        s_Data.compute_shader_workgroup_x = work_group_size_x;
        s_Data.compute_shader_workgroup_y = work_group_size_y;
        s_Data.compute_out_tex_binding = compute_shader.get_uniform_binding_location("outTexture");
        s_Data.compute_normal_binding = compute_shader.get_uniform_binding_location("normalTexture");
        s_Data.ssbo_binding = compute_shader.get_buffer_binding_location("bufferData");

        // Data to bind to GPU
        // -------------------
        //std::vector<ComputeShaderTriangle> triangles;
        //triangles.resize(s_Data.max_triangles);
        //printf("Size of init triangles: %i \n", sizeof(triangles));

        /*glm::vec4 data = glm::vec4(1.0, 0.0, 0.0, 1.0);*/

        // Ray tracing SSBO
        // ----------------
        printf("bind slot: %i \n", s_Data.ssbo_binding);

        GLuint SSBO;
        glGenBuffers(1, &SSBO);
        CHECK_OPENGL_ERROR(0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        CHECK_OPENGL_ERROR(1)

        //glBufferData(GL_SHADER_STORAGE_BUFFER, s_Data.data.size() * sizeof(glm::vec4), &s_Data.data[0], GL_DYNAMIC_DRAW);
        //initialize empty ssbo
        glBufferData(GL_SHADER_STORAGE_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
        CHECK_OPENGL_ERROR(2);
        s_Data.refresh_ssbo = true;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, SSBO);
        CHECK_OPENGL_ERROR(3);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        CHECK_OPENGL_ERROR(4);

        s_Data.ssbo = SSBO;

        //GPU Info
        printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
        int params[1];
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
        printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
        //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
        //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
        //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96

        //Resources
        //https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
        //https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
        //https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
        //https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

        // draw as wireframe
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void Renderer::draw_pass(draw_scene_desc& desc, const GameState& state)
    {
        int width, height = 0;
        GameWindow& window = desc.window;
        window.GetSize(width, height);
        glm::mat4 view_projection = desc.camera.get_view_projection_matrix(width, height);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.g_buffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Render a cube
        {
            s_Data.geometry_shader.use();
            s_Data.geometry_shader.setMat4("view_projection", view_projection);

            //either this or textures
            //s_Data.geometry_shader.setVec3("diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
            s_Data.geometry_shader.setFloat("specular", 1.0f);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
            model = glm::scale(model, glm::vec3(1.0f));
            s_Data.geometry_shader.setMat4("model", model);
            state.cornel_box->model->draw(s_Data.geometry_shader, s_Data.stats.DrawCalls);
        }

        //Update scene's triangle description
        {
            if ( s_Data.refresh_ssbo ) {

                std::vector<FGTriangle> triangles_in_scene;
                std::vector<FGTriangle> triangles_in_cornell = state.cornel_box->model->get_all_triangles_in_meshes();
                triangles_in_scene = triangles_in_cornell;

                int triangles_in_scene_size = triangles_in_scene.size();

                //Check max triangles
                if (triangles_in_scene_size > s_Data.max_triangles)
                {
                    printf("too many triangles! handle this scenario");
                    return;
                }

                //Check ssbo size
                if (s_Data.set_triangles != triangles_in_scene_size)
                {
                    printf("Updating SSBO triangles with: %i triangles \n", triangles_in_scene_size);

                    //refresh triangle data
                    s_Data.triangles.clear();
                    s_Data.triangles.resize(triangles_in_scene_size);

                    //convert FGTriangle to ComputeShaderTriangle
                    for (int i = 0; i < triangles_in_scene_size; i++)
                    {
                        ComputeShaderVertex v1;
                        v1.pos = glm::vec4(triangles_in_scene[i].p1.Position, 1.0);
                        v1.nml = glm::vec4(triangles_in_scene[i].p1.Normal, 1.0);
                        v1.tex = glm::vec4(triangles_in_scene[i].p1.TexCoords, 1.0, 1.0);

                        ComputeShaderVertex v2;
                        v2.pos = glm::vec4(triangles_in_scene[i].p2.Position, 1.0);
                        v2.nml = glm::vec4(triangles_in_scene[i].p2.Normal, 1.0);
                        v2.tex = glm::vec4(triangles_in_scene[i].p2.TexCoords, 1.0, 1.0);

                        ComputeShaderVertex v3;
                        v3.pos = glm::vec4(triangles_in_scene[i].p3.Position, 1.0);
                        v3.nml = glm::vec4(triangles_in_scene[i].p3.Normal, 1.0);
                        v3.tex = glm::vec4(triangles_in_scene[i].p3.TexCoords, 1.0, 1.0);

                        s_Data.triangles[i].p1 = v1;
                        s_Data.triangles[i].p2 = v2;
                        s_Data.triangles[i].p3 = v3;
                    }

                    //upload data to ssbo when triangle size changes
                    //note: if the triangles vertices change, this wont update the ssbo currently
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, triangles_in_scene_size * sizeof(ComputeShaderTriangle), &s_Data.triangles[0], GL_STATIC_DRAW);
                    //glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), (unsigned int)all_balls.size());
                    //glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), all_balls.size() * sizeof(Ball), &(all_balls[0]));

                    s_Data.set_triangles = triangles_in_scene_size;
                }

                s_Data.refresh_ssbo = false;
            }
        }

        if (desc.hdr) {

            // 2. Lighting pass: Raytracing
            // ------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, s_Data.ray_fbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            s_Data.compute_shader.use();

            //Set viewing frustrum corner rays in shader
            s_Data.compute_shader.setVec3("eye", desc.camera.Position);
            glm::vec3 eye_ray;
            eye_ray = desc.camera.get_eye_ray(-1, -1, width, height);
            s_Data.compute_shader.setVec3("ray00", eye_ray);
            eye_ray = desc.camera.get_eye_ray(-1, 1, width, height);
            s_Data.compute_shader.setVec3("ray01", eye_ray);
            eye_ray = desc.camera.get_eye_ray(1, -1, width, height);
            s_Data.compute_shader.setVec3("ray10", eye_ray);
            eye_ray = desc.camera.get_eye_ray(1, 1, width, height);
            s_Data.compute_shader.setVec3("ray11", eye_ray);
            CHECK_OPENGL_ERROR(5);

            // Bind framebuffer texture as writable image in the shader.
            glBindImageTexture(s_Data.compute_normal_binding, s_Data.g_normal, 0, false, 0, GL_READ_ONLY, GL_RGBA16F);
            glBindImageTexture(s_Data.compute_out_tex_binding, s_Data.out_texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA16F);
            CHECK_OPENGL_ERROR(6);

            s_Data.compute_shader.set_compute_buffer_bind_location("bufferData");
            CHECK_OPENGL_ERROR(7);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, s_Data.ssbo);
            CHECK_OPENGL_ERROR(9);

            //set the ssbo size in a uniform
            s_Data.compute_shader.setInt("set_triangles", s_Data.set_triangles);

            // Compute appropriate invocation dimension
            int worksizeX = next_power_of_two(width);
            int worksizeY = next_power_of_two(height);
            if (s_Data.compute_shader_workgroup_x == 0 || s_Data.compute_shader_workgroup_y == 0)
            {
                std::cout << "failed to load your compute shader!";
                return;
            }

            /* Invoke the compute shader. */
            // This function takes as argument the number of work groups in each of the
            // three possible dimensions. The number of work groups is NOT the number of
            // global work items in each dimension, but is divided by the work group size
            // that the shader specified in that layout declaration.
            glDispatchCompute(
                worksizeX / s_Data.compute_shader_workgroup_x,
                worksizeY / s_Data.compute_shader_workgroup_y
                , 1);

            //Synchronize all writes to the framebuffer image
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // Reset bindings
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, 0);
            glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
            glUseProgram(0);
        }

        // 3. Normal drawing pass
        // -------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s_Data.quad_shader.use();
        glActiveTexture(GL_TEXTURE0);

        if (desc.hdr) {
            glBindTexture(GL_TEXTURE_2D, s_Data.out_texture);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, s_Data.g_albedo_spec);
        }
        renderQuad();

        //Cornell
       //{
       //    s_Data.object_shader.use();
       //    s_Data.object_shader.setMat4("view_projection", view_projection);
       //    //cube's material properties
       //    s_Data.object_shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
       //    s_Data.object_shader.setFloat("material.shininess", 32.0f);
       //    //flat lighting
       //    glm::vec3 light_direction = glm::vec3(-0.2, -1.0, -0.3);
       //    s_Data.object_shader.setVec3("light.direction", light_direction);
       //    s_Data.object_shader.setVec3("light.ambient", glm::vec3(0.2, 0.2, 0.2));
       //    s_Data.object_shader.setVec3("light.diffuse", glm::vec3(0.5, 0.5, 0.5)); // darken diffuse light a bit
       //    s_Data.object_shader.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));

       //    model = glm::mat4(1.0f);
       //    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
       //    model = glm::scale(model, glm::vec3(0.5f));
       //    s_Data.object_shader.setMat4("model", model);
       //    state.cornel_box->model->draw(s_Data.object_shader, s_Data.stats.DrawCalls);
       //}

        ImGui::Begin("Renderer Profiler");
        ImGui::Text("Draw Calls: %i", s_Data.stats.DrawCalls);
        ImGui::End();
        s_Data.stats.DrawCalls = 0;
    }

    void Renderer::new_frame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        ImGui::SetCurrentContext(_imgui);
        ImGui::NewFrame();
    }

    void Renderer::end_frame(SDL_Window* window)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }

    void Renderer::shutdown()
    {
        ImGui::SetCurrentContext(_imgui);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(get_gl_context());
    }

    void Renderer::init_opengl_and_imgui(const GameWindow& window)
    {
        //OpenGL
        gl_context = SDL_GL_CreateContext(window.GetHandle());
        SDL_GL_MakeCurrent(window.GetHandle(), gl_context);

        int width, height;
        window.GetSize(width, height);
        RenderCommand::set_viewport(0, 0, width, height);

        if (gl_context == NULL)
        {
            printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
            throw std::runtime_error("Failed creating SDL2 window: " + std::string(SDL_GetError()));
        }
        else
        {
            //Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum glewError = glewInit();
            if (glewError != GLEW_OK)
            {
                printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
                throw std::runtime_error("Error initializing GLEW! " + std::string(SDL_GetError()));
            }

            //Check OpenGL
            GLenum error;
            while ((error = glGetError()) != GL_NO_ERROR)
            {
                printf("ERROR GLEW: %s\n", gl_error_to_string(error));
            }
        }

        //Setup ImGui
        IMGUI_CHECKVERSION();
        _imgui = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        std::string glsl_version = "#version 430";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        //4, 3 because that's when compute shaders were introduced

        // setup platform/renderer bindings
        ImGui_ImplSDL2_InitForOpenGL(window.GetHandle(), gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        //configure opengl state
        RenderCommand::init();
    }

    void Renderer::screen_size_changed(unsigned int width, unsigned int height)
    {
        printf("renderer callback! %i %i ", width, height);
    }

    // renderQuad() renders a 1x1 XY quad in NDC
    // -----------------------------------------
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void Renderer::renderQuad()
    {
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}

//unsigned int Renderer::hdr_fbo()
//{
//    // configure floating point framebuffer
//    // ------------------------------------
//    unsigned int hdrFBO;
//    glGenFramebuffers(1, &hdrFBO);
//    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
//    return hdrFBO;
//}
//
//std::array<unsigned int, 2> Renderer::hdr_colour_buffer(int width, int height, unsigned int hdr_fbo)
//{
//    // create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
//    unsigned int colorBuffers[2];
//    glGenTextures(2, colorBuffers);
//    for (unsigned int i = 0; i < 2; i++)
//    {
//        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        // attach texture to framebuffer
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
//    }
//    // create and attach depth buffer (renderbuffer)
//    unsigned int rboDepth;
//    glGenRenderbuffers(1, &rboDepth);
//    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
//    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
//    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
//    glDrawBuffers(2, attachments);
//    // finally check if framebuffer is complete
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "Framebuffer not complete!" << std::endl;
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    std::array<unsigned int, 2> color_buffer_array;
//    color_buffer_array[0] = colorBuffers[0];
//    color_buffer_array[1] = colorBuffers[1];
//    return color_buffer_array;
//}

//void hdr_bloom_init()
//{
//    //// configure (floating point) framebuffers
//    //// ---------------------------------------
//    //s_Data.hdr_fbo = hdr_fbo();
//    //s_Data.hdr_colour_buffers = hdr_colour_buffer(screen_width, screen_height, s_Data.hdr_fbo);
//
//    //// ping-pong-framebuffer for blurring
//    //unsigned int pingpongFBO[2];
//    //unsigned int pingpongColorbuffers[2];
//    //glGenFramebuffers(2, pingpongFBO);
//    //glGenTextures(2, pingpongColorbuffers);
//    //for (unsigned int i = 0; i < 2; i++)
//    //{
//    //    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
//    //    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
//    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
//    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
//    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
//    //    // also check if framebuffers are complete (no need for depth buffer)
//    //    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//    //        std::cout << "Framebuffer not complete!" << std::endl;
//    //}
//    //s_Data.pingpong_fbo[0] = pingpongFBO[0];
//    //s_Data.pingpong_fbo[1] = pingpongFBO[1];
//    //s_Data.pingpong_colour_buffers[0] = pingpongColorbuffers[0];
//    //s_Data.pingpong_colour_buffers[1] = pingpongColorbuffers[1];
//
//    //// lighting info
//    //// -------------
//    //// positions
//    //std::vector<glm::vec3> lightPositions;
//    //lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
//    //lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
//    //lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
//    //lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
//    //s_Data.light_positions = lightPositions;
//    //// colors
//    //std::vector<glm::vec3> lightColors;
//    //lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
//    //lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
//    //lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
//    //lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
//    //s_Data.light_colours = lightColors;
//
//    //// shader configuration
//    //// --------------------
//    //s_Data.object_shader.use();
//    //s_Data.object_shader.setInt("texture_diffuse1", 0);
//    //s_Data.blur_shader.use();
//    //s_Data.blur_shader.setInt("image", 0);
//    //s_Data.hdr_bloom_final_shader.use();
//    //s_Data.hdr_bloom_final_shader.setInt("scene", 0);
//    //s_Data.hdr_bloom_final_shader.setInt("bloomBlur", 1);
//}

//void hdr_bloom_draw()
//{
//    //   // 1. render scene into floating point framebuffer
//    //// -----------------------------------------------
//    //   glBindFramebuffer(GL_FRAMEBUFFER, s_Data.hdr_fbo);
//    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    //   s_Data.object_shader.use();
//    //   s_Data.object_shader.setMat4("view_projection", view_projection);
//    //   s_Data.object_shader.setVec3("viewPos", desc.camera.Position);
//    //   glActiveTexture(GL_TEXTURE0);
//    //   glBindTexture(GL_TEXTURE_2D, s_Data.wood_texture);
//    //   // set lighting uniforms
//    //   for (unsigned int i = 0; i < s_Data.light_positions.size(); i++)
//    //   {
//    //       s_Data.object_shader.setVec3("lights[" + std::to_string(i) + "].Position", s_Data.light_positions[i]);
//    //       s_Data.object_shader.setVec3("lights[" + std::to_string(i) + "].Color", s_Data.light_colours[i]);
//    //   }
//
//    //   // create one large cube that acts as the floor
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
//    //   model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//
//    //   // then create multiple cubes as the scenery
//    //   glBindTexture(GL_TEXTURE_2D, s_Data.second_texture);
//
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
//    //   model = glm::scale(model, glm::vec3(0.5f));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   //state.cubes[0]->model->draw(s_Data.object_shader, s_Data.stats.DrawCalls);
//    //   renderCube(s_Data.stats.DrawCalls);
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
//    //   model = glm::scale(model, glm::vec3(0.5f));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
//    //   model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
//    //   model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//    //   model = glm::scale(model, glm::vec3(1.25));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
//    //   model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//    //   model = glm::mat4(1.0f);
//    //   model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
//    //   model = glm::scale(model, glm::vec3(0.5f));
//    //   s_Data.object_shader.setMat4("model", model);
//    //   renderCube(s_Data.stats.DrawCalls);
//
//
//
//    //   // show all the light sources as bright cubes
//    //   s_Data.light_positions[0] = state.cubes[0]->transform.Position;
//
//    //   s_Data.light_shader.use();
//    //   s_Data.light_shader.setMat4("view_projection", view_projection);
//    //   for (unsigned int i = 0; i < s_Data.light_positions.size(); i++)
//    //   {
//    //       model = glm::mat4(1.0f);
//    //       model = glm::translate(model, glm::vec3(s_Data.light_positions[i]));
//    //       model = glm::scale(model, glm::vec3(0.25f));
//    //       s_Data.light_shader.setMat4("model", model);
//    //       s_Data.light_shader.setVec3("lightColor", s_Data.light_colours[i]);
//    //       renderCube(s_Data.stats.DrawCalls);
//    //   }
//    //   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    //   // 2. blur bright fragments with two-pass Gaussian Blur 
//    //   // --------------------------------------------------
//    //   bool horizontal = true, first_iteration = true;
//    //   unsigned int amount = 10;
//    //   s_Data.blur_shader.use();
//    //   s_Data.blur_shader.setInt("blur_x", amount / 2);
//    //   s_Data.blur_shader.setInt("blur_y", amount / 2);
//    //   for (unsigned int i = 0; i < amount; i++)
//    //   {
//    //       glBindFramebuffer(GL_FRAMEBUFFER, s_Data.pingpong_fbo[horizontal]);
//    //       s_Data.blur_shader.setInt("horizontal", horizontal);
//    //       glBindTexture(GL_TEXTURE_2D, first_iteration ? s_Data.hdr_colour_buffers[1] : s_Data.pingpong_colour_buffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
//    //       renderQuad();
//    //       horizontal = !horizontal;
//    //       if (first_iteration)
//    //           first_iteration = false;
//    //   }
//    //   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    //   // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
//    //   // --------------------------------------------------------------------------------------------------------------------------
//    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    //   s_Data.hdr_bloom_final_shader.use();
//    //   glActiveTexture(GL_TEXTURE0);
//    //   glBindTexture(GL_TEXTURE_2D, s_Data.hdr_colour_buffers[0]);
//    //   glActiveTexture(GL_TEXTURE1);
//    //   glBindTexture(GL_TEXTURE_2D, s_Data.pingpong_colour_buffers[!horizontal]);
//    //   s_Data.hdr_bloom_final_shader.setInt("bloom", desc.hdr);
//    //   s_Data.hdr_bloom_final_shader.setFloat("exposure", desc.exposure);
//    //   renderQuad();
//    //   //std::cout << "bloom: " << (desc.hdr ? "on" : "off") << "| exposure: " << desc.exposure << std::endl;
//}
//
