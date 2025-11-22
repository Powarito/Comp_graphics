#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ProjectionType {
    DIMETRIC,
    PERSPECTIVE,
    ORTHOGRAPHIC
};

class ProjectionManager {
public:
    ProjectionType currentProjection = ProjectionType::DIMETRIC;

    // Dimetric parameters - ideal dimetric angles by default
    float dimetricAngleY = -180.0f / glm::pi<float>() * glm::atan(1.0f / glm::sqrt(7.0f));
    float dimetricAngleX = 180.0f / glm::pi<float>() * glm::atan(1.0f / (2.0f * glm::sqrt(2.0f)));

    // Orthographic parameters (used for dimetric and orthographic)
    float orthoSize = 10.0f;
    float orthoNear = -100.0f;
    float orthoFar = 100.0f;

    // Perspective parameters
    float perspectiveFov = 45.0f;
    float perspectiveNear = 0.1f;
    float perspectiveFar = 1000.0f;

    // Get projection matrix based on current type
    inline glm::mat4 getProjectionMatrix(float aspectRatio) const {
        switch (currentProjection) {
            case ProjectionType::DIMETRIC:
            case ProjectionType::ORTHOGRAPHIC:
                return getOrthographicProjection(aspectRatio);
            case ProjectionType::PERSPECTIVE:
                return getPerspectiveProjection(aspectRatio);
        }
        return glm::mat4(1.0f);
    }

    // Get view matrix for dimetric projection
    inline glm::mat4 getDimetricViewMatrix() const {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(dimetricAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(dimetricAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
        return view;
    }

    // Reset to ideal dimetric angles
    inline void resetDimetricAngles() {
        dimetricAngleY = -180.0f / glm::pi<float>() * glm::atan(1.0f / glm::sqrt(7.0f));
        dimetricAngleX = 180.0f / glm::pi<float>() * glm::atan(1.0f / (2.0f * glm::sqrt(2.0f)));
    }

    // Get name of current projection
    inline const char* getProjectionName() const {
        switch (currentProjection) {
            case ProjectionType::DIMETRIC: return "Dimetric";
            case ProjectionType::PERSPECTIVE: return "Perspective";
            case ProjectionType::ORTHOGRAPHIC: return "Orthographic";
        }
        return "Unknown";
    }

private:
    // Orthographic projection
    inline glm::mat4 getOrthographicProjection(float aspectRatio) const {
        float halfSize = orthoSize;
        return glm::ortho(
            -halfSize * aspectRatio, halfSize * aspectRatio,
            -halfSize, halfSize,
            orthoNear, orthoFar
        );
    }

    // Standard perspective projection
    inline glm::mat4 getPerspectiveProjection(float aspectRatio) const {
        return glm::perspective(
            glm::radians(perspectiveFov),
            aspectRatio,
            perspectiveNear,
            perspectiveFar
        );
    }
};

