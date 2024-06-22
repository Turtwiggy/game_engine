#version 460 core

out vec4 colour;

in vec3 o_fragpos;
in vec2 o_uv;
in vec3 o_normal;
in vec4 o_colour;

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light
{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 camera_pos;
uniform sampler2D texture_diffuse;

void
main()
{
  // ambient
  vec3 ambient = light.ambient * texture(texture_diffuse, o_uv).rgb * material.ambient;

  // diffuse
  vec3 norm = normalize(o_normal);
  vec3 lightDir = normalize(-light.direction);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(texture_diffuse, o_uv).rgb * material.diffuse;

  // specular
  vec3 viewDir = normalize(camera_pos - o_fragpos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * material.specular;

  vec3 result = ambient + diffuse + specular;
  colour = vec4(result, 1.0);
}