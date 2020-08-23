#include "camera_2d.hpp"

#include "glm/glm.hpp"

namespace fightingengine {

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

    vec2 Camera2D::tile_to_screen(GameWindow& win, vec2 tile_pos) const
    {
        int width, height = 0;
        win.GetSize(width, height);

        vec2 half_dim = vec2{ width, height } / 2.f;

        vec2 pos = vec2(tile_pos.x, tile_pos.y);

        vec2 relative = (  pos * (float)tile_pixels - pos) * calculate_scale() + half_dim;

        return relative;
    }

    vec2 Camera2D::screen_to_tile(GameWindow& win, vec2 screen_pos) const
    {
        int width, height = 0;
        win.GetSize(width, height);

        vec2 half_dim = vec2{ width, height } / 2.f;

        vec2 scaled = screen_pos - half_dim;
        scaled /= calculate_scale();

        scaled += pos;

        scaled /= tile_pixels;

        return scaled;
    }

    vec2 Camera2D::world_to_screen(GameWindow& win, vec2 world_pos) const
    {
        int width, height = 0;
        win.GetSize(width, height);

        vec2 half_dim = vec2{ width, height } / 2.f;

        vec2 relative = (world_pos - pos) * calculate_scale() + half_dim;

        return relative;
    }

    vec2 Camera2D::screen_to_world(GameWindow& win, vec2 screen_pos) const
    {
        int width, height = 0;
        win.GetSize(width, height);

        vec2 half_dim = vec2{ width, height } / 2.f;

        vec2 absolute = ((screen_pos - half_dim) / calculate_scale()) + pos;

        return absolute;
    }

    vec2 Camera2D::tile_to_world(vec2 pos, float tile_pixels)
    {
        return vec2{ pos.x, pos.y } *tile_pixels + vec2{ tile_pixels / 2, tile_pixels / 2 };
    }

    //vec2f camera::world_to_tile(vec2f pos)
    //{
    //    return (pos - vec2f{ TILE_PIX / 2, TILE_PIX / 2 }) / (float)TILE_PIX ;
    //}

    void Camera2D::translate(vec2 amount)
    {
        pos += amount / calculate_scale();
    }

    void Camera2D::zoom(float number_of_levels)
    {
        zoom_level += number_of_levels;

        zoom_level = clamp(zoom_level, -3.0f, 3.0f);
    }

    float Camera2D::calculate_scale() const
    {
        return linear_zoom_to_scale(zoom_level);
    }

}
