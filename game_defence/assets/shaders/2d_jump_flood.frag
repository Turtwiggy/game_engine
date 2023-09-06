#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform float u_offset;
uniform vec2 screen_wh;
uniform sampler2D tex;

void
main()
{
  vec2 SCREEN_PIXEL_SIZE = vec2(1.0/screen_wh.x, 1.0/screen_wh.y);

	float best_dist = 9999.9;
  vec2 best_pos = vec2(0.0);

  // Sample 9 neighbour texels
   for(float x = -1.0; x <= 1.0; x++)
   {
    for(float y = -1.0; y <= 1.0; y++)
    {
      vec2 voffset = v_uv;
			voffset += vec2(x, y) * SCREEN_PIXEL_SIZE * u_offset;

      vec2 pos = texture(tex, voffset).xy;
      float dist = distance(pos.xy, v_uv.xy);

      if(pos.x != 0.0 && pos.y != 0.0 && dist < best_dist){
        best_dist = dist;
        best_pos = pos;
      }
    }
  }
  
  out_color = vec4(best_pos, 0.0, 1.0);
}