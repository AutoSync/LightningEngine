#version 330 core
out vec4 FragColor;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
struct Light
{
	//vec3 location;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
in vec3 FragLocation;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 ViewLocation;
uniform Material material;
uniform Light light;

void main()
{	
	//Ambiant
	// ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(light.position - FragPos);
    vec3 lightDir = normalize(-light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(ViewLocation - FragLocation);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}