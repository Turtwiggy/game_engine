
//header
#include "camera_2d.hpp"

//other library headers
#include <glm/glm.hpp>

namespace fightingengine {

#define BASE 2
//#define BASE sqrtf(2)

float linear_zoom_to_scale(float linear)
{
    return static_cast<float>(pow(BASE, linear));
}

float logb(float base, float val)
{
    return log(val) / log(base);
}

float scale_to_linear_zoom(float scale)
{
    return logb(BASE, scale);
}

///scale = r2 ^ linear;

glm::vec2 Camera2D::tile_to_screen(GameWindow& win, glm::vec2 tile_pos) const
{
    int width, height = 0;
    win.GetSize(width, height);

    glm::vec2 half_dim = glm::vec2{ width, height } / 2.f;

    glm::vec2 pos = glm::vec2(tile_pos.x, tile_pos.y);

    glm::vec2 relative = (  pos * (float)tile_pixels - pos) * calculate_scale() + half_dim;

    return relative;
}

glm::vec2 Camera2D::screen_to_tile(GameWindow& win, glm::vec2 screen_pos) const
{
    int width, height = 0;
    win.GetSize(width, height);

    glm::vec2 half_dim = glm::vec2{ width, height } / 2.f;

    glm::vec2 scaled = screen_pos - half_dim;
    scaled /= calculate_scale();

    scaled += pos;

    scaled /= tile_pixels;

    return scaled;
}

glm::vec2 Camera2D::world_to_screen(GameWindow& win, glm::vec2 world_pos) const
{
    int width, height = 0;
    win.GetSize(width, height);

    glm::vec2 half_dim = glm::vec2{ width, height } / 2.f;

    glm::vec2 relative = (world_pos - pos) * calculate_scale() + half_dim;

    return relative;
}

glm::vec2 Camera2D::screen_to_world(GameWindow& win, glm::vec2 screen_pos) const
{
    int width, height = 0;
    win.GetSize(width, height);

    glm::vec2 half_dim = glm::vec2{ width, height } / 2.f;

    glm::vec2 absolute = ((screen_pos - half_dim) / calculate_scale()) + pos;

    return absolute;
}

glm::vec2 Camera2D::tile_to_world(glm::vec2 pos, float tile_pixels)
{
    return glm::vec2{ pos.x, pos.y } *tile_pixels + glm::vec2{ tile_pixels / 2, tile_pixels / 2 };
}

//glm::vec2f camera::world_to_tile(glm::vec2f pos)
//{
//    return (pos - glm::vec2f{ TILE_PIX / 2, TILE_PIX / 2 }) / (float)TILE_PIX ;
//}

void Camera2D::translate(glm::vec2 amount)
{
    pos += amount / calculate_scale();
}

void Camera2D::zoom(float number_of_levels)
{
    zoom_level += number_of_levels;

    zoom_level = glm::clamp(zoom_level, -3.0f, 3.0f);
}

float Camera2D::calculate_scale() const
{
    return linear_zoom_to_scale(zoom_level);
}

} //namespace fightingengine
