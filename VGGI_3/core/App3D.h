#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "auxiliary/ShaderProgram.h"
#include "auxiliary/Camera.h"
#include "Renderer3D.h"
#include "Surface.h"
#include "SurfaceTexture.h"
#include "Projection.h"

class App3D {
public:
    App3D(unsigned int width, unsigned int height);
    ~App3D();

    void Run();

private:
    void initGL();
    void initImGui();
    void initShaders();
    void initBuffers();
    void initSurface();

    void updateSurfaceMesh();
    void updateWireframeMesh();
    void updateTextureMeshes();
    void updateAxesMesh();

    void cleanupTextureMeshes();

    void ImGuiNewFrame();
    void renderScene();
    void renderUI();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    
    void processInput(GLFWwindow* window);

    void terminateGL();
    void terminateImGui();

private:
    // Window
    GLFWwindow*             window                  = nullptr;
    unsigned int            width                   = 1920;
    unsigned int            height                  = 1200;

    // Camera
    Camera                  camera;
    bool                    firstMouse              = true;
    float                   lastX                   = 0.0f;
    float                   lastY                   = 0.0f;
    float                   deltaTime               = 0.0f;
    float                   lastFrame               = 0.0f;
    
    // Mouse button state tracking
    int                     prevMouseButtonState    = GLFW_RELEASE;

    // Projection
    ProjectionManager       projManager;
    
    // Surface
    Surface*                surface                 = nullptr;
    float                   surfaceRadius           = 3.0f;
    int                     surfaceSegments         = 50;
    glm::vec3               surfaceColor            = glm::vec3(0.7f, 0.7f, 0.9f);
    glm::vec3               wireframeColor          = glm::vec3(0.3f, 0.3f, 0.5f);
    
    // Surface transformation
    glm::vec3               surfaceTranslation      = glm::vec3(0.0f);
    glm::vec3               surfaceRotation         = glm::vec3(0.0f);  // Degrees
    
    // Texture
    SurfaceTexture          texture;
    std::string             textureFile             = "palm_tree.dat";
    
    // Display options
    bool                    showSurface             = true;
    bool                    showWireframe           = true;
    bool                    showTexture             = true;
    bool                    showAxes                = true;
    float                   axesLength              = 10.0f;
    
    // Colors
    glm::vec3               bgColor                 = glm::vec3(0.1f, 0.1f, 0.15f);

    // Shaders
    ShaderProgram*          surfaceShader           = nullptr;
    ShaderProgram*          simpleShader            = nullptr;

    // GPU resources
    MeshData                surfaceMesh;
    MeshData                wireframeMesh;
    MeshData                axesMesh;
    
    // Texture meshes - one per curve to avoid connecting unrelated curves
    std::vector<MeshData>   textureCurveMeshes;
    
    bool                    surfaceMeshValid        = false;
    bool                    wireframeMeshValid      = false;
    bool                    axesMeshValid           = false;
};

