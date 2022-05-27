#version 330 core
layout (location = 0 ) in vec3 Location;
layout (location = 1 ) in vec3 NormalSurface; 

out vec3 FragLocation;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragLocation = vec3(model * vec4(Location, 1.0));
	Normal = mat3(transpose(inverse(model))) * NormalSurface;

	gl_Position = projection * view * vec4(FragLocation, 1.0);
}