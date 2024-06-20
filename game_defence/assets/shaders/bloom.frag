#version 460

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D blur_texture;
uniform sampler2D scene_texture;
uniform float exposure;
uniform bool do_bloom;

void
main()
{
  // color = sprite_colour; //* texture(tex, TexCoords);
  // color = sprite_colour * texture(tex, TexCoords);
  vec4 blur_col = texture(blur_texture, v_uv);
  vec4 scene_col = texture(scene_texture, v_uv);
  
  if(do_bloom){
    vec4 blend = scene_col + blur_col;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-blend.rgb * exposure);

    out_color = vec4(result, 1.0);
  } else {
    out_color = scene_col;
  }
  // out_color = blend;1
}