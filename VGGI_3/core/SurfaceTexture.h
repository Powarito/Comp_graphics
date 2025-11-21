#pragma once

#include "../geometry/BezierCurve.h"
#include "Surface.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// Manages mapping 2D Bezier curves onto 3D surface
class SurfaceTexture {
public:
    std::vector<std::shared_ptr<BezierCurve>> curves;
    
    // Texture transformation parameters
    glm::vec2 offset = glm::vec2(0.0f);     // UV offset
    float scale = 1.0f;                      // UV scale (local, around center)
    float rotation = 0.0f;                   // Rotation angle (degrees, local)
    
    // Load curves from file
    inline void loadFromFile(const std::string& filename) {
        curves.clear();
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open texture file: " << filename << std::endl;
            return;
        }
        
        std::size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        for (std::size_t i = 0; i < count; ++i) {
            std::array<glm::vec2, 4> controlPoints;
            glm::vec3 color;
            int segments;
            
            file.read(reinterpret_cast<char*>(controlPoints.data()), sizeof(glm::vec2) * 4);
            file.read(reinterpret_cast<char*>(&color), sizeof(glm::vec3));
            file.read(reinterpret_cast<char*>(&segments), sizeof(int));
            
            curves.push_back(std::make_shared<BezierCurve>(
                controlPoints[0], controlPoints[1],
                controlPoints[2], controlPoints[3],
                color, segments
            ));
        }
        
        file.close();
        std::cout << "Loaded " << count << " texture curves from " << filename << std::endl;
        
        // Compute center for local transformations
        computeCenter();
    }
    
    // Compute center of all curves (for local transformations)
    inline void computeCenter() {
        if (curves.empty()) {
            textureCenter = glm::vec2(0.0f);
            return;
        }
        
        glm::vec2 minPt(std::numeric_limits<float>::max());
        glm::vec2 maxPt(std::numeric_limits<float>::lowest());
        
        for (const auto& curve : curves) {
            for (int i = 0; i < 4; ++i) {
                minPt = glm::min(minPt, curve->controlPoints[i]);
                maxPt = glm::max(maxPt, curve->controlPoints[i]);
            }
        }
        
        textureCenter = (minPt + maxPt) * 0.5f;
    }
    
    // Convert 2D point to UV coordinates
    inline glm::vec2 xyToUV(const glm::vec2& xy) const {
        // Base conversion formula from lab description
        float u = xy.x * (glm::pi<float>() / 6.0f) / 10.0f;
        float v = xy.y * (glm::pi<float>() / 6.0f) / 10.0f;
        
        return glm::vec2(u, v);
    }
    
    // Apply texture transformation to UV (LOCAL - around texture center)
    inline glm::vec2 transformUV(const glm::vec2& originalXY) const {
        // Work in original XY space for proper local transform
        glm::vec2 localXY = originalXY - textureCenter;
        
        // Apply rotation around center
        if (std::abs(rotation) > 0.001f) {
            float angle = glm::radians(rotation);
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            
            float x = localXY.x * cosA - localXY.y * sinA;
            float y = localXY.x * sinA + localXY.y * cosA;
            
            localXY = glm::vec2(x, y);
        }
        
        // Apply scale around center
        localXY *= scale;
        
        // Move back to world space
        glm::vec2 transformedXY = localXY + textureCenter;
        
        // Convert to UV
        glm::vec2 uv = xyToUV(transformedXY);
        
        // Apply UV offset
        uv += offset;
        
        return uv;
    }
    
    // Wrap UV coordinates to valid range with proper wrapping
    inline glm::vec2 wrapUV(glm::vec2 uv, const Surface& surface) const {
        float uRange = surface.uMax - surface.uMin;
        float vRange = surface.vMax - surface.vMin;
        
        // Wrap U coordinate (azimuthal - full circle)
        while (uv.x < surface.uMin) uv.x += uRange;
        while (uv.x > surface.uMax) uv.x -= uRange;
        
        // For V coordinate (polar angle), we need special handling at poles
        // Instead of clamping, we reflect at the poles
        while (uv.y < surface.vMin || uv.y > surface.vMax) {
            if (uv.y < surface.vMin) {
                uv.y = surface.vMin + (surface.vMin - uv.y);
                uv.x += uRange / 2.0f; // Flip to opposite side
            }
            if (uv.y > surface.vMax) {
                uv.y = surface.vMax - (uv.y - surface.vMax);
                uv.x += uRange / 2.0f; // Flip to opposite side
            }
            // Re-wrap U after flipping
            while (uv.x < surface.uMin) uv.x += uRange;
            while (uv.x > surface.uMax) uv.x -= uRange;
        }
        
        return uv;
    }
    
    // Generate 3D vertices for texture curves on surface
    // Each curve is stored separately to avoid connecting unrelated curves
    // Returns: vector of curves, each curve is vector of vertices (pos.xyz + color.rgb)
    std::vector<std::vector<float>> generateTextureCurves(const Surface& surface) const {
        std::vector<std::vector<float>> allCurves;
        
        for (const auto& curve : curves) {
            std::vector<float> curveVerts;
            curveVerts.reserve((curve->segments + 1) * 6);
            
            // Generate points along the Bezier curve
            for (int i = 0; i <= curve->segments; ++i) {
                float t = static_cast<float>(i) / curve->segments;
                glm::vec2 point2D = curve->evaluate(t);
                
                // Apply texture transformation (local)
                glm::vec2 uv = transformUV(point2D);
                
                // Wrap UV coordinates properly
                uv = wrapUV(uv, surface);
                
                // Map to 3D surface
                glm::vec3 point3D = surface.evaluate(uv.x, uv.y);
                
                curveVerts.insert(curveVerts.end(), {
                    point3D.x, point3D.y, point3D.z,
                    curve->color.r, curve->color.g, curve->color.b
                });
            }
            
            allCurves.push_back(std::move(curveVerts));
        }
        
        return allCurves;
    }
    
    glm::vec2 getCenter() const { return textureCenter; }
    
private:
    glm::vec2 textureCenter = glm::vec2(0.0f);
};

