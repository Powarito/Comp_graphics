#pragma once

#include <glm/glm.hpp>

// Material properties for Phong shading
struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    // Default constructor with wood material properties
    Material()
        : ambient(0.2f, 0.1f, 0.0f)
        , diffuse(0.4f, 0.2f, 0.1f)
        , specular(0.02f, 0.02f, 0.02f)
        , shininess(10.0f)
    {}

    // Reset to default wood material
    void resetToWood() {
        ambient = glm::vec3(0.2f, 0.1f, 0.0f);
        diffuse = glm::vec3(0.4f, 0.2f, 0.1f);
        specular = glm::vec3(0.02f, 0.02f, 0.02f);
        shininess = 10.0f;
    }
};

// Point light properties
struct PointLight {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    PointLight()
        : position(5.0f, 10.0f, 5.0f)
        , ambient(0.2f, 0.2f, 0.2f)
        , diffuse(1.0f, 1.0f, 1.0f)
        , specular(1.0f, 1.0f, 1.0f)
        , constant(1.0f)
        , linear(0.045f)
        , quadratic(0.0075f)
    {}

    // Reset to default values
    void reset() {
        position = glm::vec3(5.0f, 10.0f, 5.0f);
        ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
        specular = glm::vec3(1.0f, 1.0f, 1.0f);
        constant = 1.0f;
        linear = 0.045f;
        quadratic = 0.0075f;
    }
};

