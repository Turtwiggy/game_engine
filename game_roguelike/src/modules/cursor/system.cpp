// your header
#include "system.hpp"

#include "events/helpers/mouse.hpp"
#include "helpers.hpp"
#include "modules/camera/helpers.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::update_cursor_system(GameEditor& editor, Game& game)
{
  auto& registry = game.state;
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(editor, game);
  update_free_cursor(registry, mouse_position);
  update_grid_cursor(registry, mouse_position);
};
