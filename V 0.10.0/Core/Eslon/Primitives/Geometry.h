#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//Transform
glm::vec3 Location = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 Scale = glm::vec3(0.0f, 0.0f, 0.0f);

class Geometry
{
		
};
class Primitives : public Geometry
{
public:
	void Box(glm::vec3 Location, float edgeLenght)
	{
		float halfside = edgeLenght * 0.5f;
		float vertex[] =
		{
		// VERTEX DATA
		//Front Face
		Location.x - halfside, Location.y + halfside, Location.z + halfside, // Top Left
		Location.x + halfside, Location.y + halfside, Location.z + halfside, // Top Right
		Location.x + halfside, Location.y - halfside, Location.z + halfside, // Bottom Left
		Location.x - halfside, Location.y - halfside, Location.z + halfside, // Bottom Right
		//Back Face
		Location.x - halfside, Location.y + halfside, Location.z - halfside, // Top Left
		Location.x + halfside, Location.y + halfside, Location.z - halfside, // Top Right
		Location.x + halfside, Location.y - halfside, Location.z - halfside, // Bottom Left
		Location.x - halfside, Location.y - halfside, Location.z - halfside, // Bottom Right
		//Left Face
		Location.x - halfside, Location.y + halfside, Location.z + halfside, // Top Left
		Location.x - halfside, Location.y + halfside, Location.z - halfside, // Top Right
		Location.x - halfside, Location.y - halfside, Location.z - halfside, // Bottom Left
		Location.x - halfside, Location.y - halfside, Location.z + halfside, // Bottom Right
		//Right Face
		Location.x + halfside, Location.y + halfside, Location.z + halfside, // Top Left
		Location.x + halfside, Location.y + halfside, Location.z - halfside, // Top Right
		Location.x + halfside, Location.y - halfside, Location.z - halfside, // Bottom Left
		Location.x + halfside, Location.y - halfside, Location.z + halfside, // Bottom Right
		//Top Face
		Location.x - halfside, Location.y + halfside, Location.z + halfside, // Top Left
		Location.x - halfside, Location.y + halfside, Location.z - halfside, // Top Right
		Location.x + halfside, Location.y + halfside, Location.z - halfside, // Bottom Left
		Location.x + halfside, Location.y + halfside, Location.z + halfside, // Bottom Right
		//Bottom Face
		Location.x - halfside, Location.y - halfside, Location.z + halfside, // Top Left
		Location.x - halfside, Location.y - halfside, Location.z - halfside, // Top Right
		Location.x + halfside, Location.y - halfside, Location.z - halfside, // Bottom Left
		Location.x + halfside, Location.y - halfside, Location.z + halfside, // Bottom Right

		};
		unsigned int VBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

		glBindVertexArray(VAO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(VAO), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION, 0, 24);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
};

#endif