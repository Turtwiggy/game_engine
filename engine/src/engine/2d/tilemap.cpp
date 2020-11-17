
//header
#include "engine/2d/tilemap.hpp"

//other project headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

//your project headers
#include "engine/graphics/colour.hpp"

namespace fightingengine {

template<typename T>
void add_to(T& in, glm::ivec2 loc)
{
    in.push_back(loc);
}

std::map<tiles::type, std::vector<glm::ivec2>>& get_locations()
{
    static std::map<tiles::type, std::vector<glm::ivec2>> ret;

    using namespace tiles;

    ///consider adding skull and crossbones to dirt
    //add_to(ret[BASE], {0, 0});
    add_to(ret[BASE], { 8, 5 });
    add_to(ret[WATER], { 8, 5 });

    add_to(ret[DIRT], { 1, 0 });
    add_to(ret[DIRT], { 2, 0 });
    add_to(ret[DIRT], { 3, 0 });
    add_to(ret[DIRT], { 4, 0 });

    add_to(ret[GRASS], { 5, 0 });
    add_to(ret[GRASS], { 6, 0 });
    add_to(ret[GRASS], { 7, 0 });

    add_to(ret[TREE_1], { 0, 1 });
    add_to(ret[TREE_1], { 1, 1 });
    add_to(ret[TREE_2], { 2, 1 });
    add_to(ret[TREE_2], { 4, 1 });
    add_to(ret[TREE_DENSE], { 3, 1 });
    add_to(ret[TREE_DENSE], { 3, 2 });
    add_to(ret[TREE_ROUND], { 5, 1 });
    add_to(ret[TREE_ROUND], { 4, 2 });

    add_to(ret[CACTUS], { 6, 1 });
    add_to(ret[DENSE_CACTUS], { 7, 1 });

    add_to(ret[VINE], { 2, 2 });
    add_to(ret[SHRUB], { 0, 2 });

    add_to(ret[ROCKS], { 5, 2 });

    add_to(ret[BRAMBLE], { 0, 2 });
    add_to(ret[BRAMBLE], { 6, 2 });

    ///top left man is 24, 0

    add_to(ret[CIVILIAN], { 25, 0 });
    add_to(ret[CIVILIAN], { 26, 0 });
    add_to(ret[CIVILIAN], { 26, 1 });
    add_to(ret[CIVILIAN], { 27, 1 });
    add_to(ret[CIVILIAN], { 29, 1 });
    add_to(ret[CIVILIAN], { 30, 1 });
    add_to(ret[CIVILIAN], { 30, 3 });
    add_to(ret[CIVILIAN], { 31, 3 });
    add_to(ret[CIVILIAN], { 30, 4 });
    add_to(ret[CIVILIAN], { 31, 4 });
    ///top left scorpion is 24, 5

    add_to(ret[SOLDIER], { 26, 4 });
    add_to(ret[SOLDIER], { 30, 9 });

    add_to(ret[SOLDIER_BASIC], { 25, 0 });
    add_to(ret[SOLDIER_SPEAR], { 26, 0 });
    add_to(ret[SOLDIER_BASIC_SHIELD], { 27, 0 });
    add_to(ret[SOLDIER_ADVANCED], { 28, 0 });
    add_to(ret[SOLDIER_ADVANCED_SPEAR], { 29, 0 });
    add_to(ret[SOLDIER_TOUGH], { 30, 0 });
    add_to(ret[SOLDIER_BEST], { 31, 0 });

    add_to(ret[GROUND_BUG], { 28, 5 });
    add_to(ret[GROUND_BUG], { 29, 5 });
    add_to(ret[GROUND_BUG], { 30, 5 });
    add_to(ret[SMALL_PINCHY], { 31, 5 });
    add_to(ret[FLYING_BUG], { 26, 5 });
    add_to(ret[ARMOURED_BUG], { 27, 5 });
    add_to(ret[SCORPION], { 24, 5 });

    add_to(ret[LAND_ANIMAL], { 25, 7 });
    add_to(ret[LAND_ANIMAL], { 26, 7 });
    add_to(ret[LAND_ANIMAL], { 27, 7 });
    add_to(ret[LAND_ANIMAL], { 28, 7 });
    //add_to(ret[LAND_ANIMAL], {29, 7});
    add_to(ret[LAND_ANIMAL], { 30, 7 });
    add_to(ret[LAND_ANIMAL], { 31, 7 });
    add_to(ret[LAND_ANIMAL], { 26, 8 }); //bat

    add_to(ret[SEA_ANIMAL], { 25, 8 });
    add_to(ret[SEA_ANIMAL], { 28, 8 });
    add_to(ret[CROCODILE], { 29, 8 });

    add_to(ret[FACE_MALE], { 24, 10 });
    add_to(ret[FACE_MALE], { 26, 10 });
    add_to(ret[FACE_MALE], { 27, 10 });
    add_to(ret[FACE_MALE], { 28, 10 });
    add_to(ret[FACE_MALE], { 29, 10 });

    add_to(ret[FACE_WOMAN], { 25, 10 });
    add_to(ret[FACE_WOMAN], { 30, 10 });
    add_to(ret[FACE_WOMAN], { 31, 10 });

    add_to(ret[THIN_DOOR_CLOSED], { 3, 4 });
    add_to(ret[THIN_DOOR_OPEN], { 4, 4 });

    add_to(ret[DOOR_CLOSED], { 3, 3 });
    add_to(ret[THIN_DOOR_OPEN], { 4, 3 });

    add_to(ret[GRAVE], { 0, 14 });
    add_to(ret[GRAVE], { 1, 14 });
    add_to(ret[GRAVE], { 2, 14 });

    add_to(ret[WOOD_FENCE_FULL], { 1, 3 });
    add_to(ret[WOOD_FENCE_FULL], { 2, 3 });
    add_to(ret[WOOD_FENCE_HALF], { 0, 3 });

    add_to(ret[TILING_WALL], { 16, 19 });
    add_to(ret[TILING_WALL], { 17, 19 });
    add_to(ret[TILING_WALL], { 18, 19 });

    add_to(ret[TILING_WALL], { 16, 20 });
    add_to(ret[TILING_WALL], { 17, 20 });
    add_to(ret[TILING_WALL], { 18, 20 });

    add_to(ret[TILING_WALL], { 16, 21 });
    add_to(ret[TILING_WALL], { 17, 21 });
    add_to(ret[TILING_WALL], { 18, 21 });

    add_to(ret[CULTIVATION], { 13, 6 });
    add_to(ret[CULTIVATION], { 14, 6 });
    add_to(ret[CULTIVATION], { 15, 6 });
    add_to(ret[CULTIVATION], { 16, 6 });
    add_to(ret[CULTIVATION], { 17, 6 });

    //24,11 slash effect
    add_to(ret[EFFECT_1], { 24, 11 });
    add_to(ret[EFFECT_2], { 25, 11 });
    add_to(ret[EFFECT_3], { 26, 11 });
    add_to(ret[EFFECT_4], { 27, 11 });
    add_to(ret[EFFECT_5], { 28, 11 });
    add_to(ret[EFFECT_6], { 29, 11 });
    add_to(ret[EFFECT_7], { 30, 11 });
    add_to(ret[EFFECT_8], { 31, 11 });
    add_to(ret[EFFECT_9], { 27, 12 });
    add_to(ret[EFFECT_10], { 28, 12 });
    add_to(ret[EFFECT_11], { 29, 12 });
    add_to(ret[EFFECT_12], { 30, 12 });
    add_to(ret[EFFECT_13], { 31, 12 });

    add_to(ret[HOUSE_1], { 0, 19 });
    add_to(ret[HOUSE_2], { 1, 19 });
    add_to(ret[HOUSE_3], { 0, 20 });
    add_to(ret[HOUSE_4], { 1, 20 });

    add_to(ret[TENT], { 6, 20 });
    add_to(ret[FANCY_TENT], { 7, 20 });
    add_to(ret[CAPITAL_TENT], { 8, 20 });

    add_to(ret[TOWER_THIN], { 2, 19 });
    add_to(ret[TOWER_MEDIUM], { 3, 19 });
    add_to(ret[TOWER_THICK], { 4, 19 });

    add_to(ret[CASTLE_1], { 5, 19 });
    add_to(ret[CASTLE_2], { 6, 19 });

    add_to(ret[PYRAMID], { 2, 20 });
    add_to(ret[CHURCH], { 3, 20 });


    return ret;
}

// SpriteHandle get_sprite_handle_of(RandomState& rng, tiles::type type)
// {
//     auto& tiles = get_locations();

//     auto& which = tiles[type];

//     if (which.size() == 0)
//         throw std::runtime_error("No tiles for type " + std::to_string(type));

//     int len = (int)(which.size());

//     int iwhich = (int)rand_det_s(rng.rng, 0, static_cast<float>(len));
//     if (iwhich >= len || iwhich < 0)
//         throw std::runtime_error("Rng is bad");

//     SpriteHandle handle;
//     handle.offset = which[0]; //get the first for now

//     ColourVec4f colour;
//     handle.colour = colour;
//     //handle.base_colour = get_colour_of(type/*, level_info::GRASS*/); //???

//     return handle;
// }

// vec4 get_colour_of(tiles::type tile_type/*, level_info::types level_type*/)
// {
//     vec3 mask_col3 = glm::normalize(srgb_to_lin_approx(vec3{ 71, 45, 60 } / 255.f));
//     vec4 mask_col = { mask_col3.x, mask_col3.y, mask_col3.z, 1.f };

//     vec4 barren_col = srgb_to_lin_approx(vec4{ 122, 68, 74, 255 } / 255.f);
//     vec4 grass_col = srgb_to_lin_approx(vec4{ 56, 217, 115, 255 } / 255.f);

//     vec4 blue_col = srgb_to_lin_approx(vec4{ 60, 172, 215, 255 } / 255.f);

//     if (tile_type == tiles::WATER)
//         return blue_col;

//     if (tile_type == tiles::BRAMBLE || tile_type == tiles::SHRUB || tile_type == tiles::BASE)
//     {
//         //if (level_type == level_info::GRASS)
//         //    return grass_col * mask_col;
//         //else
//             return barren_col;
//     }

//     if (tile_type == tiles::DIRT)
//         return barren_col;

//     if (tile_type == tiles::GRASS)
//         return grass_col * grass_col;

//     if (tile_type == tiles::TREE_1 || tile_type == tiles::TREE_2 || tile_type == tiles::TREE_DENSE ||
//         tile_type == tiles::TREE_ROUND || tile_type == tiles::CACTUS || tile_type == tiles::VINE ||
//         tile_type == tiles::CULTIVATION || tile_type == tiles::CROCODILE)
//         return grass_col;

//     vec4 wood_col = srgb_to_lin_approx(vec4{ 191, 121, 88, 255 } / 255.f);
//     vec4 building_gray = srgb_to_lin_approx(vec4{ 207, 198, 184, 255 } / 255.f);
//     vec4 generic_red = srgb_to_lin_approx(vec4{ 230, 72, 46, 255 } / 255.f);
//     vec4 white_col = srgb_to_lin_approx(vec4{ 255, 255, 255, 255 } / 255.f);

//     if (tile_type == tiles::EFFECT_1 || tile_type == tiles::EFFECT_2 || tile_type == tiles::EFFECT_3 ||
//         tile_type == tiles::EFFECT_4 || tile_type == tiles::EFFECT_5 || tile_type == tiles::EFFECT_6 ||
//         tile_type == tiles::EFFECT_7 || tile_type == tiles::EFFECT_8 || tile_type == tiles::EFFECT_9 ||
//         tile_type == tiles::EFFECT_10 || tile_type == tiles::EFFECT_11 || tile_type == tiles::EFFECT_12 ||
//         tile_type == tiles::EFFECT_13)
//         return white_col;

//     if (tile_type == tiles::ROCKS || tile_type == tiles::GRAVE || tile_type == tiles::TILING_WALL)
//         return building_gray;

//     if (tile_type == tiles::LAND_ANIMAL)
//         return building_gray;

//     if (tile_type == tiles::SEA_ANIMAL)
//         return building_gray;

//     if (tile_type == tiles::CIVILIAN || (tile_type >= tiles::SOLDIER && tile_type <= tiles::SOLDIER_BEST) ||
//         tile_type == tiles::GROUND_BUG || tile_type == tiles::FLYING_BUG || tile_type == tiles::ARMOURED_BUG || tile_type == tiles::SMALL_PINCHY)
//         return building_gray;

//     if (tile_type == tiles::SCORPION)
//         return generic_red;

//     if (tile_type == tiles::FACE_MALE || tile_type == tiles::FACE_WOMAN)
//         return building_gray;

//     if (tile_type == tiles::WOOD_FENCE_FULL || tile_type == tiles::WOOD_FENCE_HALF || tile_type == tiles::TILING_WALL ||
//         tile_type == tiles::THIN_DOOR_CLOSED || tile_type == tiles::THIN_DOOR_OPEN ||
//         tile_type == tiles::DOOR_CLOSED || tile_type == tiles::DOOR_OPEN)
//         return wood_col;

//     if (tile_type == tiles::CASTLE_1 || tile_type == tiles::CASTLE_2 ||
//         tile_type == tiles::HOUSE_1 || tile_type == tiles::HOUSE_2 || tile_type == tiles::HOUSE_3 || tile_type == tiles::HOUSE_4 ||
//         tile_type == tiles::TENT || tile_type == tiles::FANCY_TENT || tile_type == tiles::CAPITAL_TENT)
//         return building_gray;

//     throw std::runtime_error("Did not find " + std::to_string(tile_type));
// }

//void tilemap::create(glm::ivec2 _dim)
//{
//    dim = _dim;
//    all_entities.resize(dim.x * dim.y);
//}

//void tilemap::add(entt::entity en, glm::ivec2 pos)
//{
//    if (pos.x < 0 || pos.y < 0 || pos.x >= dim.x || pos.y >= dim.y)
//        throw std::runtime_error("Add out of bounds");

//    all_entities[pos.y * dim.x + pos.x].push_back(en);
//}

//void tilemap::remove(entt::entity en, glm::ivec2 pos)
//{
//    if (pos.x < 0 || pos.y < 0 || pos.x >= dim.x || pos.y >= dim.y)
//    {
//        std::string err = "Remove out of bounds: pos.x(): " + std::to_string(pos.x) +
//            " pos.y(): " + std::to_string(pos.y) +
//            " dim.x(): " + std::to_string(dim.x) +
//            " dim.y(): " + std::to_string(dim.y);
//        throw std::runtime_error(err);
//    }

//    std::vector<entt::entity>& lst = all_entities[pos.y * dim.x + pos.x];

//    int size = (int)lst.size();

//    if (size == 0)
//        return;

//    for (int id = 0; id < size; id++)
//    {
//        entt::entity& ent = lst[id];

//        if (ent == en)
//        {
//            lst.erase(lst.begin() + id);
//            break;
//        }
//    }
//}

//void tilemap::move(entt::entity en, glm::ivec2 from, glm::ivec2 to)
//{
//    if (from.x < 0 || from.y < 0 || from.x >= dim.x || from.y >= dim.y)
//        throw std::runtime_error("From out of bounds");

//    if (to.x < 0 || to.y < 0 || to.x >= dim.x || to.y >= dim.y)
//        throw std::runtime_error("To out of bounds");

//    std::vector<entt::entity>& lst = all_entities[from.y * dim.x + from.x];

//    int size = (int)lst.size();

//    if (size == 0)
//        return;

//    for (int id = 0; id < size; id++)
//    {
//        entt::entity& ent = lst[id];

//        if (ent == en)
//        {
//            remove(ent, from);

//            add(ent, to);

//            break;
//        }
//    }

//}

//void tilemap::render(entt::registry& registry, render_window& win, camera& cam, sprite_renderer& renderer, vec2 mpos)
//{
//    vec2 mouse_tile = cam.screen_to_tile(win, mpos);
//    glm::ivec2 i_tile = { mouse_tile.x, mouse_tile.y };

//    //bool mouse_clicked = ImGui::IsMouseClicked(0) && !ImGui::IsAnyWindowHovered();
//    //bool mouse_hovering = !ImGui::IsAnyWindowHovered();

//    for (int y = 0; y < dim.y; y++)
//    {
//        for (int x = 0; x < dim.x; x++)
//        {
//            const auto& lst = all_entities[y * dim.x + x];

//            for (int id = 0; id < (int)lst.size(); id++)
//            {
//                auto en = lst[id];

//                //if (registry.has<mouse_interactable>(en))
//                //{
//                //    reset_interactable_state(registry, en);
//                //    mouse_interactable& interact = registry.get<mouse_interactable>(en);
//                //    if (i_tile == vec2i{ x, y })
//                //    {
//                //        if (mouse_hovering)
//                //        {
//                //            interact.is_hovered = true;
//                //            if (mouse_clicked)
//                //            {
//                //                interact.just_clicked = true;
//                //                selected = en;
//                //            }
//                //        }
//                //    }
//                //}
//                //else
//                //{
//                //    //Clicked something unclickable
//                //    if (mouse_clicked && i_tile == vec2i{ x, y })
//                //    {
//                //        selected = std::nullopt;
//                //    }
//                //}

//                SpriteHandle& handle = registry.get<SpriteHandle>(en);
//                RenderDescriptor desc = registry.get<RenderDescriptor>(en);

//                vec4f old_col = handle.base_colour;

//                vec4f shaded_col = srgb_to_lin_approx(vec4f{ 0.02, 0.02, 0.02, 1 });

//                if (id > 0 && id != (int)lst.size() - 1 && lst.size() > 2)
//                {
//                    handle.base_colour = mix(shaded_col, handle.base_colour, 0.1);
//                    //handle.base_colour.w() *= 0.3;
//                }

//                if (mouse_hovering && desc.depress_on_hover && i_tile == vec2i{ x, y })
//                {
//                    if (id > 0)
//                    {
//                        handle.base_colour.w() = 1;
//                        desc.pos.y() -= 3;
//                    }

//                    if (id == 0)
//                    {
//                        handle.base_colour = mix(shaded_col, handle.base_colour, 0.5);
//                    }
//                }

//                renderer.add(handle, desc);

//                handle.base_colour = old_col;
//            }
//        }
//    }

//    if (ImGui::IsMouseClicked(1) && !ImGui::IsAnyWindowHovered())
//    {
//        selected = std::nullopt;
//    }

//    if (selected.has_value())
//    {
//        if (registry.has<building_tag>(selected.value()))
//        {
//            building_tag& tag = registry.get<building_tag>(selected.value());

//            tag.show_build_ui();
//        }
//    }
//}

//int tilemap::entities_at_position(vec2i pos)
//{
//    if (pos.x() < 0 || pos.y() < 0 || pos.x() >= dim.x() || pos.y() >= dim.y())
//        throw std::runtime_error("Out of bounds");

//    return all_entities[pos.y() * dim.x() + pos.x()].size();
//}

} //namespace fightingengine
