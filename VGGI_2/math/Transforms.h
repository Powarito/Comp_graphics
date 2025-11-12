#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

// Creates a rotation matrix around an arbitrary point.
// Basically the same as translate to rotatePoint -> rotate for angleDegrees -> translate back,
// but as one matrix
inline glm::mat4 rotateAroundPoint2D(
    float               angleDegrees,
    const glm::vec2&    rotatePoint
) {
    float a = glm::radians(angleDegrees);
    float cosA = std::cos(a);
    float sinA = std::sin(a);

    float pX = rotatePoint.x;
    float pY = rotatePoint.y;

    glm::mat4 M(1.0f);

    M[0][0] = cosA;
    M[0][1] = sinA;
    M[1][0] = -sinA;
    M[1][1] = cosA;

    M[3][0] = -pX * (cosA - 1.0f) + pY * sinA;
    M[3][1] = -pX * sinA - pY * (cosA - 1.0f);

    return M;
}

