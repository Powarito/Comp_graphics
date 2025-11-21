#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <cmath>

// Parametric surface definition
class Surface {
public:
    using SurfaceFunction = std::function<glm::vec3(float u, float v)>;
    
    SurfaceFunction surfaceFunc;
    float uMin, uMax;
    float vMin, vMax;
    int uSegments, vSegments;
    
    Surface(
        SurfaceFunction func,
        float uMin, float uMax,
        float vMin, float vMax,
        int uSegments = 50,
        int vSegments = 50
    ) : surfaceFunc(func),
        uMin(uMin), uMax(uMax),
        vMin(vMin), vMax(vMax),
        uSegments(uSegments), vSegments(vSegments)
    {
    }
    
    // Evaluate surface at (u, v)
    inline glm::vec3 evaluate(float u, float v) const {
        return surfaceFunc(u, v);
    }
    
    // Generate mesh vertices with normals and UV coordinates
    // Returns: position.xyz, normal.xyz, uv.xy (8 floats per vertex)
    std::vector<float> generateMeshVertices() const {
        std::vector<float> vertices;
        vertices.reserve((uSegments + 1) * (vSegments + 1) * 8);
        
        float uStep = (uMax - uMin) / uSegments;
        float vStep = (vMax - vMin) / vSegments;
        
        for (int i = 0; i <= vSegments; ++i) {
            float v = vMin + i * vStep;
            
            for (int j = 0; j <= uSegments; ++j) {
                float u = uMin + j * uStep;
                
                glm::vec3 pos = evaluate(u, v);
                glm::vec3 normal = computeNormal(u, v);
                
                // Normalize UV to [0, 1] range
                float uvU = (u - uMin) / (uMax - uMin);
                float uvV = (v - vMin) / (vMax - vMin);
                
                vertices.insert(vertices.end(), {
                    pos.x, pos.y, pos.z,
                    normal.x, normal.y, normal.z,
                    uvU, uvV
                });
            }
        }
        
        return vertices;
    }
    
    // Generate indices for triangle mesh
    std::vector<unsigned int> generateMeshIndices() const {
        std::vector<unsigned int> indices;
        indices.reserve(uSegments * vSegments * 6);
        
        for (int i = 0; i < vSegments; ++i) {
            for (int j = 0; j < uSegments; ++j) {
                unsigned int topLeft = i * (uSegments + 1) + j;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (i + 1) * (uSegments + 1) + j;
                unsigned int bottomRight = bottomLeft + 1;
                
                // First triangle
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);
                
                // Second triangle
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
        
        return indices;
    }
    
    // Generate wireframe edges
    std::vector<float> generateWireframeVertices() const {
        std::vector<float> vertices;
        
        float uStep = (uMax - uMin) / uSegments;
        float vStep = (vMax - vMin) / vSegments;
        
        // U-direction lines
        for (int i = 0; i <= vSegments; ++i) {
            float v = vMin + i * vStep;
            
            for (int j = 0; j < uSegments; ++j) {
                float u1 = uMin + j * uStep;
                float u2 = uMin + (j + 1) * uStep;
                
                glm::vec3 p1 = evaluate(u1, v);
                glm::vec3 p2 = evaluate(u2, v);
                
                vertices.insert(vertices.end(), {
                    p1.x, p1.y, p1.z,
                    p2.x, p2.y, p2.z
                });
            }
        }
        
        // V-direction lines
        for (int j = 0; j <= uSegments; ++j) {
            float u = uMin + j * uStep;
            
            for (int i = 0; i < vSegments; ++i) {
                float v1 = vMin + i * vStep;
                float v2 = vMin + (i + 1) * vStep;
                
                glm::vec3 p1 = evaluate(u, v1);
                glm::vec3 p2 = evaluate(u, v2);
                
                vertices.insert(vertices.end(), {
                    p1.x, p1.y, p1.z,
                    p2.x, p2.y, p2.z
                });
            }
        }
        
        return vertices;
    }
    
private:
    // Compute normal using partial derivatives
    inline glm::vec3 computeNormal(float u, float v) const {
        const float eps = 0.001f;
        
        glm::vec3 p = evaluate(u, v);
        glm::vec3 pu = evaluate(u + eps, v) - p;
        glm::vec3 pv = evaluate(u, v + eps) - p;
        
        glm::vec3 normal = glm::cross(pu, pv);
        
        if (glm::length(normal) > 1e-6f) {
            return glm::normalize(normal);
        }
        
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
};

// Factory functions for specific surfaces

// Pear surface based on sphere with radius R
inline Surface createPearSurface(float R, int segments = 50) {
    auto pearFunc = [R](float u, float v) -> glm::vec3 {
        // u is azimuthal angle [0, 2*PI]
        // v is polar angle [0, PI]
        
        // Sphere coordinates
        float x_sphere = R * std::sin(v) * std::cos(u);
        float y_sphere = R * std::sin(v) * std::sin(u);
        float z_sphere = R * std::cos(v);
        
        // Pear transformation
        float x = x_sphere;
        float y = y_sphere;
        float z;
        
        if (z_sphere < R / 2.0f) {
            z = z_sphere;
        } else {
            float t = z_sphere / R - 0.5f;
            z = z_sphere + 2.5f * R * t * t;
        }
        
        return glm::vec3(x, y, z);
    };
    
    return Surface(pearFunc, 0.0f, 2.0f * glm::pi<float>(), 0.0f, glm::pi<float>(), segments, segments);
}

// Sphere surface for testing
inline Surface createSphereSurface(float R, int segments = 50) {
    auto sphereFunc = [R](float u, float v) -> glm::vec3 {
        float x = R * std::sin(v) * std::cos(u);
        float y = R * std::sin(v) * std::sin(u);
        float z = R * std::cos(v);
        return glm::vec3(x, y, z);
    };
    
    return Surface(sphereFunc, 0.0f, 2.0f * glm::pi<float>(), 0.0f, glm::pi<float>(), segments, segments);
}

