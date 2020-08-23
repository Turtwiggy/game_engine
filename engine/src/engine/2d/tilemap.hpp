#pragma once

#include "engine/2d/renderer/sprite_renderer.hpp"
#include "engine/core/random.hpp"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include <vector>
#include <map>
#include <optional>

namespace fightingengine
{
    namespace tiles
    {
        enum type
        {
            BASE,
            WATER,
            DIRT,
            GRASS,
            TREE_1,
            TREE_2,
            TREE_DENSE,
            TREE_ROUND,
            CACTUS,
            DENSE_CACTUS,
            VINE,
            SHRUB,
            ROCKS,
            BRAMBLE,
            CIVILIAN,
            SOLDIER,

            SOLDIER_BASIC,
            SOLDIER_SPEAR,
            SOLDIER_BASIC_SHIELD,
            SOLDIER_ADVANCED,
            SOLDIER_ADVANCED_SPEAR,
            SOLDIER_TOUGH,
            SOLDIER_BEST,

            GROUND_BUG,
            FLYING_BUG,
            ARMOURED_BUG,
            SCORPION,
            SMALL_PINCHY,
            LAND_ANIMAL,
            SEA_ANIMAL,
            CROCODILE,
            FACE_MALE,
            FACE_WOMAN,
            THIN_DOOR_CLOSED,
            THIN_DOOR_OPEN,
            DOOR_CLOSED,
            DOOR_OPEN,
            GRAVE,
            WOOD_FENCE_FULL,
            WOOD_FENCE_HALF,
            TILING_WALL,
            CULTIVATION,

            //effects
            EFFECT_1,       //swipe
            EFFECT_2,       //curved swipe
            EFFECT_3,       //slash claws
            EFFECT_4,
            EFFECT_5,       //fire
            EFFECT_6,       //fireball
            EFFECT_7,       //
            EFFECT_8,       //stationary fire
            EFFECT_9,
            EFFECT_10,      //snow?
            EFFECT_11,      //snow?
            EFFECT_12,      //snow?
            EFFECT_13,      //snow?

            //medieval houses in increasing height
            HOUSE_1,
            HOUSE_2,
            HOUSE_3,
            HOUSE_4,

            TENT,
            FANCY_TENT,
            CAPITAL_TENT, //not sure, big fancy thing

            TOWER_THIN,
            TOWER_MEDIUM,
            TOWER_THICK,

            //variations on the same style
            CASTLE_1,
            CASTLE_2,

            PYRAMID,
            CHURCH,
        };
    };

    std::map<tiles::type, std::vector<glm::ivec2>>& get_locations();
    SpriteHandle get_sprite_handle_of(random_state& rng, tiles::type type);
    glm::vec4 get_colour_of(tiles::type type/*, level_info::types level_type*/);

    struct tilemap
    {
        //std::optional<entt::entity> selected;

        //glm::vec2 dim;
        // x * y, back to front rendering
        //std::vector<std::vector<entt::entity>> all_entities;

        //void create(glm::ivec2 dim);
        //void add(entt::entity en, glm::ivec2 pos);
        //void remove(entt::entity en, glm::ivec2 pos);
        //void move(entt::entity en, glm::ivec2 from, glm::ivec2 to);
        //void render(entt::registry& reg, render_window& win, camera& cam, sprite_renderer& renderer, vec2f mpos);

        //int entities_at_position(glm::vec2 pos);
    };

}
