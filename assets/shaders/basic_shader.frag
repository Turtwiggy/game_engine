#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform bool greyscale;
uniform sampler2D tex;

void
main()
{
  if (greyscale) {
    FragColor = vec4(vec3(texture(tex, TexCoord).r), 1.0);
  } else {
    FragColor = texture(tex, TexCoord);
  }
}
