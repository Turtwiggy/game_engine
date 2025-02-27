#include "ui_debug_menubar_helpers.hpp"
#include "ui_debug_menubar_components.hpp"

namespace game2d {

MenuBar
gesert_menubar_state(SINGLE_DebugMenuBar& state, std::string label)
{
  auto matching_label = [&label](const MenuBar& mb) { return mb.name == label; };
  auto it = std::find_if(state.windows.begin(), state.windows.end(), matching_label);
  if (it == state.windows.end()) {
    // create new
    MenuBar menubar;
    menubar.enabled = false;
    menubar.name = label;
    state.windows.push_back(menubar);

    // sort based on name.
    auto alphabetically = [](const MenuBar& a, const MenuBar& b) { return a.name[0] < b.name[0]; };
    std::sort(state.windows.begin(), state.windows.end(), alphabetically);

    return menubar;
  }
  return *it;
}

} // namespace game2d