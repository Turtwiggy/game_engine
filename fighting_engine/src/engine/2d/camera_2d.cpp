#include "camera_2d.hpp"

#include "glm/glm.hpp"

#define BASE 2
//#define BASE sqrtf(2)

float linear_zoom_to_scale(float linear)
{
    return pow(BASE, linear);
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

vec2f camera::tile_to_screen(render_window& win, vec2f tile_pos) const
{
    vec2i screen_dim = win.get_window_size();

    vec2f half_dim = vec2f{ screen_dim.x(), screen_dim.y() } / 2.f;

    vec2f relative = (vec2f{ tile_pos.x(), tile_pos.y() } *TILE_PIX - pos) * calculate_scale() + half_dim;

    return relative;
}

vec2f camera::screen_to_tile(render_window& win, vec2f screen_pos) const
{
    vec2i screen_dim = win.get_window_size();

    vec2f half_dim = vec2f{ screen_dim.x(), screen_dim.y() } / 2.f;

    vec2f scaled = screen_pos - half_dim;
    scaled /= calculate_scale();

    scaled += pos;

    scaled /= TILE_PIX;

    return scaled;
}

vec2f camera::world_to_screen(render_window& win, vec2f world_pos) const
{
    vec2i screen_dim = win.get_window_size();

    vec2f half_dim = vec2f{ screen_dim.x(), screen_dim.y() } / 2.f;

    vec2f relative = (world_pos - pos) * calculate_scale() + half_dim;

    return relative;
}

vec2f camera::screen_to_world(render_window& win, vec2f screen_pos) const
{
    vec2i screen_dim = win.get_window_size();

    vec2f half_dim = vec2f{ screen_dim.x(), screen_dim.y() } / 2.f;

    vec2f absolute = ((screen_pos - half_dim) / calculate_scale()) + pos;

    return absolute;
}

vec2f camera::tile_to_world(vec2f pos)
{
    return vec2f{ pos.x(), pos.y() } *TILE_PIX + vec2f{ TILE_PIX / 2, TILE_PIX / 2 };
}

//vec2f camera::world_to_tile(vec2f pos)
//{
//    return (pos - vec2f{ TILE_PIX / 2, TILE_PIX / 2 }) / (float)TILE_PIX ;
//}

void camera::translate(vec2f amount)
{
    pos += amount / calculate_scale();
}

void camera::zoom(float number_of_levels)
{
    zoom_level += number_of_levels;

    zoom_level = clamp(zoom_level, -3, 3);
}

float camera::calculate_scale() const
{
    return linear_zoom_to_scale(zoom_level);
}
