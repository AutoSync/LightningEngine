#include "Geometry.h"

Lightning::GeometryComponent::GeometryComponent()
{
	vector<Vertex>		_vertices;	//Empty Vertex
	vector<uint>		_indices;	//Empty Index
	vector<Texture>		_textures;	//Empty Texture
	MeshInitialize(_vertices, _indices, _textures);
}

Lightning::GeometryComponent::GeometryComponent(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures)
{
	MeshInitialize(_vertices, _indices, _textures);
}

void Lightning::GeometryComponent::Render(Shader* shader)
{
    uint DiffuseN = 1;
    uint SpecularN = 1;
    uint NormalN = 1;
    uint HeightN = 1;

    for (uint i = 0; i < textures.size(); i++)
    {
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(DiffuseN++);
        else if (name == "texture_specular")
            number = std::to_string(SpecularN++);
        else if (name == "texture_normal")
            number = std::to_string(NormalN++);
        else if (name == "texture_height")
            number = std::to_string(HeightN++);
        glUniform1i(glGetUniformLocation(shader->Id, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].Id);
    }
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);;
}

void Lightning::GeometryComponent::MeshInitialize(vector<Vertex> _vertices, vector<uint> _indices, vector<Texture> _textures)
{
	this->vertices = _vertices;
	this->indices = _indices;
	this->textures = _textures;
	MeshBuild();
}

void Lightning::GeometryComponent::MeshBuild()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

Lightning::MeshComponent::MeshComponent(string const& path, bool gamma)
{
    Count();
    console.Init();
    console.Log("IMPORT MESH INSTANCE" + std::to_string(countmodel) + " ->START ");
    LoadMesh(path);
    pivot.Rotation = V3(0.5, 0.5f, 0.5f);
    console.Log("IMPORT MESH -> COMPLETED");
}

Lightning::MeshComponent::MeshComponent()
{
    Count();
    console.Log("MESH INSTANCE" + std::to_string(countmodel) + " -> INICIALIZATION");
}

void Lightning::MeshComponent::Draw(Shader* shader)
{
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(transform.Position.x, transform.Position.y, transform.Position.z));
    shader->SetMat4("model", model);

    if (visible)
    {
        for (uint i = 0; i < meshes.size(); i++)
        {
            meshes[i].Render(shader);
        }
    }
}

void Lightning::MeshComponent::Load(string const& path)
{
    LoadMesh(path);
}

void Lightning::MeshComponent::SetVisible(bool newVisibility)
{
    this->visible = newVisibility;
}

void Lightning::MeshComponent::Count()
{
    ++countmodel;
}

void Lightning::MeshComponent::LoadMesh(string const& path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        string error_msg = importer.GetErrorString();
        Msg::Emit(Flow::OUTPUT, "ERROR::IMPORT-MODEL::ASSIMP:: " + error_msg);
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene);
}

void Lightning::MeshComponent::ProcessNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (uint i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (uint i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Lightning::GeometryComponent Lightning::MeshComponent::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    std::vector<Vertex>  vertices;
    std::vector<uint>	  indices;
    std::vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (uint i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        V3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            V2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = V2(0.0f, 0.0f);
        }

        // tangent
        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.Tangent = vector;
        // bitangent
        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        vertex.Bitangent = vector;


        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (uint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return GeometryComponent(vertices, indices, textures);
}

std::vector<Lightning::Texture> Lightning::MeshComponent::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string TypeName)
{
    std::vector<Texture> textures;
    Image* importer = new Image();
    for (uint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (uint j = 0; j < texturesLoaded.size(); j++)
        {
            if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(texturesLoaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;

            texture.Id = importer->TextureFromFile(str.C_Str(), this->directory);
            texture.type = TypeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}
