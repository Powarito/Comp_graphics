#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "auxiliary/ShaderProgram.h"
#include "Renderer.h"
#include "../geometry/GenerateGrid.h"
#include "../geometry/GenerateTrochoid.h"
#include "../math/Transforms.h"
#include <functional>

class App {
public:
    App(unsigned int width, unsigned int height);
    ~App();

    void Run();

private:
    void initGL();
    void initImGui();
    void initBuffers();
    void initDraggablePoints();

    void updateTrochoidBuffer();
    void updateGridBuffer();
    void updatePointsBuffer();
    void updateDraggablePoints();
    void processPanning();
    bool processUIWantCaptureMouse();

    void ImGuiNewFrame();
    void renderScene();
    void renderUI();
    
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow* window);
    glm::vec2 screenToWorld(double mouseX, double mouseY, bool useAffine);

    void terminateGL();
    void terminateImGui();

private:
    // Program status
    GLFWwindow*     window              = nullptr;
    unsigned int    width               = 1280;
    unsigned int    height              = 720;

    bool            isDragging          = false;
    int             draggedPointIdx     = -1;
    glm::vec2       dragOffset          = { 0.0f, 0.0f };

    bool            isPanning           = false;
    glm::vec2       panStartMouse;
    glm::vec2       panStartOrigin;

    // Grid graphic parameters
    glm::vec2       origin              = { 150.0f, 150.0f };;
    float           pixelsPerUnit       = 50.0f;
    std::size_t     gridNum             = 20;

    // Geometry and Transformations
    glm::vec2       O                   = { 0.0f, 0.0f };
    glm::vec2       X                   = { 1.0f, 0.0f };
    glm::vec2       Y                   = { 0.0f, 1.0f };

    glm::vec2       figurePositionBase  = { 3.0f, 2.0f};
    glm::vec2       figurePosition      = figurePositionBase;
    glm::vec2       rotatePoint         = { 1.0f, 1.0f };
    float           angleDegrees        = 0.0f;
    glm::vec2       scale               = { 1.0f, 1.0f };
    bool            isMirrored          = false;

    // Draggable Points
    struct DraggablePoint {
        glm::vec2*                  position;
        glm::vec3                   color;
        bool                        useAffine;
        std::function<void()>       onUpdate;

        DraggablePoint(
            glm::vec2*              position,
            glm::vec3               color,
            bool                    useAffine   = true,
            std::function<void()>   callback    = nullptr
        )
            : position(position)
            , color(color)
            , useAffine(useAffine)
            , onUpdate(std::move(callback))
        {}
    };
    std::vector<DraggablePoint>     draggablePoints;

    // Figure - Trochoid
    struct TrochoidParameters {
        float       r                   = 1.0f;
        float       h                   = 1.0f;
        float       tMax                = 25.0f;
        float       dt                  = 0.01f;
    };
    TrochoidParameters              trochoidParams;

    // Colors
    glm::vec3       bgColor             = { 1.0f, 1.0f, 1.0f };
    glm::vec3       gridColor           = { 0.75f, 0.75f, 0.75f };
    glm::vec3       axisXColor          = { 1.0f, 0.0f, 0.0f };
    glm::vec3       axisYColor          = { 0.0f, 1.0f, 0.0f };
    glm::vec3       trochoidColor       = { 0.9f, 0.2f, 1.0f };
    glm::vec3       rotatePointColor    = { 1.0f, 0.8f, 0.2f };

    // Transformation Matrices
    glm::mat4       projection;
    glm::mat4       moveOrigin;
    glm::mat4       scaleToPixels;
    glm::mat4       affineBasis;

    // GPU resources
    ShaderProgram*  shader              = nullptr;
    RenderData      gridRenderData;
    RenderData      axesRenderData;
    RenderData      trochoidRenderData;
    RenderData      pointsFigureRenderData;
    RenderData      pointsBasisRenderData;
};

