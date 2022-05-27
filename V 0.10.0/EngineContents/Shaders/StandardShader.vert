#version 330 core
layout (location = 0 ) in vec3 Location;			// Vertices location
layout (location = 1 ) in vec3 NormalSurface;		// Vertices Normal Surface
layout (location = 2) in vec2 TextureCoordinate;	// Vertices Texture Coordinate

out vec3 FragLocation;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//FragLocation = vec3(model * vec4(Location, 1.0));
	//Normal = mat3(transpose(inverse(model))) * NormalSurface;
	Normal = NormalSurface;
	TexCoords = TextureCoordinate;
	gl_Position = projection * view * model * vec4(FragLocation, 1.0);
}