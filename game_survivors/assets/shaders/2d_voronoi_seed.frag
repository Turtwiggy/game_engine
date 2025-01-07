// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;

uniform sampler2D tex;

vec2 F16_V2(float f) { return vec2(floor(f * 255.0) / 255.0, fract(f * 255.0)); }

// this shader stores the uv coordinates in the texture

void
main()
{
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