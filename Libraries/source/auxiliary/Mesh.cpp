#include "auxiliary/Mesh.h"


void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

void Mesh::Draw(const ShaderProgram &shaderProgram) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int emissiveNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    bool useEmission;

    for (int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        
        std::string number;
        std::string name = textures[i].type;
        
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_emissive")
            number = std::to_string(emissiveNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        shaderProgram.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    if (emissiveNr > 1)
        shaderProgram.setBool("useEmission", true);
    else
        shaderProgram.setBool("useEmission", false);

    // Draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    int maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    
    // We should unbind textures from units after drawing a model
    for (int i = 0; i < maxTextureUnits; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);

    //for (int i = 1; i < diffuseNr; ++i) shaderProgram.setInt(("material.texture_diffuse" + std::to_string(i)).c_str(), -1);
    //for (int i = 1; i < specularNr; ++i) shaderProgram.setInt(("material.texture_specular" + std::to_string(i)).c_str(), -1);
    //for (int i = 1; i < emissiveNr; ++i) shaderProgram.setInt(("material.texture_emissive" + std::to_string(i)).c_str(), -1);
    //for (int i = 1; i < normalNr; ++i) shaderProgram.setInt(("material.texture_normal" + std::to_string(i)).c_str(), -1);
    //for (int i = 1; i < heightNr; ++i) shaderProgram.setInt(("material.texture_height" + std::to_string(i)).c_str(), -1);
}