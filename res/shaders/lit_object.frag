#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  

uniform vec3 light_position; 
uniform vec3 view_position; 
uniform vec3 light_colour;
uniform vec3 object_colour;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light_colour;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_colour;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(view_position - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light_colour;  
        
    vec3 result = (ambient + diffuse + specular) * object_colour;
    FragColor = vec4(result, 1.0);
} 
