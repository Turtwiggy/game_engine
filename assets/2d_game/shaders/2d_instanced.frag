#version 330 core
out vec4 out_colour;

in vec3 FragPos;
in vec2 v_tex;
in vec4 v_colour;
in vec2 v_sprite_pos;
in float v_tex_index;

uniform bool do_pixel = false;
uniform sampler2D textures[3];
uniform bool do_lighting = false;
const int num_lights = 2;
uniform vec3 light_pos[num_lights];
uniform float light_linear[num_lights];
uniform float light_quadratic[num_lights];

// hard coded for kennynl spritesheet atm
const int num_cols = 48;
const int num_rows = 22;
const float scale_x = 1.0f / num_cols;
const float scale_y = 1.0f / num_rows;
const float w = 768.0f;
const float h = 352.0f;

void
main()
{
  int index = int(v_tex_index);

  if (do_lighting) {
    vec4 tex_main = texture(textures[1], v_tex);
    vec4 tex_shadow = texture(textures[2], v_tex);
    vec4 r;

    for (int i = 0; i < num_lights; i++) {
      float distance = length(light_pos[i] - FragPos);
      const float light_constant = 1.0f;
      float linear = light_linear[i];
      float quadratic = light_quadratic[i];
      float attenuation = 1.0 / (light_constant + linear * distance + quadratic * (distance * distance));
      vec4 c = tex_main * attenuation;
      r += c;
    }
    vec4 c = r;

    // pixel is in shadow
    if (tex_shadow.r == 0.0f) {
      c = c * vec4(0.92f, 0.92f, 0.92f, 1.0f);
    }

    out_colour = c;
    out_colour.a = 1.0f;
    return;
  }

  vec4 c;
  if (v_sprite_pos.x == 0 && v_sprite_pos.y == 0) { // a whole texture
    c = v_colour * texture(textures[index], v_tex);
    out_colour = c;
    return;
  } else if (v_sprite_pos.x == 8 && v_sprite_pos.y == 5) { // the square on the kennynl sprite sheet
    c = v_colour;
    out_colour = c;
    return;
  }

  if (do_pixel) {
    // pixel shader attempt 1
    // https://csantosbh.wordpress.com/2014/02/05/automatically-detecting-the-texture-filter-threshold-for-pixelated-magnifications/
    // i have no idea if below is correct
    vec2 vUv = v_tex * vec2(w, h);
    vec2 alpha = 0.7 * vec2(dFdx(vUv.x), dFdy(vUv.y));
    vec2 x = fract(vUv);
    vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
    vec2 uv = (floor(vUv) + x_) / vec2(w, h);

    // clang-format off
    vec2 sprite_tex_coord = vec2(
      uv.x / num_cols + v_sprite_pos.x * scale_x,
      uv.y / num_rows + v_sprite_pos.y * scale_y
    );
    // clang-format on

    out_colour = v_colour * texture(textures[0], sprite_tex_coord);
    return;
  }
}