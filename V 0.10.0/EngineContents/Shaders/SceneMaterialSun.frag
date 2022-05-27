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
	vec3 direction; //Direction to 
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragLocation;

uniform vec3 ViewLocation;
uniform Material material;
uniform Light light;

void main()
{	
	
	//Ambiant
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
	//Diffuse
	vec3 norm = normalize(Normal);
	//vec3 LightDirection = normalize(light.location - FragLocation);
	vec3 LightDirection = normalize(-light.direction);
	float diff = max(dot(norm, LightDirection), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
	//Specular
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	vec3 ReflectionDirection = reflect(-LightDirection, norm);
	float spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
	//Final Color;
	vec3 Result = ambient + diffuse + specular;
	FragColor = vec4(Result, 1.0);
}