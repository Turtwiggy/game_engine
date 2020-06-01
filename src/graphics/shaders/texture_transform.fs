#version 330 core
layout(location = 0) out vec4 color;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
// linearly interpolate between both textures (80% container, 20% awesomeface)
//color = texture(u_Texture, TexCoord) * u_Color;
color = u_Color;
//FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}