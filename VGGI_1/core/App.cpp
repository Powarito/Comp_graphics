#include "App.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <limits>

App::App(unsigned int width, unsigned int height) {
    this->width     = width;
    this->height    = height;

    initGL();
    initImGui();
    initBuffers();
    initDraggablePoints();
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

    auto add = [&](std::vector<float>& verts, const glm::vec2& p, const glm::vec3& c) {
        verts.insert(verts.end(), { p.x, p.y, c.r, c.g, c.b });
        };

    std::vector<float> pointsFigureVertices;
    std::vector<float> pointsBasisVertices;
    pointsFigureVertices.reserve(2 * 5);
    pointsBasisVertices.reserve(3 * 5);

    add(pointsFigureVertices, figurePosition, trochoidColor);
    add(pointsFigureVertices, rotatePoint,    rotatePointColor);
    add(pointsBasisVertices,  O,              gridColor);
    add(pointsBasisVertices,  X,              axisXColor);
    add(pointsBasisVertices,  Y,              axisYColor);

    gridRenderData         = Renderer::createRenderData(gridVertices,           GL_LINES,       1.0f,   GL_STATIC_DRAW);
    axesRenderData         = Renderer::createRenderData(axesVertices,           GL_LINES,       3.0f,   GL_STATIC_DRAW);
    trochoidRenderData     = Renderer::createRenderData(trochoidVertices,       GL_LINE_STRIP,  3.0f,   GL_STATIC_DRAW);
    pointsFigureRenderData = Renderer::createRenderData(pointsFigureVertices,   GL_POINTS,      10.0f,  GL_DYNAMIC_DRAW);
    pointsBasisRenderData  = Renderer::createRenderData(pointsBasisVertices,    GL_POINTS,      10.0f,  GL_DYNAMIC_DRAW);
}

void App::initDraggablePoints() {
    draggablePoints.clear();

    draggablePoints.emplace_back(&O, gridColor, false);
    draggablePoints.emplace_back(&X, axisXColor, false);
    draggablePoints.emplace_back(&Y, axisYColor, false);

    draggablePoints.emplace_back(
        &figurePosition, trochoidColor, true,
        [this]() {
            figurePositionBase = computeRotatedPosition(figurePosition, rotatePoint, -angleDegrees);
        }
    );

    draggablePoints.emplace_back(&rotatePoint, rotatePointColor, true);
}

void App::updateTrochoidBuffer() {
    std::vector<float> trochoidVertices = generateTrochoidVertices(
        trochoidParams.r, trochoidParams.h,
        trochoidParams.tMax, trochoidParams.dt,
        trochoidColor
    );

    trochoidRenderData.vertexCount = trochoidVertices.size() / 5;

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

void App::updatePointsBuffer() {
    auto add = [&](std::vector<float>& verts, const glm::vec2& p, const glm::vec3& c) {
        verts.insert(verts.end(), { p.x, p.y, c.r, c.g, c.b });
        };

    std::vector<float> pointsFigureVertices;
    std::vector<float> pointsBasisVertices;
    pointsFigureVertices.reserve(2 * 5);
    pointsBasisVertices.reserve(3 * 5);

    add(pointsFigureVertices, figurePosition, trochoidColor);
    add(pointsFigureVertices, rotatePoint, rotatePointColor);
    add(pointsBasisVertices, O, gridColor);
    add(pointsBasisVertices, X, axisXColor);
    add(pointsBasisVertices, Y, axisYColor);

    glBindBuffer(GL_ARRAY_BUFFER, pointsFigureRenderData.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pointsFigureVertices.size() * sizeof(float), pointsFigureVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, pointsBasisRenderData.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pointsBasisVertices.size()  * sizeof(float), pointsBasisVertices.data());
}

void App::updateDraggablePoints() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (mousePressed && !isDragging) {
        const float pickRadius = 10.0f / pixelsPerUnit;

        glm::vec2 mouseWorldNoAffine = screenToWorld(mouseX, mouseY, false);
        glm::vec2 mouseWorldAffine = screenToWorld(mouseX, mouseY, true);

        float minDist = std::numeric_limits<float>::max();
        int pickedIdx = -1;
        glm::vec2 pickedOffset;

        for (int i = 0; i < draggablePoints.size(); ++i) {
            auto& dp = draggablePoints[i];
            glm::vec2 target = dp.useAffine ? mouseWorldAffine : mouseWorldNoAffine;
            float dist = glm::length(target - *dp.position);

            if (dist < pickRadius && dist < minDist) {
                pickedIdx = i;
                pickedOffset = target - *dp.position;
                minDist = dist;
            }
        }

        if (pickedIdx >= 0) {
            isDragging = true;
            draggedPointIdx = pickedIdx;
            dragOffset = pickedOffset;
        }
    }
    else if (!mousePressed) {
        isDragging = false;
        draggedPointIdx = -1;
    }

    if (isDragging && draggedPointIdx != -1) {
        auto& dp = draggablePoints[draggedPointIdx];
        glm::vec2 mouseWorld = screenToWorld(mouseX, mouseY, dp.useAffine);
        *dp.position = mouseWorld - dragOffset;

        if (dp.onUpdate) dp.onUpdate();
        updatePointsBuffer();
    }
}

