#pragma once

#include "random-rankit.h"			//Necessary library with ready types and structures
#include "equinox-bake-opengl.h"	//Shader
#include "lightning-backend.h"
#include "lightning-stream.h"

#include <assimp/Importer.hpp>		// * Assimp
#include <assimp/scene.h>			// ********
#include <assimp/postprocess.h>		// *********

#include <glm/gtc/matrix_transform.hpp>

using namespace Random::Structs;

namespace Lightning
{
	namespace Model
	{
		class Primitive
		{
		public:
			std::vector<Vertex>		vertices;
			std::vector<uint>		 indices;
			std::vector<Texture>	 textures;

			Primitive(std::vector<Vertex> vertices, std::vector<uint> indices, std::vector<Texture> textures);
			void Draw(Shader* shader);
		private:
			Console console;
			uint VAO, VBO, EBO;
		private:
			void MakePrimitive();
		};
		class Geometry 
		{
		public:
			Geometry();
			~Geometry();
			void Draw(Shader* shader);
			void AddVertex(v3 VertexLocation);
			void AddUV(v2 TextureCordinate);
			void AddNormal(v3 NormalPosition);
		private:
			
		};
		class HardMesh
		{
			//Public Functions
		public:
			HardMesh(text const& path, bool gamma = false);
			HardMesh();
			~HardMesh() { --countmodel; }
			//Rendering Mesh
			void Draw(Shader* shader);
			void Load(text const& path);
			void SetVisible(bool newVisibility);
			Transform getTransform();
			void setTransform(Transform NewTransfrom);
			v3 getLocation();
			void setLocation(v3 NewLocation);
			r3 getRotation();
			void setRotation(r3 NewRotation);
			v3 getScale();
			void setScale(v3 NewScale);
			static int countmodel;
			//Private Variables
		private:
			Console						   console;
			Transform					 transform;
			Transform						 pivot;
			std::vector<Texture>    texturesLoaded;
			std::vector<Primitive>		    meshes;
			text						 directory;
			bool		   gammacorrection = false;
			bool					visible = true;
		private:
			void LoadMesh(text const& path);
			void ProcessNode(aiNode* node, const aiScene* scene);
			Primitive ProcessMesh(aiMesh* mesh, const aiScene* scene);
			std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, text TypeName);
		};
		class FoldMesh
		{};
	}
}