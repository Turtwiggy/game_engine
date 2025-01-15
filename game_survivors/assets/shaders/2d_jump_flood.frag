// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  float v_tex_unit;
  vec2 v_vertex;
} fs_in;

uniform float u_offset;
uniform vec2 screen_wh;
uniform sampler2D tex;

float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);
  // vec2 SCREEN_PIXEL_SIZE = vec2(1.0/screen_wh.x, 1.0/screen_wh.y);

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
  
  float closest_dist = 9999999.9;
  vec4 closest_data = vec4(0.0);

  for(int i = 0; i < 9; i++) {
    // vec2 voffset = v_uv;
    // voffset += offsets[i] * SCREEN_PIXEL_SIZE * u_offset;
    
    vec2 jump = v_uv + (offsets[i] * (u_offset / screen_wh));
    vec4 seed = texture(tex, jump);
    vec2 seedpos = vec2(V2_F16(seed.xy), V2_F16(seed.zw));
    float dist = distance(seedpos, v_uv);

    if(seedpos != vec2(0.0) && dist <= closest_dist){
      closest_dist = dist;
      closest_data = seed;
    }
  }

  out_colour = closest_data;
}