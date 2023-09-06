#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;
uniform float u_dist_mod = 1.0;

void
main()
{
  // input is the voronoi output which stores in each pixel the UVs of the closest surface.
  // here we simply take that value, calculate the distance between the closest surface and this
  // pixel, and return that distance. 
  vec4 t = texture(tex, v_uv);
  float dist = distance(t.xy, v_uv);
  float mapped = clamp(dist * u_dist_mod, 0.0, 1.0);
  out_color = vec4(vec3(mapped), 1.0);
}