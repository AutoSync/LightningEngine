#pragma once
#include "Shader.h"
#include "Types.h"
#include <vector>


namespace Lightning
{
	struct Vertex
	{
		V3 Position;
		V3 Normal;
		V3 Tangent;
		V3 Binormals;
		V2 TexCoords;
	};
	struct Texture
	{
		uint Id;
		string type;
	};

	class MeshComponent
	{
	private:
		vector<Vertex>		vertices;
		vector<uint>		 indices;
		vector<Texture>		textures;
		uint VAO = 0, VBO = 0, EBO = 0;
	public:
		MeshComponent();
		MeshComponent(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures);
		void Render(Shader& shader);

	private:
		void MeshInitialize(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures);
		void MeshBuild();
	};
}

