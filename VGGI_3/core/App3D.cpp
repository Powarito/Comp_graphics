#include "App3D.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

App3D::App3D(unsigned int width, unsigned int height)
    : width(width), height(height),
      camera(glm::vec3(0.0f, 5.0f, 20.0f))
{
    lastX = width / 2.0f;
    lastY = height / 2.0f;
    
    // Camera starts inactive
    camera.setIsActive(false);
    
    initGL();
    initImGui();
    initShaders();
    initSurface();
    initBuffers();
    
    // Load texture
    texture.loadFromFile(textureFile);
    updateTextureMeshes();
}

App3D::~App3D() {
    if (surfaceMeshValid) Renderer3D::deleteMesh(surfaceMesh);
    if (wireframeMeshValid) Renderer3D::deleteMesh(wireframeMesh);
    if (axesMeshValid) Renderer3D::deleteMesh(axesMesh);
    cleanupTextureMeshes();
    
    delete surface;
    delete surfaceShader;
    delete simpleShader;
    
    terminateImGui();
    terminateGL();
}

void App3D::Run() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        ImGuiNewFrame();
        renderScene();
        renderUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App3D::initGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, "Surface Projection - Lab 3", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        exit(-1);
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App3D::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    float fontScale = 1.3f;
    ImGui::GetStyle().ScaleAllSizes(fontScale);
    io.FontGlobalScale = fontScale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void App3D::initShaders() {
    surfaceShader = new ShaderProgram("resources/shaders/shader3d.vert", "resources/shaders/shader3d.frag");
    simpleShader = new ShaderProgram("resources/shaders/simple.vert", "resources/shaders/simple.frag");
}

void App3D::initBuffers() {
    updateSurfaceMesh();
    updateWireframeMesh();
    updateAxesMesh();
}

void App3D::initSurface() {
    if (surface) delete surface;
    surface = new Surface(createPearSurface(surfaceRadius, surfaceSegments));
}

void App3D::updateSurfaceMesh() {
    if (surfaceMeshValid) Renderer3D::deleteMesh(surfaceMesh);
    
    // Create new surface
    initSurface();
    
    // Generate surface mesh
    auto vertices = surface->generateMeshVertices();
    auto indices = surface->generateMeshIndices();
    surfaceMesh = Renderer3D::createSurfaceMesh(vertices, indices, GL_TRIANGLES);
    surfaceMeshValid = true;
}

void App3D::updateWireframeMesh() {
    if (wireframeMeshValid) Renderer3D::deleteMesh(wireframeMesh);
    
    if (!surface) return;
    
    // Generate wireframe
    auto wireVerts = surface->generateWireframeVertices();
    std::vector<float> wireVertsWithColor;
    wireVertsWithColor.reserve(wireVerts.size() / 3 * 6);
    
    for (size_t i = 0; i < wireVerts.size(); i += 3) {
        wireVertsWithColor.push_back(wireVerts[i]);
        wireVertsWithColor.push_back(wireVerts[i + 1]);
        wireVertsWithColor.push_back(wireVerts[i + 2]);
        wireVertsWithColor.push_back(wireframeColor.r);
        wireVertsWithColor.push_back(wireframeColor.g);
        wireVertsWithColor.push_back(wireframeColor.b);
    }
    
    wireframeMesh = Renderer3D::createSimpleMesh(wireVertsWithColor, GL_LINES, 1.0f);
    wireframeMeshValid = true;
}

void App3D::updateTextureMeshes() {
    cleanupTextureMeshes();
    
    if (texture.curves.empty() || !surface) return;
    
    // Generate separate mesh for each curve
    auto allCurves = texture.generateTextureCurves(*surface);
    
    for (const auto& curveVerts : allCurves) {
        if (!curveVerts.empty()) {
            MeshData mesh = Renderer3D::createSimpleMesh(curveVerts, GL_LINE_STRIP, 3.0f);
            textureCurveMeshes.push_back(mesh);
        }
    }
}

void App3D::updateAxesMesh() {
    if (axesMeshValid) Renderer3D::deleteMesh(axesMesh);
    
    auto axesVerts = generateAxes3D(axesLength);
    axesMesh = Renderer3D::createSimpleMesh(axesVerts, GL_LINES, 2.0f);
    axesMeshValid = true;
}

void App3D::cleanupTextureMeshes() {
    for (auto& mesh : textureCurveMeshes) {
        Renderer3D::deleteMesh(mesh);
    }
    textureCurveMeshes.clear();
}

