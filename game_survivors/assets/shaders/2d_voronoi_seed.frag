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

uniform sampler2D tex;

vec2 F16_V2(float f) { return vec2(floor(f * 255.0) / 255.0, fract(f * 255.0)); }

// this shader stores the uv coordinates in the texture

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);
  
  vec4 scene = texture(tex, v_uv);

  // for debugging
  // out_colour = vec4(v_uv.x * scene.a, v_uv.y * scene.a, 0.0, 1.0);
  
  if(scene.a > 0.0)
    out_colour = vec4(1.0);
  else
    out_colour = vec4(0.0, 0.0, 0.0, 1.0);

  // any pixel with .a > 0 will be recognised as an emitter or occluder
  out_colour = vec4(F16_V2(v_uv.x * scene.a), F16_V2(v_uv.y * scene.a));
}