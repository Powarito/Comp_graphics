#include "App.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

App::App(unsigned int width, unsigned int height) {
    this->width = width;
    this->height = height;

    initGL();
    initImGui();
    initBuffers();

    // Load default fractals
    lsystem.loadFromFile(lsystemFilePath);
    ifs.loadFromFile(ifsFilePath);

    updateLSystemBuffer();
    updateIFSBuffer();
}

App::~App() {
    terminateImGui();
    terminateGL();
}

void App::Run() {
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        ImGuiNewFrame();
        renderScene();
        renderUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::initGL() {
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

    window = glfwCreateWindow(width, height, "Fractal Visualization - L-System & IFS", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, [](GLFWwindow* w, double xOffset, double yOffset) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onScroll(yOffset);
        });
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        exit(-1);
    }

    glViewport(0, 0, width, height);
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    float fontScale = 1.75f;
    ImGui::GetStyle().ScaleAllSizes(fontScale);
    io.FontGlobalScale = fontScale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void App::initBuffers() {
    shader = new ShaderProgram("resources/shaders/shader.vert", "resources/shaders/shader.frag");

    std::vector<float> gridVertices = generateGridVertices(gridNum, gridColor);
    std::vector<float> axesVertices = generateAxesVertices(gridNum, axisXColor, axisYColor);

    gridRenderData = Renderer::createRenderData(gridVertices, GL_LINES, 1.0f, GL_STATIC_DRAW);
    axesRenderData = Renderer::createRenderData(axesVertices, GL_LINES, 3.0f, GL_STATIC_DRAW);

    // Initialize empty render data for fractals
    // L-System uses GL_LINES (pairs of vertices for each segment)
    // IFS uses GL_POINTS (individual points)
    std::vector<float> empty;
    lsystemRenderData = Renderer::createRenderData(empty, GL_LINES, 2.0f, GL_DYNAMIC_DRAW);
    ifsRenderData = Renderer::createRenderData(empty, GL_POINTS, 1.0f, GL_DYNAMIC_DRAW);
}

void App::updateGridBuffer() {
    std::vector<float> gridVertices = generateGridVertices(gridNum, gridColor);
    std::vector<float> axesVertices = generateAxesVertices(gridNum, axisXColor, axisYColor);

    gridRenderData.vertexCount = gridVertices.size() / 5;
    axesRenderData.vertexCount = axesVertices.size() / 5;

    glBindBuffer(GL_ARRAY_BUFFER, gridRenderData.VBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, axesRenderData.VBO);
    glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(float), axesVertices.data(), GL_STATIC_DRAW);
}

