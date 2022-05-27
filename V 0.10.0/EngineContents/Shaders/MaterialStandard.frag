#version 330 core
out vec4 FragColor;

struct Material
{
	vec3 ambiant;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
struct Light
{
	vec3 location;
	vec3 ambiant;
	vec3 diffuse;
	vec3 specular;
};

in vec3 Normal;
in vec3 FragLocation;

uniform vec3 ViewLocation;
uniform Material material;
uniform Light light;

void main()
{	
	//Ambiant
	vec3 ambiant = light.ambiant * material.ambiant;
	//Diffuse
	vec3 norm = normalize(Normal);
	vec3 LightDirection = normalize(light.location - FragLocation);
	float diff = max(dot(norm, LightDirection), 0.0);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);
	//Specular
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	vec3 ReflectionDirection = reflect(-LightDirection, Normal);
	float spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);
	//Final Color;
	vec3 Result = ambiant + diffuse + specular;
	FragColor = vec4(Result, 1.0);
}