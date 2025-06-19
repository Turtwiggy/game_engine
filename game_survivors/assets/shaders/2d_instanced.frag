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
uniform int RENDERER_TEX_UNIT_COUNT;

// #define FLUIDSIM 0
#ifdef FLUIDSIM 
uniform sampler2D tex_fluid;
uniform int tex_fluid_tex_unit;
uniform float tex_fluid_texel_size;
uniform bool tex_fluid_shading;
#endif

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  // out_colour.a = 1.0f;

  // special case: render the "empty" sprite as just the colour
  if(index == RENDERER_TEX_UNIT_COUNT && (v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f)) {
    out_colour = v_colour;
    out_colour.a = 1.0f;
    return;
  }

  // A spritesheet texture
  {
    // v_uv goes from 0 to 1
    // convert from 0 to 1 to the width/height desired
    vec2 sprite_uv = vec2(
      (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
      (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
    );

    // the engine generates code here e.g. 
  vec4 col = vec4(1.0f);
{{ generate_sampler_if_statements }}
    out_colour = v_colour * col;
  }

#ifdef FLUIDSIM 
  if(index == tex_fluid_tex_unit){
    vec2 uv = v_uv;
    vec3 c = texture(tex_fluid, uv).rgb;
    float a = max(c.r, max(c.g, c.b));
    out_colour = vec4(c, a);
    return;
  }
#endif
}