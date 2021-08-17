#version 330 core
out vec4 out_colour;

in vec3 FragPos;
in vec2 v_tex;
in vec4 v_colour;
in vec2 v_sprite_pos;
in float v_tex_index;

// hard coded for kennynl spritesheet atm
const int num_cols = 48;
const int num_rows = 22;
const float scale_x = 1.0f / num_cols;
const float scale_y = 1.0f / num_rows;

uniform sampler2D textures[3];

uniform bool do_lighting = false;
const int num_lights = 2;
uniform vec3 light_pos[num_lights];

void
main()
{
  // clang-format off
  vec2 sprite_tex_coord = vec2(
    v_tex.x / num_cols + v_sprite_pos.x * scale_x,
    v_tex.y / num_rows + v_sprite_pos.y * scale_y
  );
  // clang-format on

  int index = int(v_tex_index);

  if (do_lighting) {

    vec4 tex_main = texture(textures[1], v_tex);
    vec4 tex_shadow = texture(textures[2], v_tex);

    vec4 r;

    for (int i = 0; i < num_lights; i++) {
      float distance = length(light_pos[i] - FragPos);
      const float light_constant = 1.0f;
      // distance 200
      // const float light_linear = 0.022f;
      // const float light_quadratic = 0.0019f;
      // distance 325
      // const float light_linear = 0.014f;
      // const float light_quadratic = 0.0007f;
      // distance 600
      // const float light_linear = 0.007f;
      // const float light_quadratic = 0.0002f;
      // distance 3250
      const float light_linear = 0.0014;
      const float light_quadratic = 0.000007f;
      float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * (distance * distance));
      vec4 c = tex_main * attenuation;
      r += c;
    }
    vec4 c = r;

    // pixel is in light if (tex_shadow.r == 1.0f) {
    // pixel is in shadow
    if (tex_shadow.r == 0.0f) {
      c = c * vec4(0.92f, 0.92f, 0.92f, 1.0f);
    }

    out_colour = c;
    out_colour.a = 1.0f;
    return;
  }

  vec4 c;
  if (v_sprite_pos.x == 0 && v_sprite_pos.y == 0) {
    c = v_colour * texture(textures[index], v_tex);
  } else if (v_sprite_pos.x == 8 && v_sprite_pos.y == 5) { // the square
    c = v_colour;
  } else {
    c = v_colour * texture(textures[index], sprite_tex_coord);
  }

  out_colour = c;
}