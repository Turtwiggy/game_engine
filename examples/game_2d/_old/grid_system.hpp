#pragma once

// TODO: GRID STUFF
// if (app.get_input().get_mouse_rmb_down()) {
//   if (gs.editor_left_click_mode == EditorMode::PLAYER_ATTACK)
//     gs.editor_left_click_mode = EditorMode::EDITOR_PLACE_MODE;
//   else if (gs.editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE)
//     gs.editor_left_click_mode = EditorMode::EDITOR_SELECT_MODE;
//   else if (gs.editor_left_click_mode == EditorMode::EDITOR_SELECT_MODE)
//     gs.editor_left_click_mode = EditorMode::PLAYER_ATTACK;
//   auto mode = std::string(magic_enum::enum_name(gs.editor_left_click_mode));
//   std::cout << "editor mode: " << mode << std::endl;
// }
// bool lmb_clicked = app.get_input().get_mouse_lmb_down();
// if (lmb_clicked && gs.editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE) {
//   glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
//   printf("(game) clicked gamegrid %i %i \n", mouse_pos.x, mouse_pos.y);
//   glm::ivec2 world_pos = mouse_pos + gs.camera.pos;
//   GameObject2D tree = gameobject::create_tree();
//   tree.pos = grid::convert_world_space_to_grid_space(world_pos, GAME_GRID_SIZE);
//   tree.pos = grid::convert_grid_space_to_worldspace(tree.pos, GAME_GRID_SIZE);
//   tree.render_size = glm::ivec2(GAME_GRID_SIZE);
//   tree.physics_size = glm::ivec2(GAME_GRID_SIZE);
//   gs.entities_trees.push_back(tree);
// }