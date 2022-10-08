#include "ui_hp_bar.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/renderer/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
init_ui_hp_bar(GameEditor& editor, Game& game, entt::entity& player)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  // hp bar for player
  {
    auto hp_back = create_gameplay(editor, game, EntityType::ui_health_bar);
    {
      create_renderable(editor, r, hp_back, EntityType::ui_health_bar);
      SpriteColourComponent& s_back = r.get<SpriteColourComponent>(hp_back);
      TransformComponent& t_back = r.get<TransformComponent>(hp_back);
      s_back.colour = colours.lin_red;
      t_back.scale.x = 200;
    }

    auto hp_front = create_gameplay(editor, game, EntityType::ui_health_bar);
    {
      create_renderable(editor, r, hp_front, EntityType::ui_health_bar);
      SpriteColourComponent& s_front = r.get<SpriteColourComponent>(hp_front);
      TransformComponent& t_front = r.get<TransformComponent>(hp_front);
      s_front.colour = colours.lin_green;
      t_front.scale.x = 200;
    }

    HpBar hp_bar;
    hp_bar.entity = player;
    hp_bar.back = hp_back;
    hp_bar.front = hp_front;
    auto hp_bar_entity = r.create();
    r.emplace<HpBar>(hp_bar_entity, hp_bar);
  }
}

void
update_ui_hp_bar(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto view = r.view<HpBar>();
  for (auto [entity, hp] : view.each()) {

    if (!r.valid(hp.entity))
      return;

    HealthComponent& health = r.get<HealthComponent>(hp.entity);
    TransformComponent& front = r.get<TransformComponent>(hp.front);
    int& cur_hp = health.hp;
    int& max_hp = health.max_hp;
    float percent = cur_hp / static_cast<float>(max_hp);
    int width = 100;
    front.scale.x = glm::max(0.0f, width * percent);
  }
};

};