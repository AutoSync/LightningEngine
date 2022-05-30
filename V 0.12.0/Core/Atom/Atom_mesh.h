#include "../../SystemStart.h"
#define GLMATH

#ifndef  MESH_ATOM
#define MESH_ATOM

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#include "Atom_Shader.h"

using namespace std;

struct Vertex
{
	glm::vec3 Location;		// Location Vertex
	glm::vec3 Normal;		// Normal Vertex
	glm::vec2 TexCoords;	// Texture Cordinates
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture
{
	unsigned int id;
	string type;
	string path;
};

class Mesh
{
public: 
	vector<Vertex>			vertices;
	vector<unsigned int>	indices;
	vector<Texture>			textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
	void Draw(Shader Shader);
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};
#endif // ! MESH_ATOM

