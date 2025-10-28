#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

// Calculates the new figurePosition after rotating around rotatePoint
inline glm::vec2 computeRotatedPosition(
    glm::vec2           figurePosition,
    glm::vec2           rotatePoint,
    float               angleDegrees
) {
    float angle = glm::radians(angleDegrees);
    glm::vec2 translated = figurePosition - rotatePoint;
    glm::vec2 rotated = glm::rotate(translated, angle);
    return rotatePoint + rotated;
}

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

// Mirror a 2D object relative to an arbitrary point according to mirrorX and mirrorY flags
inline glm::mat4 mirrorAroundPoint2D(
    const glm::vec2&    mirrorPoint,
    bool                mirrorX,
    bool                mirrorY
) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(mirrorPoint, 0.0f));

    float sx = mirrorX ? -1.0f : 1.0f;
    float sy = mirrorY ? -1.0f : 1.0f;
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

    model = glm::translate(model, glm::vec3(-mirrorPoint, 0.0f));

    return model;
}

// Scale a 2D object relative to an arbitrary point
inline glm::mat4 scaleAroundPoint2D(
    const glm::vec2& scale,
    const glm::vec2& scalePoint
) {
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(scalePoint, 0.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
    model = glm::translate(model, glm::vec3(-scalePoint, 0.0f));

    return model;
}

// Calculates the general affine transformation matrix
inline glm::mat4 computeTransformMatrix(
    const glm::vec2&    figurePosition,
    const glm::vec2&    rotatePoint,
    float               angleDegrees,
    const glm::vec2&    scale,
    bool                mirrorX,
    bool                mirrorY
) {
    glm::mat4 model(1.0f);

    model = rotateAroundPoint2D(angleDegrees, rotatePoint);
    model *= mirrorAroundPoint2D(rotatePoint, mirrorX, mirrorY);
    model *= scaleAroundPoint2D(scale, figurePosition);
    model = glm::translate(model, glm::vec3(figurePosition, 0.0f));

    return model;
}

// Build an affine matrix from a new coordinate basis
inline glm::mat4 buildAffineBasis2D(
    const glm::vec2&    O,   // beginning of a new coordinate system
    const glm::vec2&    X,   // end point of a unit segment of the X axis
    const glm::vec2&    Y    // end point of a unit segment of the Y axis
) {
    glm::vec2 i = X - O;
    glm::vec2 j = Y - O;

    glm::mat4 M(1.0f);

    M[0][0] = i.x;
    M[0][1] = i.y;

    M[1][0] = j.x;
    M[1][1] = j.y;

    M[3][0] = O.x;
    M[3][1] = O.y;

    return M;
}

