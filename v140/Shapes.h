#pragma once
#include "glad/glad.h"
#include "Types.h"


namespace LightningEngine
{
	class Shapes
	{
	private:
		unsigned int VAO, VBO, EBO;
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		C3 color;
	public:
		Shapes();
		~Shapes();

		void DrawRect(V2 pos, V2 size, const C3& drawColor);
		void DrawCircle(V2 center, float radius, const C3& drawColor, int segments = 36);
		void setColor(const C3& newColor);

	private:
		void SetupBuffers();
		void ClearBuffers();
	};
}
