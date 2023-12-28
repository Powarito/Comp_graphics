#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "ShaderProgram.h"

#include <string>
#include <vector>


class Model {
public:
    // if there're slashes in path, they should be '\\', NOT '/'!
    explicit inline Model(const std::string &path, unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace) {
        loadModel(path, flags);
    }

    void Draw(const ShaderProgram &shaderProgram);

    inline std::string getDirectory() const { return this->directory; }

protected:
    // Model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> texturesLoaded;

    void loadModel(const std::string &path, unsigned int flags);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);
};


unsigned int TextureFromFile(const std::string &path);

unsigned int inline TextureFromFile(const std::string& relPath, const std::string& directory) {
    return TextureFromFile(directory + '\\' + relPath);
}