
#include "imgui_setup.h"

#include "engine/core/application.h"
#include "engine/core/game_window.h"

#include <GL/glew.h>
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif

namespace fightingengine {

    ImGui_Manager::ImGui_Manager(GameWindow* window)
        //: _mousePressed{ false, false, false }
        //, _mouseCursors{ 0 }
        //, _clipboardTextData(nullptr)
        //, _lastScroll(0)
    {
        //Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // setup platform/renderer
        ImGui_ImplSDL2_InitForOpenGL(window->GetHandle(), window->get_gl_context());
        ImGui_ImplOpenGL3_Init(window->get_glsl_version().c_str());
    }

    ImGui_Manager::~ImGui_Manager()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGui_Manager::begin(const GameWindow& window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window.GetHandle());

        ImGui::NewFrame();
    }

    void ImGui_Manager::end(const GameWindow& window)
    {
        int width, height = 0;
        window.GetSize(width, height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        // Rendering
        ImGui::Render();
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
    }

    bool ImGui_Manager::ProcessEventSdl2(const SDL_Event& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        switch (event.type)
        {
        case SDL_QUIT:
            return false;
        case SDL_MOUSEWHEEL:
        {
            if (event.wheel.x > 0)
                io.MouseWheelH += 1;
            if (event.wheel.x < 0)
                io.MouseWheelH -= 1;
            if (event.wheel.y > 0)
                io.MouseWheel += 1;
            if (event.wheel.y < 0)
                io.MouseWheel -= 1;
            return io.WantCaptureMouse;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            return io.WantCaptureMouse;
        }
        case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event.text.text);
            return io.WantTextInput;
        }
        case SDL_KEYDOWN:
            //if (event.key.keysym.sym == SDLK_BACKQUOTE)
            //{
            //}
        case SDL_KEYUP:
        {
            int key = event.key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (event.type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            return io.WantCaptureKeyboard;
        }
        }
        return io.WantCaptureMouse;
    }

    //const char* ImGui_Manager::GetClipboardText()
    //{
    //    if (_clipboardTextData)
    //        SDL_free(_clipboardTextData);
    //    _clipboardTextData = SDL_GetClipboardText();
    //    return _clipboardTextData;
    //}

    //void ImGui_Manager::SetClipboardText(const char* text)
    //{
    //    SDL_SetClipboardText(text);
    //}
}


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