void App::processPanning() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    mouseY = height - mouseY;

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (!isDragging) {
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
}

bool App::processUIWantCaptureMouse() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        isDragging = false;
        isPanning = false;
        return true;
    }
    return false;
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
    affineBasis = buildAffineBasis2D(O, X, Y);

    shader->setMat4("projection", projection);
    shader->setMat4("moveOrigin", moveOrigin);
    shader->setMat4("scaleToPixels", scaleToPixels);
    shader->setMat4("affineBasis", affineBasis);

    // Grid and axes
    shader->setMat4("rawModel", glm::mat4(1.0f));
    Renderer::Draw(*shader, gridRenderData);
    Renderer::Draw(*shader, axesRenderData);

    // Points
    // We need to draw O, X and Y before using affineBasis, but the Figure points - after using that matrix
    shader->setMat4("affineBasis", glm::mat4(1.0f));
    Renderer::Draw(*shader, pointsBasisRenderData);

    shader->setMat4("affineBasis", affineBasis);
    Renderer::Draw(*shader, pointsFigureRenderData);

    // Trochoid
    glm::mat4 trochoidModel = computeTransformMatrix(figurePositionBase, rotatePoint, angleDegrees, scale, isMirrored, isMirrored);
    shader->setMat4("rawModel", trochoidModel);
    Renderer::Draw(*shader, trochoidRenderData);

    figurePosition = computeRotatedPosition(figurePositionBase, rotatePoint, angleDegrees);
}

void App::renderUI() {
    ImGui::Begin("Trochoid Controls");

    // Trochoid params
    if (ImGui::CollapsingHeader("Trochoid Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool changed = false;
        changed |= ImGui::DragFloat("r",    &trochoidParams.r,      0.01f,  0.1f,   10.0f);
        changed |= ImGui::DragFloat("h",    &trochoidParams.h,      0.01f,  0.0f,   10.0f);
        changed |= ImGui::DragFloat("tMax", &trochoidParams.tMax,   0.1f,   1.0f,   100.0f);
        changed |= ImGui::DragFloat("dt",   &trochoidParams.dt,     0.01f,  0.001f, 2.0f);
        if (changed) {
            updateTrochoidBuffer();
        }
    }

    bool changedPoints = false;

    // Euclidean transforms
    if (ImGui::CollapsingHeader("Euclidean Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool changedPosition = ImGui::DragFloat2("Figure Position", glm::value_ptr(figurePosition), 0.1f);
        if (changedPosition) {
            figurePositionBase = computeRotatedPosition(figurePosition, rotatePoint, -angleDegrees);
        }
        changedPoints   |= changedPosition;
        changedPoints   |= ImGui::DragFloat2("Rotate Point", glm::value_ptr(rotatePoint), 0.1f);
        changedPoints   |= ImGui::DragFloat("Angle (deg)", &angleDegrees, 0.5f, -360.0f, 360.0f);
    }

    // Affine transformations
    if (ImGui::CollapsingHeader("Affine Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
        changedPoints |= ImGui::DragFloat2("O", glm::value_ptr(O), 0.1f);
        changedPoints |= ImGui::DragFloat2("X", glm::value_ptr(X), 0.1f);
        changedPoints |= ImGui::DragFloat2("Y", glm::value_ptr(Y), 0.1f);
        ImGui::DragFloat2("Scale", glm::value_ptr(scale), 0.01f, 0.1f, 10.0f);
        ImGui::Checkbox("Is Mirrored", &isMirrored);
    }

    if (changedPoints) {
        updatePointsBuffer();
    }

    // Grid
    if (ImGui::CollapsingHeader("Grid Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Pixels per Unit", &pixelsPerUnit, 1.0f, 10.0f, 200.0f);
        if (ImGui::DragInt("Grid Size", (int*)&gridNum, 1, 2, 50)) {
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
    // Keyboard
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Mouse
    if (processUIWantCaptureMouse() == false) {
        updateDraggablePoints();
        processPanning();
    }
}

glm::vec2 App::screenToWorld(double mouseX, double mouseY, bool useAffine) {
    mouseY = height - mouseY;

    glm::mat4 transform = moveOrigin * scaleToPixels;
    if (useAffine) {
        transform *= affineBasis;
    }

    glm::mat4 inv = glm::inverse(transform);

    glm::vec4 world = inv * glm::vec4(mouseX, mouseY, 0.0f, 1.0f);
    return glm::vec2(world.x, world.y);
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