void App3D::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void App3D::renderScene() {
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    
    // Get projection matrix
    glm::mat4 projection = projManager.getProjectionMatrix(aspectRatio);
    
    // View matrix depends on projection type
    glm::mat4 view;
    if (projManager.currentProjection == ProjectionType::DIMETRIC) {
        view = projManager.getDimetricViewMatrix();
    } else {
        view = camera.GetViewMatrix();
    }
    
    // Create model matrix for surface
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, surfaceTranslation);
    model = glm::rotate(model, glm::radians(surfaceRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(surfaceRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(surfaceRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Render axes (at world origin, no model transform)
    if (showAxes && axesMeshValid) {
        simpleShader->use();
        simpleShader->setMat4("projection", projection);
        simpleShader->setMat4("view", view);
        simpleShader->setMat4("model", glm::mat4(1.0f));
        Renderer3D::DrawIndexed(*simpleShader, axesMesh);
    }

    // Render surface
    if (showSurface && surfaceMeshValid) {
        surfaceShader->use();
        surfaceShader->setMat4("projection", projection);
        surfaceShader->setMat4("view", view);
        surfaceShader->setMat4("model", model);
        surfaceShader->setVec3("objectColor", surfaceColor);
        
        Renderer3D::DrawIndexed(*surfaceShader, surfaceMesh);
    }

    // Render wireframe
    if (showWireframe && wireframeMeshValid) {
        simpleShader->use();
        simpleShader->setMat4("projection", projection);
        simpleShader->setMat4("view", view);
        simpleShader->setMat4("model", model);
        Renderer3D::DrawIndexed(*simpleShader, wireframeMesh);
    }

    // Render texture curves (each curve separately)
    if (showTexture) {
        simpleShader->use();
        simpleShader->setMat4("projection", projection);
        simpleShader->setMat4("view", view);
        simpleShader->setMat4("model", model);
        
        for (const auto& curveMesh : textureCurveMeshes) {
            Renderer3D::DrawIndexed(*simpleShader, curveMesh);
        }
    }
}

void App3D::renderUI() {
    // Skip UI rendering when camera is active
    if (camera.getIsActive()) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return;
    }
    
    ImGui::Begin("Surface Controls");

    if (ImGui::CollapsingHeader("Projection", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* projTypes[] = { "Dimetric", "Perspective", "Orthographic" };
        int currentProj = static_cast<int>(projManager.currentProjection);
        
        if (ImGui::Combo("Projection Type", &currentProj, projTypes, 3)) {
            projManager.currentProjection = static_cast<ProjectionType>(currentProj);
        }
        
        if (projManager.currentProjection == ProjectionType::DIMETRIC) {
            ImGui::DragFloat("Angle Y (deg)", &projManager.dimetricAngleY, 0.5f, -180.0f, 180.0f);
            ImGui::DragFloat("Angle X (deg)", &projManager.dimetricAngleX, 0.5f, -180.0f, 180.0f);
            ImGui::DragFloat("Ortho Size", &projManager.orthoSize, 0.1f, 0.1f, 50.0f);
            
            if (ImGui::Button("Reset Dimetric Angles")) {
                projManager.resetDimetricAngles();
            }
        }
        else if (projManager.currentProjection == ProjectionType::PERSPECTIVE) {
            ImGui::DragFloat("FOV", &projManager.perspectiveFov, 0.5f, 10.0f, 120.0f);
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera: RMB to toggle");
            ImGui::Text("WASD - move, Space/Ctrl - up/down");
        }
        else if (projManager.currentProjection == ProjectionType::ORTHOGRAPHIC) {
            ImGui::DragFloat("Ortho Size", &projManager.orthoSize, 0.1f, 0.1f, 50.0f);
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera: RMB to toggle");
            ImGui::Text("WASD - move, Space/Ctrl - up/down");
        }
    }

    if (ImGui::CollapsingHeader("Surface Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool meshChanged = false;
        meshChanged |= ImGui::DragFloat("Radius", &surfaceRadius, 0.1f, 0.5f, 20.0f);
        meshChanged |= ImGui::DragInt("Segments", &surfaceSegments, 1, 10, 100);
        
        if (meshChanged) {
            updateSurfaceMesh();
            updateWireframeMesh();
            updateTextureMeshes();
        }
        
        ImGui::ColorEdit3("Surface Color", glm::value_ptr(surfaceColor));
        
        if (ImGui::ColorEdit3("Wireframe Color", glm::value_ptr(wireframeColor))) {
            updateWireframeMesh();
        }
    }

    if (ImGui::CollapsingHeader("Surface Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Translation", glm::value_ptr(surfaceTranslation), 0.1f);
        ImGui::DragFloat3("Rotation (deg)", glm::value_ptr(surfaceRotation), 1.0f, -180.0f, 180.0f);
        
        if (ImGui::Button("Reset Surface Transform")) {
            surfaceTranslation = glm::vec3(0.0f);
            surfaceRotation = glm::vec3(0.0f);
        }
    }

    if (ImGui::CollapsingHeader("Texture Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool texChanged = false;
        texChanged |= ImGui::DragFloat2("UV Offset", glm::value_ptr(texture.offset), 0.01f);
        texChanged |= ImGui::DragFloat("UV Scale", &texture.scale, 0.01f, 0.1f, 5.0f);
        texChanged |= ImGui::DragFloat("UV Rotation (deg)", &texture.rotation, 1.0f, -180.0f, 180.0f);
        
        if (texChanged) {
            updateTextureMeshes();
        }
        
        if (ImGui::Button("Reset Texture Transform")) {
            texture.offset = glm::vec2(0.0f);
            texture.scale = 1.0f;
            texture.rotation = 0.0f;
            updateTextureMeshes();
        }
        
        glm::vec2 center = texture.getCenter();
        ImGui::Text("Texture center: (%.2f, %.2f)", center.x, center.y);
    }

    if (ImGui::CollapsingHeader("Display Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Surface", &showSurface);
        ImGui::Checkbox("Show Wireframe", &showWireframe);
        ImGui::Checkbox("Show Texture", &showTexture);
        ImGui::Checkbox("Show Axes", &showAxes);
        
        if (ImGui::DragFloat("Axes Length", &axesLength, 0.5f, 1.0f, 50.0f)) {
            updateAxesMesh();
        }
        
        ImGui::ColorEdit3("Background", glm::value_ptr(bgColor));
    }

    if (ImGui::CollapsingHeader("Camera Info")) {
        glm::vec3 pos = camera.getPosition();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", camera.getYaw(), camera.getPitch());
        ImGui::Text("Active: %s", camera.getIsActive() ? "Yes" : "No");
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App3D::framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    App3D* app = static_cast<App3D*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->width = w;
        app->height = h;
        glViewport(0, 0, w, h);
    }
}

void App3D::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    App3D* app = static_cast<App3D*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    float xposF = static_cast<float>(xpos);
    float yposF = static_cast<float>(ypos);

    if (app->firstMouse) {
        app->lastX = xposF;
        app->lastY = yposF;
        app->firstMouse = false;
    }

    float xoffset = xposF - app->lastX;
    float yoffset = app->lastY - yposF;

    app->lastX = xposF;
    app->lastY = yposF;

    // ProcessMouseMovement internally checks if camera is active
    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void App3D::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App3D* app = static_cast<App3D*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    // Process scroll only when camera IS active (UI is hidden)
    if (app->camera.getIsActive()) {
        if (app->projManager.currentProjection == ProjectionType::PERSPECTIVE) {
            app->projManager.perspectiveFov -= static_cast<float>(yoffset) * 2.0f;
            app->projManager.perspectiveFov = glm::clamp(app->projManager.perspectiveFov, 10.0f, 120.0f);
        } else {
            app->projManager.orthoSize -= static_cast<float>(yoffset) * 0.5f;
            app->projManager.orthoSize = glm::clamp(app->projManager.orthoSize, 1.0f, 50.0f);
        }
    }
}

void App3D::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (camera.getIsActive()) {
            // Deactivate camera on ESC
            camera.setIsActive(false);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetWindowShouldClose(window, true);
        }
    }

    // Toggle camera with RMB (only for non-dimetric projections)
    int currentMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (currentMouseButtonState == GLFW_PRESS && prevMouseButtonState == GLFW_RELEASE) {
        // Check if mouse is over ImGui
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && projManager.currentProjection != ProjectionType::DIMETRIC) {
            bool newState = !camera.getIsActive();
            camera.setIsActive(newState);
            
            if (newState) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; // Reset to avoid jump
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }
    prevMouseButtonState = currentMouseButtonState;

    // Camera movement when active (only for non-dimetric)
    if (camera.getIsActive() && projManager.currentProjection != ProjectionType::DIMETRIC) {
        float velocity = deltaTime;
        
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            velocity *= 3.0f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, velocity);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, velocity);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::LEFT, velocity);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, velocity);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::UP, velocity);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.ProcessKeyboard(Camera::Camera_Movement::DOWN, velocity);
    }
}

void App3D::terminateGL() {
    glfwTerminate();
}

void App3D::terminateImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

