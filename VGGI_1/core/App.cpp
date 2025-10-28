#include "App.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

App::App(unsigned int width, unsigned int height) {
    this->width         = width;
    this->height        = height;
    window              = nullptr;
    origin              = { 150.0f, 150.0f };
    pixelsPerUnit       = 50.0f;
    gridNum             = 20;
    shader              = nullptr;

    figurePositionBase  = { 5.0f, 5.0f};
    figurePosition      = figurePositionBase;
    rotatePoint         = { 1.0f, 1.0f };
    angleDegrees        = 0.0f;
    scale               = { 1.0f, 1.0f };
    isMirrored          = false;

    O                   = { 0.0f, 0.0f };
    X                   = { 1.0f, 0.0f };
    Y                   = { 0.0f, 1.0f };

    trochoidParams.r    = 1.0f;
    trochoidParams.h    = 1.0f;
    trochoidParams.tMax = 25.0f;
    trochoidParams.dt   = 0.01f;

    bgColor             = { 1.0f, 1.0f, 1.0f };
    gridColor           = { 0.75f, 0.75f, 0.75f };
    axisXColor          = { 1.0f, 0.0f, 0.0f };
    axisYColor          = { 0.0f, 1.0f, 0.0f };
    trochoidColor       = { 0.9f, 0.2f, 1.0f };

    initGL();
    initImGui();
    initBuffers();
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

    window = glfwCreateWindow(width, height, "Trochoid Model - Transformations", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        exit(-1);
    }

    glViewport(0, 0, width, height);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

    std::vector<float> gridVertices     = generateGridVertices(gridNum, gridColor);
    std::vector<float> axesVertices     = generateAxesVertices(gridNum, axisXColor, axisYColor);
    std::vector<float> trochoidVertices = generateTrochoidVertices(
        trochoidParams.r, trochoidParams.h, 
        trochoidParams.tMax, trochoidParams.dt, 
        trochoidColor
    );

    gridRenderData     = Renderer::createRenderData(gridVertices, 1.0f);
    axesRenderData     = Renderer::createRenderData(axesVertices, 3.0f);
    trochoidRenderData = Renderer::createRenderData(trochoidVertices, 3.0f);
}

void App::updateTrochoidBuffer() {
    std::vector<float> trochoidVertices = generateTrochoidVertices(
        trochoidParams.r, trochoidParams.h,
        trochoidParams.tMax, trochoidParams.dt,
        trochoidColor
    );

    trochoidRenderData.vertexCount = trochoidVertices.size() / 5;
    // Do we need to bind VAO here?
    glBindBuffer(GL_ARRAY_BUFFER, trochoidRenderData.VBO);
    glBufferData(GL_ARRAY_BUFFER, trochoidVertices.size() * sizeof(float), trochoidVertices.data(), GL_STATIC_DRAW);
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

void App::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void App::renderScene() {
    glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    glm::mat4 moveOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(origin, 0.0f));
    glm::mat4 scaleToPixels = glm::scale(glm::mat4(1.0f), glm::vec3(pixelsPerUnit, pixelsPerUnit, 1.0f));
    glm::mat4 affineBasis = buildAffineBasis2D(O, X, Y);

    shader->setMat4("projection", projection);
    shader->setMat4("moveOrigin", moveOrigin);
    shader->setMat4("scaleToPixels", scaleToPixels);
    shader->setMat4("affineBasis", affineBasis);

    // Grid and axes
    shader->setMat4("rawModel", glm::mat4(1.0f));
    Renderer::Draw(*shader, gridRenderData, GL_LINES);
    Renderer::Draw(*shader, axesRenderData, GL_LINES);

    // Trochoid
    glm::mat4 trochoidModel = computeTransformMatrix(figurePositionBase, rotatePoint, angleDegrees, scale, isMirrored, isMirrored);
    shader->setMat4("rawModel", trochoidModel);
    Renderer::Draw(*shader, trochoidRenderData, GL_LINE_STRIP);

    figurePosition = computeRotatedPosition(figurePositionBase, rotatePoint, angleDegrees);
}

void App::renderUI() {
    ImGui::Begin("Trochoid Controls");

    // Trochoid params
    if (ImGui::CollapsingHeader("Trochoid Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool changed = false;
        changed |= ImGui::DragFloat("r", &trochoidParams.r, 0.01f, 0.1f, 10.0f);
        changed |= ImGui::DragFloat("h", &trochoidParams.h, 0.01f, 0.0f, 10.0f);
        changed |= ImGui::DragFloat("tMax", &trochoidParams.tMax, 0.1f, 1.0f, 100.0f);
        changed |= ImGui::DragFloat("dt", &trochoidParams.dt, 0.001f, 0.001f, 0.1f);
        if (changed) {
            updateTrochoidBuffer();
        }
    }

    // Euclidean transforms
    if (ImGui::CollapsingHeader("Euclidean Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::DragFloat2("Figure position", glm::value_ptr(figurePosition), 0.1f)) {
            figurePositionBase = computeRotatedPosition(figurePosition, rotatePoint, -angleDegrees);
        }
        ImGui::DragFloat2("Rotate point", glm::value_ptr(rotatePoint), 0.1f);
        ImGui::DragFloat("Angle (deg)", &angleDegrees, 0.5f, -360.0f, 360.0f);
    }

    // Affine transformations
    if (ImGui::CollapsingHeader("Affine Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("O", glm::value_ptr(O), 0.1f);
        ImGui::DragFloat2("X", glm::value_ptr(X), 0.1f);
        ImGui::DragFloat2("Y", glm::value_ptr(Y), 0.1f);
        ImGui::DragFloat2("Scale", glm::value_ptr(scale), 0.01f, 0.1f, 10.0f);
        ImGui::Checkbox("Is Mirrored", &isMirrored);
    }

    // Grid
    if (ImGui::CollapsingHeader("Grid Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Pixels per Unit", &pixelsPerUnit, 1.0f, 10.0f, 200.0f);
        if (ImGui::DragInt("Grid size", (int*)&gridNum, 1, 2, 50)) {
            updateGridBuffer();
        }
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
}

void App::terminateGL()
{
    glDeleteVertexArrays(1, &gridRenderData.VAO);
    glDeleteBuffers(1, &gridRenderData.VBO);
    glDeleteVertexArrays(1, &axesRenderData.VAO);
    glDeleteBuffers(1, &axesRenderData.VBO);
    glDeleteVertexArrays(1, &trochoidRenderData.VAO);
    glDeleteBuffers(1, &trochoidRenderData.VBO);

    delete shader;

    glfwTerminate();
}

void App::terminateImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
