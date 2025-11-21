#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>

class BezierCurve {
public:
    std::array<glm::vec2, 4>    controlPoints;
    glm::vec3                   color;
    int                         segments;

    inline BezierCurve(
        const glm::vec2&    p0,
        const glm::vec2&    p1,
        const glm::vec2&    p2,
        const glm::vec2&    p3,
        const glm::vec3&    color    = { 0.0f, 0.0f, 0.0f },
        int                 segments = 50
    ) : color(color), segments(segments) {
        controlPoints[0] =  p0;
        controlPoints[1] =  p1;
        controlPoints[2] =  p2;
        controlPoints[3] =  p3;
    }

    inline glm::vec2 evaluate(float t) const {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        glm::vec2 p = uuu * controlPoints[0];
        p += 3.0f * uu * t * controlPoints[1];
        p += 3.0f * u * tt * controlPoints[2];
        p += ttt * controlPoints[3];

        return p;
    }

    inline glm::vec2 derivative(float t) const {
        float u = 1.0f - t;
        float uu = u * u;
        float tt = t * t;

        glm::vec2 d = -3.0f * uu * controlPoints[0];
        d += 3.0f * uu * controlPoints[1] - 6.0f * u * t * controlPoints[1];
        d += 6.0f * u * t * controlPoints[2] - 3.0f * tt * controlPoints[2];
        d += 3.0f * tt * controlPoints[3];

        return d;
    }

    inline std::vector<float> generateCurveVertices() const {
        std::vector<float> vertices;
        vertices.reserve((segments + 1) * 5);

        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / segments;
            glm::vec2 point = evaluate(t);
            vertices.insert(vertices.end(), {
                point.x, point.y,
                color.r, color.g, color.b
                });
        }

        return vertices;
    }

    inline std::vector<float> generateSkeletonLines() const {
        std::vector<float> vertices;
        glm::vec3 skeletonColor = { 0.5f, 0.5f, 0.5f };

        for (int i = 0; i < 3; ++i) {
            vertices.insert(vertices.end(), {
                controlPoints[i].x, controlPoints[i].y,
                skeletonColor.r, skeletonColor.g, skeletonColor.b,
                controlPoints[i + 1].x, controlPoints[i + 1].y,
                skeletonColor.r, skeletonColor.g, skeletonColor.b
                });
        }

        return vertices;
    }

    // Generate skeleton points with optional size override
    inline std::vector<float> generateSkeletonPoints(float sizeMultiplier = 1.0f) const {
        std::vector<float> vertices;

        for (int i = 0; i < 4; ++i) {
            glm::vec3 pointColor = (i == 0 || i == 3) ? glm::vec3{ 1.0f, 0.5f, 0.0f } : glm::vec3{ 1.0f, 0.65f, 0.3f };
            vertices.insert(vertices.end(), {
                controlPoints[i].x, controlPoints[i].y,
                pointColor.r, pointColor.g, pointColor.b
                });
        }

        return vertices;
    }
};

