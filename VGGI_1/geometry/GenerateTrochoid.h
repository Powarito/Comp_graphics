#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cmath>

// Генерує параметричну криву трохоїди
inline std::vector<float> generateTrochoidVertices(
    float               r,          // radius of a circle
    float               h,          // distance of a point from the center of a circle
    float               tMax,       // upper limit of the parameter t
    float               dt,         // step t
    const glm::vec3&    color       // line color
) {
    std::vector<float> verts;
    verts.reserve(static_cast<std::size_t>(std::ceil(tMax / dt)) * 5);

    for (float t = 0.0f; t <= tMax; t += dt) {
        float x = r * t - h * std::sin(t);
        float y = r - h * std::cos(t);

        verts.insert(verts.end(), { x, y, color.r, color.g, color.b });
    }

    return verts;
}

