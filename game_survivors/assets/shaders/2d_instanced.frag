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

// this key is replaced by the engine with
// e.g. "uniform sampler2D tex_monochrome_transparent_packed"
{{ generate_user_samplers }}

uniform sampler2D tex_fluid;
uniform int tex_fluid_tex_unit;
uniform int RENDERER_TEX_UNIT_COUNT;

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  // A spritesheet texture
  {
    // v_uv goes from 0 to 1
    // convert from 0 to 1 to the width/height desired
    vec2 sprite_uv = vec2(
      (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
      (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
    );

    out_colour = v_colour;

    // bias, range -4, 4
    float bias = -0.75;
    // aa_scale, range: 0.75, 10
    float aa_scale = 1.25;

    // the engine generates code here e.g. 
    // if(index == RENDERER_TEX_UNIT_COUNT)
    //   out_colour *= tex2dss(tex_monochrome_transparent_packed, sprite_uv, bias, aa_scale);
{{ generate_sampler_if_statements }}

  }

  if(index == tex_fluid_tex_unit){
    vec2 uv = v_uv;
    vec3 col = texture2D(tex_fluid, uv).rgb;
    float len = length(col);
    out_colour = vec4(col, len);
    return;
  }

  // Sample texture directly
  if ((v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f)) { // a whole texture
    out_colour = v_colour;
    return;
  }

}