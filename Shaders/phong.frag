#version 460 core

in vec3 normal;  
in vec3 fragPos;  

out vec4 FragColor;
  
uniform vec3 viewPos; 
uniform vec4 color;
uniform vec3 lightPos;

const vec3 lightColor = vec3(1.f, 1.f, 1.f);

const float ambientStrength = 0.01f;
const float specularStrength = 0.5f;

void main()
{
    vec3 ambient = ambientStrength * lightColor;
  	
    vec3 norm = -normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.f);
    vec3 diffuse = diff * lightColor;
    
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.f), 2.f);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * color.xyz;

    FragColor = vec4(result, color.w);
} 