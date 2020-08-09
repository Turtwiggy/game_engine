
#include "graphics/opengl/renderers/renderer_ray_traced.h"

#include "graphics/opengl/util/util_functions.h"

namespace fightingengine {

    struct RayTracedData {
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

        //input for debugging
        bool is_c_held = false;

        uint32_t draw_calls = 0;
    };
    static RayTracedData s_Data;


    void RendererRayTraced::init(int screen_width, int screen_height)
    {
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
            .attach_shader("assets/shaders/raytraced/compute/random.glsl", GL_COMPUTE_SHADER)
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
    }

    void RendererRayTraced::draw_pass(draw_scene_desc& desc, const GameState& state)
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
            state.cornel_box->model->draw(s_Data.geometry_shader, s_Data.draw_calls);
        }

        //Update scene's triangle description
        {
            if (s_Data.refresh_ssbo) {

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
                        v1.colour = triangles_in_scene[i].p1.Colour.colour;

                        ComputeShaderVertex v2;
                        v2.pos = glm::vec4(triangles_in_scene[i].p2.Position, 1.0);
                        v2.nml = glm::vec4(triangles_in_scene[i].p2.Normal, 1.0);
                        v2.tex = glm::vec4(triangles_in_scene[i].p2.TexCoords, 1.0, 1.0);
                        v2.colour = triangles_in_scene[i].p2.Colour.colour;

                        ComputeShaderVertex v3;
                        v3.pos = glm::vec4(triangles_in_scene[i].p3.Position, 1.0);
                        v3.nml = glm::vec4(triangles_in_scene[i].p3.Normal, 1.0);
                        v3.tex = glm::vec4(triangles_in_scene[i].p3.TexCoords, 1.0, 1.0);
                        v3.colour = triangles_in_scene[i].p3.Colour.colour;

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

        if (s_Data.is_c_held) {

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

            s_Data.compute_shader.setFloat("time", state.time);

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
            s_Data.compute_shader.setFloat("phongExponent", 128.0f);
            s_Data.compute_shader.setFloat("specularFactor", desc.exposure);

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

        if (s_Data.is_c_held) {
            glBindTexture(GL_TEXTURE_2D, s_Data.out_texture);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, s_Data.g_albedo_spec);
        }
        renderQuad();

        ImGui::Begin("RayRenderer Profiler");
        ImGui::Text("Draw Calls: %i", s_Data.draw_calls);
        ImGui::End();
        s_Data.is_c_held = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_C));

        s_Data.draw_calls = 0;
    }

    void RendererRayTraced::resize(int width, int height)
    {
        printf("resize raytraced");
    }
}
