#pragma once

#include "../geometry/BezierCurve.h"
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

class ContourManager {
public:
    std::vector<std::shared_ptr<BezierCurve>> curves;

    inline void addCurve(const BezierCurve& curve) {
        curves.push_back(std::make_shared<BezierCurve>(curve));
    }

    inline void removeCurve(std::size_t index) {
        if (index < curves.size()) {
            curves.erase(curves.begin() + index);
        }
    }

    inline void clear() {
        curves.clear();
    }

    // curve1End: true = use P3 of curve1, false = use P0 of curve1
    // curve2End: true = use P3 of curve2, false = use P0 of curve2
    inline void makeC1Smooth(
        std::size_t curve1Idx, 
        std::size_t curve2Idx, 
        bool        curve1End, 
        bool        curve2End
    ) {
        if (curve1Idx >= curves.size() || curve2Idx >= curves.size()) return;
        if (curve1Idx == curve2Idx) return;

        auto& c1 = curves[curve1Idx];
        auto& c2 = curves[curve2Idx];

        int idx1_connect = curve1End ? 3 : 0;
        int idx1_control = curve1End ? 2 : 1;
        int idx2_connect = curve2End ? 3 : 0;
        int idx2_control = curve2End ? 2 : 1;

        glm::vec2& p1_connect = c1->controlPoints[idx1_connect];
        glm::vec2& p1_control = c1->controlPoints[idx1_control];
        glm::vec2& p2_connect = c2->controlPoints[idx2_connect];
        glm::vec2& p2_control = c2->controlPoints[idx2_control];

        // C0: Move to the second curve's point
        p1_connect = p2_connect;

        // C1: Mirroring the control point of the first curve to preserve the tangent direction
        glm::vec2 reflected = 2.0f * p2_connect - p2_control;
        p1_control = reflected;

        // Alternative approach:
        // C1: Continuity of the tangent direction
        // Tangent to the second curve (fixed)
        //glm::vec2 r2_prime;
        //if (!curve2End) {
        //    r2_prime = 3.0f * (c2->controlPoints[1] - c2->controlPoints[0]); // r2'(0)
        //}
        //else {
        //    r2_prime = 3.0f * (c2->controlPoints[3] - c2->controlPoints[2]); // r2'(1)
        //}
        //
        //float a2 = glm::length(r2_prime);
        //if (a2 < 1e-6f) {
        //    r2_prime = glm::vec2(1.0f, 0.0f);
        //    a2 = 1.0f;
        //}
        //
        //glm::vec2 dir = r2_prime / a2;
        //
        //if (curve1End == curve2End) {
        //    dir = -dir;
        //}
        //
        //glm::vec2 desired_r1_prime = dir * a2;
        //
        //if (curve1End) {
        //    // r1'(1) = 3*(P3 - P2) -> P2 = P3 - r1'/3
        //    p1_control = p1_connect - desired_r1_prime / 3.0f;
        //}
        //else {
        //    // r1'(0) = 3*(P1 - P0) -> P1 = P0 + r1'/3
        //    p1_control = p1_connect + desired_r1_prime / 3.0f;
        //}
    }

    // Save to binary file
    inline void saveToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for writing: " << filename << std::endl;
            return;
        }

        std::size_t count = curves.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));

        for (const auto& curve : curves) {
            file.write(reinterpret_cast<const char*>(curve->controlPoints.data()), sizeof(glm::vec2) * 4);
            file.write(reinterpret_cast<const char*>(&curve->color), sizeof(glm::vec3));
            file.write(reinterpret_cast<const char*>(&curve->segments), sizeof(int));
        }

        file.close();
        std::cout << "Saved " << count << " curves to " << filename << std::endl;
    }

    // Load from binary file
    inline void loadFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for reading: " << filename << std::endl;
            return;
        }

        curves.clear();

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
        std::cout << "Loaded " << count << " curves from " << filename << std::endl;
    }
};

