#pragma once

#include <glm/glm.hpp>
#include <vector>

// Generates grid lines (without axes)
inline std::vector<float> generateGridVertices(
    std::size_t         gridNum,
    const glm::vec3&    colorGrid
) {
    const float step = 1.0f; // 1 unit
    const std::size_t gridLines = gridNum + 1;
    const float axisLenX = step * gridNum;
    const float axisLenY = step * gridNum;

    std::vector<float> verts;
    verts.reserve(gridLines * 4 * 5); // 4 vertices per iteration, 5 values per vertex: (x,y,r,g,b)

    for (std::size_t i = 0; i < gridLines; ++i) {
        float y = i * step;
        float x = i * step;

        // Horizontal lines
        verts.insert(verts.end(), {
            0.0f,     y, colorGrid.r, colorGrid.g, colorGrid.b,
            axisLenX, y, colorGrid.r, colorGrid.g, colorGrid.b
            });

        // Vertical lines
        verts.insert(verts.end(), {
            x, 0.0f,     colorGrid.r, colorGrid.g, colorGrid.b,
            x, axisLenY, colorGrid.r, colorGrid.g, colorGrid.b
            });
    }

    return verts;
}

// Generates X and Y axes
inline std::vector<float> generateAxesVertices(
    std::size_t         gridNum,
    const glm::vec3&    colorAxisX,
    const glm::vec3&    colorAxisY
) {
    const float step = 1.0f; // 1 unit
    const float axisLenX = step * gridNum;
    const float axisLenY = step * gridNum;

    std::vector<float> verts;
    verts.reserve(4 * 5); // 2 vertices for X, 2 for Y, 5 values per vertex: (x,y,r,g,b)

    // Axis X
    verts.insert(verts.end(), {
        0.0f,     0.0f, colorAxisX.r, colorAxisX.g, colorAxisX.b,
        axisLenX, 0.0f, colorAxisX.r, colorAxisX.g, colorAxisX.b
        });

    // Axis Y
    verts.insert(verts.end(), {
        0.0f, 0.0f,     colorAxisY.r, colorAxisY.g, colorAxisY.b,
        0.0f, axisLenY, colorAxisY.r, colorAxisY.g, colorAxisY.b
        });

    return verts;
}

