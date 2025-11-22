#include "LSystem.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

bool LSystem::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open L-System file: " << filename << std::endl;
        return false;
    }

    config.rules.clear();
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        // Parse axiom
        if (line.rfind("axiom=", 0) == 0) {
            config.axiom = line.substr(6);
        }
        // Parse angle
        else if (line.rfind("angle=", 0) == 0) {
            config.angle = std::stof(line.substr(6));
        }
        // Parse forward vector (format: forward=x,y)
        else if (line.rfind("forward=", 0) == 0) {
            std::string coords = line.substr(8);
            size_t comma = coords.find(',');
            if (comma != std::string::npos) {
                config.forward.x = std::stof(coords.substr(0, comma));
                config.forward.y = std::stof(coords.substr(comma + 1));
            }
        }
        // Parse rules (format: X=XF-F+F-XF...)
        else {
            size_t eq = line.find('=');
            if (eq != std::string::npos && eq > 0) {
                char var = line[0];
                std::string replacement = line.substr(eq + 1);
                config.rules[var] = replacement;
            }
        }
    }

    file.close();
    return true;
}

std::string LSystem::generate(int iterations) const {
    std::string current = config.axiom;

    for (int i = 0; i < iterations; ++i) {
        std::string next;
        next.reserve(current.size() * 4);

        for (char c : current) {
            auto it = config.rules.find(c);
            if (it != config.rules.end()) {
                next += it->second;
            }
            else {
                next += c;
            }
        }
        current = std::move(next);
    }

    return current;
}

std::vector<float> LSystem::generateVertices(
    int                     iterations,
    const glm::vec3&        color,
    const glm::vec2&        startPos,
    float startAngle
) const {
    std::string generated = generate(iterations);
    std::vector<glm::vec2> segments = interpretString(generated, startPos, startAngle);

    std::vector<float> vertices;
    vertices.reserve(segments.size() * 5);

    // segments contains pairs: [start1, end1, start2, end2, ...]
    for (const auto& p : segments) {
        vertices.insert(vertices.end(), { p.x, p.y, color.r, color.g, color.b });
    }

    return vertices;
}

void LSystem::getBoundingBox(
    int                     iterations,
    glm::vec2&              minBound,
    glm::vec2&              maxBound
) const {
    std::string generated = generate(iterations);
    std::vector<glm::vec2> segments = interpretString(generated, { 0, 0 }, 0);

    minBound = glm::vec2(std::numeric_limits<float>::max());
    maxBound = glm::vec2(std::numeric_limits<float>::lowest());

    for (const auto& p : segments) {
        minBound = glm::min(minBound, p);
        maxBound = glm::max(maxBound, p);
    }

    // Handle empty fractal
    if (segments.empty()) {
        minBound = maxBound = glm::vec2(0);
    }
}

// Returns pairs of points (start, end) for each line segment
// This works with GL_LINES primitive
std::vector<glm::vec2> LSystem::interpretString(
    const std::string&      str,
    const glm::vec2&        startPos,
    float                   startAngle
) const {
    std::vector<glm::vec2> segments; // Pairs: [start1, end1, start2, end2, ...]
    std::stack<TurtleState> stateStack;

    TurtleState turtle;
    turtle.position  = startPos;
    turtle.direction = startAngle;

    float stepLength = glm::length(config.forward);
    float angleRad = glm::radians(config.angle);

    for (char c : str) {
        switch (c) {
        case 'F':
        case 'G': {
            // Move forward and draw line segment
            glm::vec2 oldPos = turtle.position;
            float dx = stepLength * std::cos(turtle.direction);
            float dy = stepLength * std::sin(turtle.direction);
            turtle.position += glm::vec2(dx, dy);

            // Add line segment (pair of points)
            segments.push_back(oldPos);
            segments.push_back(turtle.position);
            break;
        }
        case 'f':
        case 'g': {
            // Move forward WITHOUT drawing
            float dx = stepLength * std::cos(turtle.direction);
            float dy = stepLength * std::sin(turtle.direction);
            turtle.position += glm::vec2(dx, dy);
            // No segment added
            break;
        }
        case '+':
            // Turn right (clockwise)
            turtle.direction -= angleRad;
            break;
        case '-':
            // Turn left (counter-clockwise)
            turtle.direction += angleRad;
            break;
        case '[':
            // Push current state to stack (for branching)
            stateStack.push(turtle);
            break;
        case ']':
            // Pop state from stack (return to branch point)
            if (!stateStack.empty()) {
                turtle = stateStack.top();
                stateStack.pop();
            }
            break;
        default:
            // Variables like X, Y are ignored during drawing
            break;
        }
    }

    return segments;
}

