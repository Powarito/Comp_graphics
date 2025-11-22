#pragma once

#include <string>
#include <map>
#include <vector>
#include <stack>
#include <glm/glm.hpp>

struct LSystemConfig {
    std::string                 axiom;
    std::map<char, std::string> rules;
    float                       angle       = 90.0f;            // Rotation angle in degrees
    glm::vec2                   forward     = { 1.0f, 0.0f };   // Forward direction vector (length = step size)
};

struct TurtleState {
    glm::vec2                   position    = { 0.0f, 0.0f };
    float                       direction   = 0.0f;             // Angle in radians
};

class LSystem {
public:
    LSystem() = default;

    bool loadFromFile(const std::string& filename);

    inline void setConfig(const LSystemConfig& config) { this->config = config; }
    const inline LSystemConfig& getConfig() const { return config; }

    // Generate string after n iterations
    std::string generate(int iterations) const;

    // Convert generated string to line vertices
    // Returns vertices in format: x, y, r, g, b (5 floats per vertex)
    std::vector<float> generateVertices(
        int                     iterations,
        const glm::vec3&        color,
        const glm::vec2&        startPos    = { 0.0f, 0.0f },
        float                   startAngle  = 0.0f
    ) const;

    // Get bounding box of generated fractal
    void getBoundingBox(
        int                     iterations,
        glm::vec2&              minBound,
        glm::vec2&              maxBound
    ) const;

private:
    LSystemConfig               config;

    // Helper function to interpret the string and generate points
    std::vector<glm::vec2> interpretString(
        const std::string&      str,
        const glm::vec2&        startPos,
        float                   startAngle
    ) const;
};

