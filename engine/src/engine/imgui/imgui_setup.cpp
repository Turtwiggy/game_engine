
// your project header
#include "imgui_setup.hpp"

// other library headers
#include <glm/glm.hpp>
#include <imgui.h>
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>
#endif
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif

namespace fightingengine {

ImGui_Manager::~ImGui_Manager()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void
ImGui_Manager::initialize(GameWindow* window)
//: _mousePressed{ false, false, false }
//, _mouseCursors{ 0 }
//, _clipboardTextData(nullptr)
//, _lastScroll(0)
{
  // Setup ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
  // Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  //
  // Fonts
  //
  // bold font
  // io.Fonts->AddFontFromFileTTF("assets/fonts/droid_sans.ttf", 12.0f);
  // regular font
  // io.FontDefault =
  // io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto/Roboto-Regular.ttf", 14.0f);

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window->get_handle(), window->get_gl_context());
  ImGui_ImplOpenGL3_Init(window->get_glsl_version().c_str());
}

void
ImGui_Manager::begin_frame(const GameWindow& window)
{
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window.get_handle());
  ImGui::NewFrame();
}

void
ImGui_Manager::end_frame(const GameWindow& window)
{
  glm::ivec2 window_size = window.get_size();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(window_size.x), static_cast<float>(window_size.y));

  // Rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this
  // code elsewhere.
  //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
  }
}

void
ImGui_Manager::process_event(const SDL_Event* event)
{
  ImGui_ImplSDL2_ProcessEvent(event);
}

} // namespace fightingengine
