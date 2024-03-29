// header
#include "helpers.hpp"

// other lib
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
using namespace engine; // used for macro

#include "modules/camera/orthographic.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#if defined(__EMSCRIPTEN__)
#include <SDL2/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif

#include <array>

game2d::ViewportInfo
game2d::render_texture_to_imgui_viewport(const int64_t& tex_id)
{
  static bool dockspace_open = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", &dockspace_open, window_flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  // Dockspace
  ImGuiIO& io = ImGui::GetIO();

  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("RootDockSpace");

    if (!ImGui::DockBuilderGetNode(dockspace_id)) {
      ImVec2 vps = ImGui::GetContentRegionAvail();
      ImGui::DockBuilderRemoveNode(dockspace_id);
      ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspace_id, vps);

      ImGuiID dock_id_main = dockspace_id;
      ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Up, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.15f, nullptr, &dock_id_main);

      // HACK: hard coded names for windows
      // TODO: improve this
      ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
      ImGui::DockBuilderDockWindow("Player", dock_id_left);
      ImGui::DockBuilderDockWindow("GameEvents", dock_id_top);

      // tools
      ImGui::DockBuilderDockWindow("ColourEditor", dock_id_down);
      ImGui::DockBuilderDockWindow("Prefabs", dock_id_down);
      ImGui::DockBuilderDockWindow("Shop", dock_id_down);
      ImGui::DockBuilderDockWindow("SpriteSearcher", dock_id_down);

      ImGui::DockBuilderFinish(dock_id_main);
    }
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGuiWindowFlags viewport_flags = ImGuiWindowFlags_NoTitleBar;

  ImGuiWindowClass window_class;
  window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
  ImGui::SetNextWindowClass(&window_class);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("Viewport", NULL, viewport_flags);
  ImGui::PopStyleVar();
  ImVec2 viewport_size = ImGui::GetContentRegionAvail();

  ViewportInfo vi;
  vi.focused = ImGui::IsWindowFocused();
  vi.hovered = ImGui::IsWindowHovered();
  vi.size = viewport_size;
  vi.pos = ImGui::GetWindowPos();

  // render opengl texture to imgui
  ImGui::Image((ImTextureID)tex_id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));

  ImGui::End();

  ImGui::End();

  return vi;
};

bool
game2d::check_if_viewport_resize(const SINGLETON_RendererInfo& ri)
{
  auto viewport_wh = ri.viewport_size_render_at;

  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (viewport_wh.x != ri.viewport_size_current.x || viewport_wh.y != ri.viewport_size_current.y)) {
    return true;
  }
  return false;
};

std::optional<game2d::TextureUnit>
game2d::search_for_texture_unit_by_texture_path(const game2d::SINGLETON_RendererInfo& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_unit;

  return std::nullopt;
};

std::optional<game2d::TextureId>
game2d::search_for_texture_id_by_texture_path(const game2d::SINGLETON_RendererInfo& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_id;

  return std::nullopt;
};

std::optional<game2d::TextureUnit>
game2d::search_for_texture_unit_by_spritesheet_path(const game2d::SINGLETON_RendererInfo& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.spritesheet_path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_unit;

  return std::nullopt;
};

std::optional<game2d::TextureId>
game2d::search_for_texture_id_by_spritesheet_path(const game2d::SINGLETON_RendererInfo& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.spritesheet_path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_id;

  return std::nullopt;
};
