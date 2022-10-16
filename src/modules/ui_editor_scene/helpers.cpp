#include "helpers.hpp"

#include "serialize.hpp"

#include "game/components/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_editor_tilemap/components.hpp"

#include "magic_enum.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void
game2d::save(const entt::registry& registry, std::string path)
{
  std::cout << "saving...\n";

  // convert entt registry to string(or bson)
  NJSONOutputArchive json_archive;
  entt::basic_snapshot{ registry }
    .entities(json_archive)
    .component<
      //
      TilemapComponent,
      TagComponent,
      EntityTypeComponent,
      TransformComponent,
      SpriteComponent,
      SpriteColourComponent>(json_archive);
  json_archive.close();

  std::string data = json_archive.as_string();

  // save to disk
  std::ofstream fout(path);
  fout << data.c_str();
};

void
game2d::load(GameEditor& editor, Game& game, std::string path)
{
  std::cout << "loading...\n";

  auto& r = game.state;
  r.each([&r](auto entity) { r.destroy(entity); });

  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data = buffer.str();

  // convert string (or bson) to entt registry
  auto& registry_to_load_in_to = r;

  NJSONInputArchive json_in(data);
  entt::basic_snapshot_loader{ registry_to_load_in_to }
    .entities(json_in)
    .component<
      // editor
      TilemapComponent,
      TagComponent,
      EntityTypeComponent,
      TransformComponent,
      SpriteComponent,
      SpriteColourComponent>(json_in);

  const auto& view = r.view<const EntityTypeComponent>();
  view.each([&editor, &r](auto entity, const EntityTypeComponent& type) {
    create_gameplay_existing_entity(editor, r, entity, type.type);
  });
};

void
game2d::load_if_exists(entt::registry& registry, std::string path){
  //   std::ifstream file(path.c_str());
  //   if (file)
  //     load(registry, path);
};
