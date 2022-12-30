#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

uniform vec3 diffuse;
uniform float specular;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition.rgb = FragPos;
    gPosition.a = 1.0;
    // also store the per-fragment normals into the gbuffer
    gNormal.rgb = normalize(Normal);
    gNormal.a = 1.0;
    // and the diffuse per-fragment color
    //gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    gAlbedoSpec.rgb = diffuse;
    // store specular intensity in gAlbedoSpec's alpha component
    //gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
    gAlbedoSpec.a = specular;
}
