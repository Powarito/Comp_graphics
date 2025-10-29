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

class App {
public:
    App(unsigned int width, unsigned int height);
    ~App();

    void Run();

private:
    void initGL();
    void initImGui();
    void initBuffers();

    void updateTrochoidBuffer();
    void updateGridBuffer();
    void updatePointsBuffer();

    void ImGuiNewFrame();
    void renderScene();
    void renderUI();
    
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow* window);

    void terminateGL();
    void terminateImGui();

    GLFWwindow*     window;
    unsigned int    width, height;

    glm::vec2       origin;
    float           pixelsPerUnit;
    std::size_t     gridNum;

    glm::vec2       figurePositionBase;
    glm::vec2       figurePosition;
    glm::vec2       rotatePoint;
    float           angleDegrees;
    glm::vec2       scale;
    bool            isMirrored;

    glm::vec2       O;
    glm::vec2       X;
    glm::vec2       Y;

    struct TrochoidParameters {
        float       r;
        float       h;
        float       tMax;
        float       dt;
    };
    TrochoidParameters  trochoidParams;

    ShaderProgram*  shader;
    RenderData      gridRenderData, axesRenderData, trochoidRenderData, 
                    pointsFigureRenderData, pointsBasisRenderData;

    std::vector<float>  pointsFigureVertices, pointsBasisVertices;

    glm::vec3       bgColor;
    glm::vec3       gridColor;
    glm::vec3       axisXColor;
    glm::vec3       axisYColor;
    glm::vec3       trochoidColor;
    glm::vec3       rotatePointColor;
};

