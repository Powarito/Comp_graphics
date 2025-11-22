#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "auxiliary/ShaderProgram.h"
#include "Renderer.h"
#include "../geometry/GenerateGrid.h"
#include "../fractals/LSystem.h"
#include "../fractals/IFS.h"

class App {
public:
    App(unsigned int width, unsigned int height);
    ~App();

    void Run();

private:
    void initGL();
    void initImGui();
    void initBuffers();

    void updateGridBuffer();
    void updateLSystemBuffer();
    void updateIFSBuffer();

    void ImGuiNewFrame();
    void renderScene();
    void renderUI();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow* window);
    void onScroll(double yOffset);
    void processPanning();
    bool processUIWantCaptureMouse();

    void terminateGL();
    void terminateImGui();

    // Auto-fit fractal to view
    void autoFitLSystem();
    void autoFitIFS();

private:
    // Program status
    GLFWwindow*     window                  = nullptr;
    unsigned int    width                   = 1280;
    unsigned int    height                  = 720;

    bool            isPanning               = false;
    glm::vec2       panStartMouse;
    glm::vec2       panStartOrigin;

    // Grid graphic parameters
    glm::vec2       origin                  = { 150.0f, 150.0f };
    float           pixelsPerUnit           = 50.0f;
    std::size_t     gridNum                 = 20;

    // Fractal mode selection
    enum class FractalMode { LSystem, IFS };
    FractalMode     currentMode             = FractalMode::LSystem;

    // L-System parameters
    LSystem         lsystem;
    int             lsystemIterations       = 3;
    glm::vec3       lsystemColor            = { 0.2f, 0.4f, 0.9f };
    float           lsystemLineWidth        = 2.0f;
    char            lsystemFilePath[256]    = "resources/fractals/sierpinski_bis.lsys";
    glm::vec2       lsystemStartPos         = { 0.0f, 0.0f };
    float           lsystemStartAngle       = 0.0f;

    // IFS parameters
    IFS             ifs;
    int             ifsNumPoints            = 50000;
    float           ifsPointSize            = 1.0f;
    char            ifsFilePath[256]        = "resources/fractals/zigzag.ifs";

    // Colors
    glm::vec3       bgColor                 = { 1.0f, 1.0f, 1.0f };
    glm::vec3       gridColor               = { 0.85f, 0.85f, 0.85f };
    glm::vec3       axisXColor              = { 1.0f, 0.0f, 0.0f };
    glm::vec3       axisYColor              = { 0.0f, 1.0f, 0.0f };

    // Transformation Matrices
    glm::mat4       projection;
    glm::mat4       moveOrigin;
    glm::mat4       scaleToPixels;

    // GPU resources
    ShaderProgram*  shader                  = nullptr;
    RenderData      gridRenderData;
    RenderData      axesRenderData;
    RenderData      lsystemRenderData;
    RenderData      ifsRenderData;
};

