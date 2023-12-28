#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include "ShaderProgram.h"

#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent; // vector perpendicular to the tangent vector and the normal vector
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};


class Mesh {
protected:
    // Mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    // Rendering data
    unsigned int VAO, VBO, EBO;

    void setupMesh();

public:
    explicit inline Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<Texture> &textures)
        : vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();
    }

    void Draw(const ShaderProgram &shaderProgram);

    // Get mesh data
    inline std::vector<Vertex>       getVertices() const { return this->vertices; }
    inline std::vector<unsigned int> getIndices()  const { return this->indices; }
    inline std::vector<Texture>      getTextures() const { return this->textures; }

    inline unsigned int getVAO() const { return this->VAO; }
    inline unsigned int getVBO() const { return this->VBO; }
    inline unsigned int getEBO() const { return this->EBO; }
};