#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;

vec2 F16_V2(float f) { return vec2(floor(f * 255.0) / 255.0, fract(f * 255.0)); }

// this shader stores the uv coordinates in the texture

void
main()
{
  vec4 scene = texture(tex, v_uv);

  // any pixel with .a > 0 will be recognised as an emitter or occluder
  out_color = vec4(F16_V2(v_uv.x * scene.a), F16_V2(v_uv.y * scene.a));
}