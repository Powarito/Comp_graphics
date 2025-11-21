#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "auxiliary/ShaderProgram.h"
#include "Renderer.h"
#include "../geometry/BezierCurve.h"
#include "ContourManager.h"
#include "../geometry/GenerateGrid.h"
#include "../math/Transforms.h"
#include <functional>
#include <memory>

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
    void createDefaultCurves();

    void updateGridBuffer();
    void updateCurvesBuffers();
    void updatePointsBuffer();
    void updateDraggablePoints();

    void ImGuiNewFrame();
    void renderScene();
    void renderUI();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow* window);
    void onScroll(double yOffset);
    void processPanning();
    bool processUIWantCaptureMouse();

    glm::vec2 screenToWorld(double mouseX, double mouseY);
    glm::vec2 worldToTransformed(const glm::vec2& worldPos);
    glm::vec2 transformedToWorld(const glm::vec2& transformedPos);

    void terminateGL();
    void terminateImGui();

    // Triangle animation
    void startTriangleAnimation();
    void updateAnimation(float deltaTime);
    void resetToOriginal();

    // Curve selection by clicking
    void handleCurveSelection();
    int findCurveAtPosition(const glm::vec2& worldPos);

    void createNewCurve();
    void deleteSelectedCurve();

    // Smart smoothness
    void handleSmartSmoothness();
    int findCurveEndpoint(const glm::vec2& worldPos, std::size_t& curveIdx, bool& isEnd);

private:
    // Program status
    GLFWwindow*     window          = nullptr;
    unsigned int    width           = 1280;
    unsigned int    height          = 720;

    bool            isDragging      = false;
    int             draggedPointIdx = -1;
    glm::vec2       dragOffset      = { 0.0f, 0.0f };

    bool            isPanning       = false;
    glm::vec2       panStartMouse;
    glm::vec2       panStartOrigin;

    // Grid graphic parameters
    glm::vec2       origin          = { 640.0f, 360.0f };
    float           pixelsPerUnit   = 50.0f;
    std::size_t     gridNum         = 20;

    // Transformations
    glm::vec2       offset          = { 0.0f, 0.0f };
    glm::vec2       rotatePoint     = { 0.0f, 0.0f };
    float           angleDegrees    = 0.0f;

    // Draggable Points
    struct DraggablePoint {
        glm::vec2*                  position;
        std::function<void()>       onUpdate;

        DraggablePoint(
            glm::vec2*              position,
            std::function<void()>   callback = nullptr
        )
            : position(position)
            , onUpdate(std::move(callback))
        {
        }
    };
    std::vector<DraggablePoint>     draggablePoints;

    // Contour Manager
    ContourManager                  contourManager;
    bool                            showSkeleton         = true;
    int                             selectedCurveIdx     = -1;

    // Animation
    bool                            isAnimating          = false;
    float                           animationTime        = 0.0f;
    float                           animationDuration    = 2.0f;
    bool                            animateToTriangle    = true;
    float                           lastFrameTime        = 0.0f;

    struct OriginalPoints {
        std::vector<std::array<glm::vec2, 4>> controlPoints;
    };
    OriginalPoints                  originalPoints;

    // Smart smoothness
    int                             hoveredEndpointCurve = -1;
    bool                            hoveredIsEnd         = false;

    // Colors
    glm::vec3       bgColor          = { 1.0f, 1.0f, 1.0f };
    glm::vec3       gridColor        = { 0.85f, 0.85f, 0.85f };
    glm::vec3       axisXColor       = { 1.0f, 0.0f, 0.0f };
    glm::vec3       axisYColor       = { 0.0f, 1.0f, 0.0f };
    glm::vec3       rotatePointColor = { 1.0f, 0.8f, 0.2f };

    // Transformation Matrices
    glm::mat4       projection;
    glm::mat4       moveOrigin;
    glm::mat4       scaleToPixels;
    glm::mat4       rotationMatrix;
    glm::mat4       transformMatrix;

    // GPU resources
    ShaderProgram*  shader          = nullptr;
    RenderData      gridRenderData;
    RenderData      axesRenderData;
    RenderData      rotatePointRenderData;

    std::vector<RenderData>         curveRenderDataList;
    std::vector<RenderData>         skeletonLinesList;
    std::vector<RenderData>         skeletonPointsList;
};

