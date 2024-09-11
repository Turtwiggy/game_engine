// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

// uniform float time;
uniform vec2 viewport_wh;
uniform vec2 camera_pos;
uniform sampler2D tex;
uniform float iTime;

void
main()
{
  vec2 fragCoord = v_uv * viewport_wh * 2.0f;
  fragCoord.x *= -1.0f;
  fragCoord.y *= -1.0f;
  
  // https://www.shadertoy.com/view/7lyyzd
  vec2 I = fragCoord;

  vec2 zoom = I/4000;
  vec2 scroll = vec2(iTime/1000);
  vec2 pos = (camera_pos + vec2(1000, -1000)) / 1000; // offset so never aligns

  vec4 O = v_colour;

  // Clear the fragcolor, texture sample with parallax, iterate 
  for(O-=O; O.r < texture(tex, zoom - pos/O.r/100 - scroll).r; O+=0.02f);

  // out_colour = vec4(1.0f) - O;
  out_colour.rgb = O.rgb;
  out_colour.a = 1.0f;
  return;
}