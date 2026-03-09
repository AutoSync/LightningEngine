#include "../../../Shapes.h"

namespace LightningEngine
{
	Shapes::Shapes() : VAO(0), VBO(0), EBO(0), color(C3(1.0f, 1.0f, 1.0f))
	{
		SetupBuffers();
	}
	Shapes::~Shapes()
	{
		ClearBuffers();
	}
	void Shapes::SetupBuffers()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}
	void Shapes::ClearBuffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
	void Shapes::DrawRect(V2 pos, V2 size, const C3& drawColor)
	{
		vertices.clear();
		indices.clear();

		vertices = {
			pos.x,          pos.y,           drawColor.r, drawColor.g, drawColor.b,
			pos.x + size.x, pos.y,           drawColor.r, drawColor.g, drawColor.b,
			pos.x + size.x, pos.y + size.y,  drawColor.r, drawColor.g, drawColor.b,
			pos.x,          pos.y + size.y,  drawColor.r, drawColor.g, drawColor.b
		};

		indices = {
			0, 1, 2,
			2, 3, 0
		};

		color = drawColor;
		// Setuo Buffers
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	void Shapes::DrawCircle(V2 center, float radius, const C3& drawColor, int segments)
	{
		vertices.clear();
		indices.clear();
		// Center vertex
		vertices.push_back(center.x);
		vertices.push_back(center.y);
		vertices.push_back(drawColor.r);
		vertices.push_back(drawColor.g);
		vertices.push_back(drawColor.b);
		// Circle vertices
		for (int i = 0; i <= segments; ++i)
		{
			float theta = 2.0f * 3.1415926f * float(i) / float(segments);
			float x = radius * cosf(theta);
			float y = radius * sinf(theta);
			vertices.push_back(center.x + x);
			vertices.push_back(center.y + y);
			vertices.push_back(drawColor.r);
			vertices.push_back(drawColor.g);
			vertices.push_back(drawColor.b);
		}
		// Indices
		for (int i = 1; i <= segments; ++i)
		{
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
		color = drawColor;
		// Setup Buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		// Position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Color attribute
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	void Shapes::setColor(const C3& newColor)
	{
		this->color = newColor;
	}
}