#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;
uniform float u_dist_mod = 1.0;
uniform vec2 screen_wh;

void
main()
{
	// translate uvs from the square voronoi buffer back to viewport size.
  vec2 SCREEN_PIXEL_SIZE = vec2(1.0/screen_wh.x, 1.0/screen_wh.y);
	vec2 uv = v_uv;
	if(SCREEN_PIXEL_SIZE.x < SCREEN_PIXEL_SIZE.y)
		uv.y = ((uv.y - 0.5) * (SCREEN_PIXEL_SIZE.x/ SCREEN_PIXEL_SIZE.y)) + 0.5;
	else
		uv.x = ((uv.x - 0.5) * (SCREEN_PIXEL_SIZE.y/ SCREEN_PIXEL_SIZE.x)) + 0.5;
		
  // input is the voronoi output which stores in each pixel the UVs of the closest surface.
  // here we simply take that value, calculate the distance between the closest surface and this
  // pixel, and return that distance. 
  vec4 t = texture(tex, uv);
  float dist = distance(t.xy, uv);
  float mapped = clamp(dist * u_dist_mod, 0.0, 1.0);
  out_color = vec4(vec3(mapped), 1.0);
}