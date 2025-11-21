#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "auxiliary/ShaderProgram.h"

struct MeshData {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    std::size_t indexCount;
    bool hasEBO;
    int primitiveType;
    float primitiveSize;
};

class Renderer3D {
public:
    // Draw indexed mesh
    static inline void DrawIndexed(
        const ShaderProgram& shader,
        const MeshData& data
    ) {
        shader.use();
        
        if (data.primitiveType == GL_POINTS) {
            glPointSize(data.primitiveSize);
        } else if (data.primitiveType == GL_LINES || data.primitiveType == GL_LINE_STRIP) {
            glLineWidth(data.primitiveSize);
        }
        
        glBindVertexArray(data.VAO);
        
        if (data.hasEBO) {
            glDrawElements(data.primitiveType, data.indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(data.primitiveType, 0, data.indexCount);
        }
        
        glBindVertexArray(0);
    }
    
    // Create mesh with positions, normals, and UVs (for surface)
    static inline MeshData createSurfaceMesh(
        const std::vector<float>& vertices,    // pos(3) + normal(3) + uv(2)
        const std::vector<unsigned int>& indices,
        int primitiveType = GL_TRIANGLES
    ) {
        MeshData data;
        data.indexCount = indices.size();
        data.hasEBO = true;
        data.primitiveType = primitiveType;
        data.primitiveSize = 1.0f;
        
        glGenVertexArrays(1, &data.VAO);
        glGenBuffers(1, &data.VBO);
        glGenBuffers(1, &data.EBO);
        
        glBindVertexArray(data.VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // UV attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
        
        return data;
    }
    
    // Create simple mesh with positions and colors (for lines, axes, texture curves)
    static inline MeshData createSimpleMesh(
        const std::vector<float>& vertices,    // pos(3) + color(3)
        int primitiveType,
        float primitiveSize = 1.0f
    ) {
        MeshData data;
        data.indexCount = vertices.size() / 6;
        data.hasEBO = false;
        data.primitiveType = primitiveType;
        data.primitiveSize = primitiveSize;
        
        glGenVertexArrays(1, &data.VAO);
        glGenBuffers(1, &data.VBO);
        
        glBindVertexArray(data.VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
        
        return data;
    }
    
    // Update simple mesh data
    static inline void updateSimpleMesh(MeshData& data, const std::vector<float>& vertices) {
        data.indexCount = vertices.size() / 6;
        
        glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    // Cleanup mesh data
    static inline void deleteMesh(MeshData& data) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
        if (data.hasEBO) {
            glDeleteBuffers(1, &data.EBO);
        }
    }
};

// Helper: Generate coordinate axes
inline std::vector<float> generateAxes3D(float length) {
    std::vector<float> vertices;
    
    // X axis - red
    vertices.insert(vertices.end(), {
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        length, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f
    });
    
    // Y axis - green
    vertices.insert(vertices.end(), {
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, length, 0.0f,  0.0f, 1.0f, 0.0f
    });
    
    // Z axis - blue
    vertices.insert(vertices.end(), {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, length,  0.0f, 0.0f, 1.0f
    });
    
    return vertices;
}

