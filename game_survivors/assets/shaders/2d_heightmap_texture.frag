// version prepended to file when loaded by engine.
//

out vec4 out_color;

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

uniform sampler2D tex_map_heightmap;

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
  vec2 camera_pos;
	vec4[32] light_positions;
  float time;
  float zoom;
  float tilesize;
};

void main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  vec2 v_vertex = fs_in.v_vertex;
  int index = int(fs_in.v_tex_unit);

  // note: only 50/128 of the texture is used for heightmap
  // the rest is for water normal map
  // so we need to scale the uv coords
  v_uv *= (50.0/128.0);
  // v_uv *= vec2(50.0/128.0, 1.0f);

  float height = texture(tex_map_heightmap, v_uv).r;
  //  if(height > 0.7)
  //   out_color.rgb = vec3(0.0f, 0.0f, 0.0f);
  // else if(height > 0.68)
  //   out_color.rgb = vec3(1.0f, 0.0f, 1.0f);
  // else
    out_color.rgb = vec3(pow(height, 3) - 0.1, 0.0f, 0.0f);

  // out_color.rgb = vec3(1.0f, 1.0f, 0.0f);
  out_color.a = 1.0f;
}