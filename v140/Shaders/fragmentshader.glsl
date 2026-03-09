#version 330 core
out vec4 FragColor;

in vec3 FragLocation;
in vec2 TexCoords;
in vec3 Normal;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material material;

void main()
{
	FragColor = texture(material.diffuse, TexCoords);
}