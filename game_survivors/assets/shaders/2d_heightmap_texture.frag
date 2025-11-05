// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_size; // e.g. 16, 16
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  vec2 v_sprite_global_pos;
  float v_tex_unit;
} fs_in;

uniform sampler2D tex_map_heightmap;
uniform float used_tex_w;

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
	vec4[32] light_positions;
  vec2 camera_pos;
  vec2 screenshake;
  float time;
  float zoom;
  float tilesize;
};

float scale(float value, float inMin, float inMax, float outMin, float outMax) {
  float clampedValue = clamp(value, inMin, inMax);
  float normalized = (clampedValue - inMin) / (inMax - inMin);
  return outMin + normalized * (outMax - outMin);
}

void main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  vec2 uv = v_uv;

  // note: only some of the heightmap texture is used
  uv *= (used_tex_w/vec2(textureSize(tex_map_heightmap, 0)).x);
  
  float height = texture(tex_map_heightmap, uv).r;
  
  //  if(height > 0.7)
  //   out_colour.rgb = vec3(0.0f, 0.0f, 0.0f);
  // else if(height > 0.68)
  //   out_colour.rgb = vec3(1.0f, 0.0f, 1.0f);
  // else

  // if(height >= 0.64) {
  //   // out_colour.r = 1.0 - 0.64;
  //   // out_colour.r = pow(height, 2) - 0.1;
  //   // out_colour.r = 1.0;
  //   // out_colour.a = 0.5f;
  //   // out_colour.a = 1.0f;
  //   // return;
  // }
  // else{
  //   // out_colour.r = 1.0f;
  //   // scale [0, X] to [1, 0];
  //   // out_colour.r =  scale(height, 0.0, 0.60, 0.0, 1.0);
  //   // out_colour.gb = vec2(0.0f);
  //   // out_colour.r = 1.0 - height;
  // }

  // out_colour.r = 0.6 - height;
  out_colour.r = height;

  // out_colour.rgb = vec3(1.0f, 1.0f, 0.0f);
  out_colour.a = 1.0f;
}