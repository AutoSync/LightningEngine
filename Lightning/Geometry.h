#pragma once
#include "Shader.h"
#include "Types.h"
#include "Msg.h"
#include "Image.h"


#include <assimp/Importer.hpp>		// * Assimp
#include <assimp/scene.h>			// ********
#include <assimp/postprocess.h>		// *********

using namespace Lightning::Msg;
namespace Lightning
{
	struct Vertex
	{
		V3 Position;		// Position Vertex
		V2 TexCoords;		// Texture Cordinates
		V3 Normal;			// Normal Vertex 
		V3 Tangent;			// Tangents Vertex 
		V3 Bitangent;		// Bitangens Vertex
	};
	struct Texture
	{
		uint	 Id;
		string type;
		string path;
	};

	class GeometryComponent
	{
	private:
		vector<Vertex>		vertices;
		vector<uint>		 indices;
		vector<Texture>		textures;
		uint VAO = 0, VBO = 0, EBO = 0;
	public:
		GeometryComponent();
		GeometryComponent(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures);
		void Render(Shader* shader);

	private:
		void MeshInitialize(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures);
		void MeshBuild();
	};

	class MeshComponent
	{
	private:
		Console						   console;
		Transform					 transform;
		Transform						 pivot;
		glm::mat4						 model;
		vector<Texture>			texturesLoaded;
		vector<GeometryComponent>		meshes;
		string						 directory;
		bool		   gammacorrection = false;
		bool					visible = true;
	public:
		MeshComponent();
		MeshComponent(string const& path, bool gamma = false);
		~MeshComponent() { --countmodel; }
		//Rendering Mesh
		void Draw(Shader* shader);
		void Load(string const& path);
		void SetVisible(bool newVisibility);
		static int countmodel;
	private:
		void Count();
		void LoadMesh(string const& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		GeometryComponent ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string TypeName);
	};

}

