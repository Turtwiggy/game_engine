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

float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }

void
main()
{
  vec2 SCREEN_PIXEL_SIZE = vec2(1.0/screen_wh.x, 1.0/screen_wh.y);

	vec2 offsets[9];
	offsets[0] = vec2(-1.0, -1.0);
	offsets[1] = vec2(-1.0, 0.0);
	offsets[2] = vec2(-1.0, 1.0);
	offsets[3] = vec2(0.0, -1.0);
	offsets[4] = vec2(0.0, 0.0);
	offsets[5] = vec2(0.0, 1.0);
	offsets[6] = vec2(1.0, -1.0);
	offsets[7] = vec2(1.0, 0.0);
  offsets[8] = vec2(1.0, 1.0);

  float best_dist = 9999999.9;
  vec2 best_pos = vec2(0.0);
  vec4 best_data = vec4(0.0);

  for(int i = 0; i < 9; i++) {
    vec2 jump = v_uv + (offsets[i] * (u_offset / screen_wh));
    vec4 seed = texture2D(tex, jump);
    vec2 seedpos = vec2(V2_F16(seed.xy), V2_F16(seed.zw));
    float dist = distance(seedpos, v_uv);

    if(seedpos != vec2(0.0) && dist <= best_dist)
    {
      best_dist = dist;
      best_data = seed;
    }
  }
  
  out_color = best_data;
}