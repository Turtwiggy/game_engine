#version 460

out vec4 o_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

// https://www.shadertoy.com/view/ssdSD2
void
main()
{
  // todo: move to v
  float thickness = 0.05;
  float fade = 0.1;

  // -1 -> 1 local space
  vec2 uv = (v_uv * 2.0) - 1.0;
    
  // Calculate distance and fill circle with white
  float distance = 1.0 - length(uv);
  float circle = smoothstep(0.0, fade, distance);
  circle *= smoothstep(thickness + fade, thickness, distance);

  // Set output colour
  o_colour = v_colour;
  o_colour.a = circle;
}