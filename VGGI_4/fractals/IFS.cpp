#include "IFS.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

const std::vector<glm::vec3> IFS::defaultColors = {
    {1.0f, 0.2f, 0.2f},     // Red
    {0.2f, 1.0f, 0.2f},     // Green
    {0.2f, 0.2f, 1.0f},     // Blue
    {1.0f, 1.0f, 0.2f},     // Yellow
    {1.0f, 0.2f, 1.0f},     // Magenta
    {0.2f, 1.0f, 1.0f},     // Cyan
    {1.0f, 0.6f, 0.2f},     // Orange
    {0.6f, 0.2f, 1.0f},     // Purple
};

IFS::IFS() : dist(0.0f, 1.0f) {
    resetSeed();
}

bool IFS::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open IFS file: " << filename << std::endl;
        return false;
    }

    transforms.clear();
    std::string line;
    int colorIdx = 0;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        IFSTransform t;

        // Read 7 values: a, b, d, e, c, f, p
        // Note: file format is a, b, d, e, c, f, p
        if (iss >> t.a >> t.b >> t.d >> t.e >> t.c >> t.f >> t.p) {
            t.color = defaultColors[colorIdx % defaultColors.size()];
            transforms.push_back(t);
            colorIdx++;
        }
    }

    file.close();

    if (transforms.empty()) {
        std::cerr << "No transforms loaded from file" << std::endl;
        return false;
    }

    // Normalize probabilities if needed
    float totalP = 0.0f;
    for (const auto& t : transforms) {
        totalP += t.p;
    }

    if (std::abs(totalP - 1.0f) > 0.01f) {
        std::cout << "Warning: probabilities sum to " << totalP
            << ", normalizing..." << std::endl;
        for (auto& t : transforms) {
            t.p /= totalP;
        }
    }

    return true;
}

void IFS::setTransformColors(const std::vector<glm::vec3>& colors) {
    for (size_t i = 0; i < transforms.size() && i < colors.size(); ++i) {
        transforms[i].color = colors[i];
    }
}

void IFS::resetSeed(unsigned int seed) {
    rng.seed(seed);
}

std::vector<float> IFS::generateVertices(
    int                 numPoints,
    const glm::vec2&    startPoint
) {
    std::vector<float> vertices;
    vertices.reserve(numPoints * 5);

    if (transforms.empty()) {
        return vertices;
    }

    glm::vec2 current = startPoint;

    // Skip first iterations (transient/warm-up phase).
    // The starting point (0,0) may be far from the fractal's attractor.
    // These iterations let the point "converge" to the attractor set before we start recording points for display.
    // Without this, we might see random dots flying from origin to the fractal.
    const int warmupIterations = 20;
    for (int i = 0; i < warmupIterations; ++i) {
        int idx = selectTransform();
        current = applyTransform(current, idx);
    }

    // Generate visible points
    // Each iteration: randomly select a transform based on probability, apply it to current point, record the result.
    // More points = denser fill of the same fractal shape (attractor).
    for (int i = 0; i < numPoints; ++i) {
        int idx = selectTransform();
        current = applyTransform(current, idx);

        // Color indicates which transform generated this point
        const glm::vec3& color = transforms[idx].color;
        vertices.insert(vertices.end(), {
            current.x, current.y,
            color.r, color.g, color.b
            });
    }

    return vertices;
}

void IFS::getBoundingBox(
    int                 numPoints,
    glm::vec2&          minBound,
    glm::vec2&          maxBound
) {
    minBound = glm::vec2(std::numeric_limits<float>::max());
    maxBound = glm::vec2(std::numeric_limits<float>::lowest());

    if (transforms.empty()) {
        minBound = maxBound = glm::vec2(0);
        return;
    }

    glm::vec2 current(0.0f);

    // Skip transient
    for (int i = 0; i < 20; ++i) {
        int idx = selectTransform();
        current = applyTransform(current, idx);
    }

    // Sample points
    for (int i = 0; i < numPoints; ++i) {
        int idx = selectTransform();
        current = applyTransform(current, idx);

        minBound = glm::min(minBound, current);
        maxBound = glm::max(maxBound, current);
    }
}

int IFS::selectTransform() {
    float r = dist(rng);
    float cumulative = 0.0f;

    for (size_t i = 0; i < transforms.size(); ++i) {
        cumulative += transforms[i].p;
        if (r <= cumulative) {
            return static_cast<int>(i);
        }
    }

    return static_cast<int>(transforms.size() - 1);
}

glm::vec2 IFS::applyTransform(const glm::vec2& point, int idx) const {
    const auto& t = transforms[idx];
    return glm::vec2(
        t.a * point.x + t.b * point.y + t.c,
        t.d * point.x + t.e * point.y + t.f
    );
}

