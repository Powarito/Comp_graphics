#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "auxiliary/ShaderProgram.h"

struct RenderData {
    unsigned int    VAO;
    unsigned int    VBO;
    std::size_t     vertexCount;
    int             primitiveType;
    float           primitiveSize;
};

class Renderer {
public:
    static inline void Draw(
        const ShaderProgram&        shader, 
        const RenderData&           data
    ) {
        shader.use();

        auto sizeFunc = data.primitiveType == GL_POINTS ? glPointSize : glLineWidth;
        sizeFunc(data.primitiveSize);

        glBindVertexArray(data.VAO);
        glDrawArrays(data.primitiveType, 0, data.vertexCount);
        glBindVertexArray(0);
    }

    static inline RenderData createRenderData(
        const std::vector<float>&   vertices, 
        int                         primitiveType,
        float                       primitiveSize,
        int                         usage
    ) {
        RenderData renderData;

        glGenVertexArrays(1, &renderData.VAO);
        glGenBuffers(1, &renderData.VBO);
        renderData.vertexCount   = vertices.size() / 5;
        renderData.primitiveType = primitiveType;
        renderData.primitiveSize = primitiveSize;

        glBindVertexArray(renderData.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, renderData.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), usage);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        return renderData;
    }
};