void App::updateLSystemBuffer() {
    std::vector<float> vertices = lsystem.generateVertices(
        lsystemIterations,
        lsystemColor,
        lsystemStartPos,
        glm::radians(lsystemStartAngle)
    );

    lsystemRenderData.vertexCount = vertices.size() / 5;
    lsystemRenderData.primitiveSize = lsystemLineWidth;

    glBindBuffer(GL_ARRAY_BUFFER, lsystemRenderData.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
}

void App::updateIFSBuffer() {
    ifs.resetSeed(42);
    std::vector<float> vertices = ifs.generateVertices(ifsNumPoints);

    ifsRenderData.vertexCount = vertices.size() / 5;
    ifsRenderData.primitiveSize = ifsPointSize;

    glBindBuffer(GL_ARRAY_BUFFER, ifsRenderData.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
}

void App::autoFitLSystem() {
    glm::vec2 minB, maxB;
    lsystem.getBoundingBox(lsystemIterations, minB, maxB);

    glm::vec2 size = maxB - minB;
    glm::vec2 center = (minB + maxB) * 0.5f;

    // Constants for UI layout
    const float uiPanelWidth = 400.0f;      // Space reserved for ImGui panel on the right
    const float verticalPadding = 100.0f;   // Top/bottom margin
    const float epsilon = 0.1f;             // Prevent division by zero for degenerate cases
    const float viewPadding = 0.9f;         // Leave 10% padding around fractal

    // Calculate scale to fit fractal in available viewport
    float availableWidth = width - uiPanelWidth;
    float availableHeight = height - verticalPadding;

    float scaleX = availableWidth / (size.x + epsilon);
    float scaleY = availableHeight / (size.y + epsilon);

    // Use smaller scale to maintain aspect ratio, with padding
    pixelsPerUnit = std::min(scaleX, scaleY) * viewPadding;

    // Center fractal in viewport (accounting for UI panel)
    float viewportCenterX = (width - uiPanelWidth) / 2.0f;
    float viewportCenterY = height / 2.0f;
    origin = glm::vec2(viewportCenterX, viewportCenterY) - center * pixelsPerUnit;
}

void App::autoFitIFS() {
    glm::vec2 minB, maxB;
    ifs.resetSeed(42);
    ifs.getBoundingBox(ifsNumPoints / 10, minB, maxB);

    glm::vec2 size = maxB - minB;
    glm::vec2 center = (minB + maxB) * 0.5f;

    // Constants for UI layout
    const float uiPanelWidth = 400.0f;
    const float verticalPadding = 100.0f;
    const float epsilon = 0.1f;
    const float viewPadding = 0.9f;

    // Calculate scale to fit fractal in available viewport
    float availableWidth = width - uiPanelWidth;
    float availableHeight = height - verticalPadding;

    float scaleX = availableWidth / (size.x + epsilon);
    float scaleY = availableHeight / (size.y + epsilon);

    // Use smaller scale to maintain aspect ratio, with padding
    pixelsPerUnit = std::min(scaleX, scaleY) * viewPadding;

    // Center fractal in viewport (accounting for UI panel)
    float viewportCenterX = (width - uiPanelWidth) / 2.0f;
    float viewportCenterY = height / 2.0f;
    origin = glm::vec2(viewportCenterX, viewportCenterY) - center * pixelsPerUnit;
}

void App::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void App::renderScene() {
    glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    moveOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(origin, 0.0f));
    scaleToPixels = glm::scale(glm::mat4(1.0f), glm::vec3(pixelsPerUnit, pixelsPerUnit, 1.0f));

    shader->setMat4("projection", projection);
    shader->setMat4("moveOrigin", moveOrigin);
    shader->setMat4("scaleToPixels", scaleToPixels);
    shader->setMat4("affineBasis", glm::mat4(1.0f));
    shader->setMat4("rawModel", glm::mat4(1.0f));

    // Draw grid and axes
    Renderer::Draw(*shader, gridRenderData);
    Renderer::Draw(*shader, axesRenderData);

    // Draw current fractal
    if (currentMode == FractalMode::LSystem) {
        Renderer::Draw(*shader, lsystemRenderData);
    }
    else {
        Renderer::Draw(*shader, ifsRenderData);
    }
}

void App::renderUI() {
    ImGui::Begin("Fractal Controls");

    // Mode selection
    const char* modes[] = { "L-System", "IFS" };
    int modeIdx = static_cast<int>(currentMode);
    if (ImGui::Combo("Fractal Type", &modeIdx, modes, 2)) {
        currentMode = static_cast<FractalMode>(modeIdx);
    }

    ImGui::Separator();

    if (currentMode == FractalMode::LSystem) {
        // L-System controls
        if (ImGui::CollapsingHeader("L-System Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputText("File Path##lsys", lsystemFilePath, sizeof(lsystemFilePath));

            if (ImGui::Button("Load L-System File")) {
                if (lsystem.loadFromFile(lsystemFilePath)) {
                    updateLSystemBuffer();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Auto Fit##lsys")) {
                autoFitLSystem();
            }

            ImGui::Separator();

            // Display current configuration
            const auto& cfg = lsystem.getConfig();
            ImGui::Text("Axiom: %s", cfg.axiom.c_str());
            ImGui::Text("Angle: %.1f deg", cfg.angle);
            ImGui::Text("Rules:");
            for (const auto& [var, rule] : cfg.rules) {
                ImGui::Text("  %c -> %s", var, rule.c_str());
            }

            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::DragInt("Iterations", &lsystemIterations, 1, 0, 8);
            changed |= ImGui::DragFloat2("Start Position", glm::value_ptr(lsystemStartPos), 0.1f);
            changed |= ImGui::DragFloat("Start Angle", &lsystemStartAngle, 1.0f, -180.0f, 180.0f);
            changed |= ImGui::ColorEdit3("Line Color", glm::value_ptr(lsystemColor));
            changed |= ImGui::DragFloat("Line Width", &lsystemLineWidth, 0.1f, 0.5f, 10.0f);

            if (changed) {
                updateLSystemBuffer();
            }
        }
    }
    else {
        // IFS controls
        if (ImGui::CollapsingHeader("IFS Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputText("File Path##ifs", ifsFilePath, sizeof(ifsFilePath));

            if (ImGui::Button("Load IFS File")) {
                if (ifs.loadFromFile(ifsFilePath)) {
                    updateIFSBuffer();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Auto Fit##ifs")) {
                autoFitIFS();
            }

            ImGui::Separator();

            // Display transforms
            const auto& transforms = ifs.getTransforms();
            ImGui::Text("Transforms: %zu", transforms.size());
            for (size_t i = 0; i < transforms.size(); ++i) {
                const auto& t = transforms[i];
                ImGui::TextColored(
                    ImVec4(t.color.r, t.color.g, t.color.b, 1.0f),
                    "T%zu: p=%.3f", i + 1, t.p
                );
            }

            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::DragInt("Num Points", &ifsNumPoints, 1000, 1000, 200000);
            changed |= ImGui::DragFloat("Point Size", &ifsPointSize, 0.1f, 0.5f, 5.0f);

            if (changed) {
                updateIFSBuffer();
            }
        }
    }

    // Grid parameters
    if (ImGui::CollapsingHeader("Grid Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Pixels per Unit", &pixelsPerUnit, 1.0f, 1.0f, 500.0f);
        if (ImGui::DragInt("Grid Size", (int*)&gridNum, 1, 2, 100)) {
            updateGridBuffer();
        }
        ImGui::DragFloat2("Grid Offset (px)", glm::value_ptr(origin), 1.0f);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->width = width;
        app->height = height;
        glViewport(0, 0, width, height);
    }
}

void App::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (processUIWantCaptureMouse() == false) {
        processPanning();
    }
}

void App::onScroll(double yOffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    const float zoomFactor = 1.1f;

    if (yOffset > 0) {
        pixelsPerUnit *= zoomFactor;
    }
    else if (yOffset < 0) {
        pixelsPerUnit /= zoomFactor;
    }

    pixelsPerUnit = std::max(pixelsPerUnit, 0.0001f);
}

void App::processPanning() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseY = height - mouseY;

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (mousePressed && !isPanning) {
        isPanning = true;
        panStartMouse = glm::vec2(mouseX, mouseY);
        panStartOrigin = origin;
    }
    else if (!mousePressed && isPanning) {
        isPanning = false;
    }

    if (isPanning) {
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - panStartMouse;
        origin = panStartOrigin + mouseDelta;
    }
}

bool App::processUIWantCaptureMouse() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        isPanning = false;
        return true;
    }
    return false;
}

void App::terminateGL() {
    glDeleteVertexArrays(1, &gridRenderData.VAO);
    glDeleteBuffers(1, &gridRenderData.VBO);
    glDeleteVertexArrays(1, &axesRenderData.VAO);
    glDeleteBuffers(1, &axesRenderData.VBO);
    glDeleteVertexArrays(1, &lsystemRenderData.VAO);
    glDeleteBuffers(1, &lsystemRenderData.VBO);
    glDeleteVertexArrays(1, &ifsRenderData.VAO);
    glDeleteBuffers(1, &ifsRenderData.VBO);

    delete shader;
    glfwTerminate();
}

void App::terminateImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

