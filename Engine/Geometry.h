#pragma once
#include "Shader.h"
#include "Types.h"
#include "SceneComponent.h"
#include "Msg.h"
#include "Image.h"
#include "System.h"


#include <assimp/Importer.hpp>		// * Assimp
#include <assimp/scene.h>			// ********
#include <assimp/postprocess.h>		// *********

using namespace Lightning::Msg;
namespace Lightning
{
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

	class MeshComponent : SceneComponent
	{
	private:
		Console						   console;
		//Transform					 transform;
		Transform						 pivot;
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
		int countmodel = 0;
	private:
		void Count();
		void LoadMesh(string const& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		GeometryComponent ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string TypeName);
	};

}

