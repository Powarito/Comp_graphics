#include "App.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <limits>

App::App(unsigned int width, unsigned int height) {
    this->width = width;
    this->height = height;

    initGL();
    initImGui();
    createDefaultCurves();
    initBuffers();
    initDraggablePoints();
}

App::~App() {
    terminateImGui();
    terminateGL();
}

void App::Run() {
    lastFrameTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processInput(window);

        if (isAnimating) {
            updateAnimation(deltaTime);
        }

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

    window = glfwCreateWindow(width, height, "Bezier Curves - Lab 2", nullptr, nullptr);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    float fontScale = 1.5f;
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

    // Rotate point
    std::vector<float> rotatePointVerts = {
        rotatePoint.x, rotatePoint.y,
        rotatePointColor.r, rotatePointColor.g, rotatePointColor.b
    };
    rotatePointRenderData = Renderer::createRenderData(rotatePointVerts, GL_POINTS, 12.0f, GL_DYNAMIC_DRAW);

    updateCurvesBuffers();
}

void App::initDraggablePoints() {
    draggablePoints.clear();

    // Add a pivot point
    draggablePoints.emplace_back(&rotatePoint, rotatePointColor, [this]() {
        updatePointsBuffer();
        });

    // Add all the curve control points
    for (std::size_t i = 0; i < contourManager.curves.size(); ++i) {
        for (std::size_t j = 0; j < 4; ++j) {
            glm::vec3 color = (j == 0 || j == 3) ? glm::vec3{ 1.0f, 0.5f, 0.0f } : glm::vec3{ 0.7f, 0.7f, 0.0f };
            draggablePoints.emplace_back(
                &contourManager.curves[i]->controlPoints[j],
                color,
                [this]() { updateCurvesBuffers(); }
            );
        }
    }
}

void App::createDefaultCurves() {
    // Create several curves that form the default pattern

    // Curve 1 - red
    contourManager.addCurve(BezierCurve(
        { -3.0f, 0.0f }, { -3.0f, 2.0f }, { -1.0f, 2.0f }, { -1.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f }, 50
    ));

    // Curve 2 - green
    contourManager.addCurve(BezierCurve(
        { -1.0f, 0.0f }, { -1.0f, -2.0f }, { 1.0f, -2.0f }, { 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }, 50
    ));

    // Curve 3 - blue
    contourManager.addCurve(BezierCurve(
        { 1.0f, 0.0f }, { 1.0f, 2.0f }, { 3.0f, 2.0f }, { 3.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }, 50
    ));
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

void App::updateCurvesBuffers() {
    // Clean up old data
    for (auto& data : curveRenderDataList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }
    for (auto& data : skeletonLinesList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }
    for (auto& data : skeletonPointsList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }

    curveRenderDataList.clear();
    skeletonLinesList.clear();
    skeletonPointsList.clear();

    // Create new data
    for (std::size_t idx = 0; idx < contourManager.curves.size(); ++idx) {
        const auto& curve = contourManager.curves[idx];

        auto curveVerts = curve->generateCurveVertices();
        curveRenderDataList.push_back(
            Renderer::createRenderData(curveVerts, GL_LINE_STRIP, 3.0f, GL_DYNAMIC_DRAW)
        );

        auto skelLines = curve->generateSkeletonLines();
        skeletonLinesList.push_back(
            Renderer::createRenderData(skelLines, GL_LINES, 1.5f, GL_DYNAMIC_DRAW)
        );

        // Skeleton points - highlight selected curve
        float pointSize = (idx == selectedCurveIdx) ? 15.0f : 10.0f;
        auto skelPoints = curve->generateSkeletonPoints();
        skeletonPointsList.push_back(
            Renderer::createRenderData(skelPoints, GL_POINTS, pointSize, GL_DYNAMIC_DRAW)
        );
    }
}

void App::updatePointsBuffer() {
    std::vector<float> rotatePointVerts = {
        rotatePoint.x, rotatePoint.y,
        rotatePointColor.r, rotatePointColor.g, rotatePointColor.b
    };

    glBindBuffer(GL_ARRAY_BUFFER, rotatePointRenderData.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, rotatePointVerts.size() * sizeof(float), rotatePointVerts.data());
}

void App::updateDraggablePoints() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (mousePressed && !isDragging) {
        const float pickRadiusPixels = 15.0f;

        glm::vec2 mouseWorld = screenToWorld(mouseX, mouseY);

        float minDist = std::numeric_limits<float>::max();
        int pickedIdx = -1;
        glm::vec2 pickedOffset;

        // Priority: point from selected curve
        int selectedCurvePointIdx = -1;
        float selectedCurveMinDist = std::numeric_limits<float>::max();
        glm::vec2 selectedCurveOffset;

        for (int i = 0; i < draggablePoints.size(); ++i) {
            auto& dp = draggablePoints[i];

            // Transform the point position and convert to screen pixels for distance comparison
            glm::vec2 pointTransformed = worldToTransformed(*dp.position);
            glm::vec4 pointScreen4 = moveOrigin * scaleToPixels * glm::vec4(pointTransformed, 0.0f, 1.0f);
            glm::vec2 pointScreen = glm::vec2(pointScreen4.x, pointScreen4.y);

            glm::vec2 mouseScreen = glm::vec2(mouseX, height - mouseY);

            float distPixels = glm::length(mouseScreen - pointScreen);
            if (distPixels < pickRadiusPixels) {
                // Determine which curve this point belongs to
                // First point (index 0) is the rotate point, skip it
                int curveIdx = -1;
                if (i > 0) {
                    int pointIndexInCurves = i - 1; // Subtract rotate point
                    curveIdx = pointIndexInCurves / 4;
                }

                // Check if this point belongs to selected curve
                if (curveIdx == selectedCurveIdx) {
                    if (distPixels < selectedCurveMinDist) {
                        selectedCurvePointIdx = i;
                        selectedCurveMinDist = distPixels;
                        selectedCurveOffset = mouseWorld - pointTransformed;
                    }
                }
                else {
                    // Regular point (not from selected curve)
                    if (distPixels < minDist) {
                        pickedIdx = i;
                        minDist = distPixels;
                        pickedOffset = mouseWorld - pointTransformed;
                    }
                }
            }
        }

        // Prioritize selected curve point if found
        if (selectedCurvePointIdx >= 0) {
            isDragging = true;
            draggedPointIdx = selectedCurvePointIdx;
            dragOffset = selectedCurveOffset;
        }
        else if (pickedIdx >= 0) {
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
        glm::vec2 mouseWorld = screenToWorld(mouseX, mouseY);

        // Convert mouse position back to original space
        glm::vec2 newPosTransformed = mouseWorld - dragOffset;
        *dp.position = transformedToWorld(newPosTransformed);

        if (dp.onUpdate) dp.onUpdate();
    }
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
    rotationMatrix = rotateAroundPoint2D(angleDegrees, rotatePoint);
    transformMatrix = glm::translate(rotationMatrix, glm::vec3(offset, 0.0f));

    shader->setMat4("projection", projection);
    shader->setMat4("moveOrigin", moveOrigin);
    shader->setMat4("scaleToPixels", scaleToPixels);
    shader->setMat4("affineBasis", glm::mat4(1.0f));

    // Grid and axes
    shader->setMat4("rawModel", glm::mat4(1.0f));
    Renderer::Draw(*shader, gridRenderData);
    Renderer::Draw(*shader, axesRenderData);
    Renderer::Draw(*shader, rotatePointRenderData);

    // Curves
    shader->setMat4("rawModel", transformMatrix);
    for (std::size_t i = 0; i < curveRenderDataList.size(); ++i) {
        Renderer::Draw(*shader, curveRenderDataList[i]);

        if (showSkeleton) {
            Renderer::Draw(*shader, skeletonLinesList[i]);
            Renderer::Draw(*shader, skeletonPointsList[i]);
        }
    }
}

void App::renderUI() {
    ImGui::Begin("Bezier Curve Controls");

    if (ImGui::CollapsingHeader("Curves Management", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Total curves: %zu", contourManager.curves.size());

        if (ImGui::Button("(A) Add New Curve")) {
            createNewCurve();
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear All")) {
            contourManager.clear();
            selectedCurveIdx = -1;
            initDraggablePoints();
            updateCurvesBuffers();
        }

        ImGui::Separator();
        ImGui::Text("Selected curve: %d", selectedCurveIdx);
        ImGui::Text("Press 'C' above the curve to select (or use dropdown)");

        if (ImGui::BeginCombo("Select Curve", selectedCurveIdx >= 0 ?
            std::to_string(selectedCurveIdx).c_str() : "None")) {
            for (int i = 0; i < contourManager.curves.size(); ++i) {
                bool isSelected = (selectedCurveIdx == i);
                if (ImGui::Selectable(std::to_string(i).c_str(), isSelected)) {
                    selectedCurveIdx = i;
                    updateCurvesBuffers();
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (selectedCurveIdx >= 0 && selectedCurveIdx < contourManager.curves.size()) {
            auto& curve = contourManager.curves[selectedCurveIdx];

            bool changed = false;
            for (int i = 0; i < 4; ++i) {
                std::string label = "P" + std::to_string(i);
                changed |= ImGui::DragFloat2(label.c_str(), glm::value_ptr(curve->controlPoints[i]), 0.1f);
            }

            changed |= ImGui::ColorEdit3("Color", glm::value_ptr(curve->color));
            changed |= ImGui::DragInt("Segments", &curve->segments, 1, 10, 200);

            if (changed) {
                updateCurvesBuffers();
            }

            if (ImGui::Button("(DEL) Delete Selected")) {
                deleteSelectedCurve();
            }
        }
    }

    if (ImGui::CollapsingHeader("C1 Smoothness", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Smart Connect (recommended):");
        ImGui::Text("1. Hover mouse over endpoint of first curve");
        ImGui::Text("2. Press 'G' to select it");
        ImGui::Text("3. Move to endpoint of second curve");
        ImGui::Text("4. Press 'G' again to connect");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Manual Connect:");
        static int curve1 = 0, curve2 = 1;
        ImGui::DragInt("Curve 1", &curve1, 1, 0, contourManager.curves.size() - 1);
        ImGui::DragInt("Curve 2", &curve2, 1, 0, contourManager.curves.size() - 1);

        if (ImGui::Button("Connect End1 -> Start2")) {
            contourManager.makeC1Smooth(curve1, curve2, true, false);
            updateCurvesBuffers();
        }
        ImGui::SameLine();
        if (ImGui::Button("Connect Start1 -> End2")) {
            contourManager.makeC1Smooth(curve1, curve2, false, true);
            updateCurvesBuffers();
        }

        if (ImGui::Button("Connect Start1 -> Start2")) {
            contourManager.makeC1Smooth(curve1, curve2, false, false);
            updateCurvesBuffers();
        }
        ImGui::SameLine();
        if (ImGui::Button("Connect End1 -> End2")) {
            contourManager.makeC1Smooth(curve1, curve2, true, true);
            updateCurvesBuffers();
        }
    }

    if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("(S) Show Skeleton", &showSkeleton);
    }

    if (ImGui::CollapsingHeader("Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::DragFloat2("Offset", glm::value_ptr(offset), 0.1f)) {
            // Offset updated automatically
        }

        if (ImGui::DragFloat2("Rotate Point", glm::value_ptr(rotatePoint), 0.1f)) {
            updatePointsBuffer();
        }
        ImGui::DragFloat("Angle (deg)", &angleDegrees, 0.5f, -360.0f, 360.0f);
    }

    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button(isAnimating ? "Stop Animation" : "Start Animation")) {
            if (isAnimating) {
                isAnimating = false;
                resetToOriginal();
            }
            else {
                startTriangleAnimation();
            }
        }

        ImGui::DragFloat("Duration (s)", &animationDuration, 0.1f, 0.5f, 10.0f);

        if (isAnimating) {
            ImGui::ProgressBar(animationTime / animationDuration);
        }
    }

    if (ImGui::CollapsingHeader("Save/Load", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char filename[256] = "curves.dat";
        ImGui::InputText("Filename", filename, sizeof(filename));

        if (ImGui::Button("Save")) {
            contourManager.saveToFile(filename);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            contourManager.loadFromFile(filename);
            selectedCurveIdx = -1;
            initDraggablePoints();
            updateCurvesBuffers();
        }
    }

    if (ImGui::CollapsingHeader("Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Pixels per Unit", &pixelsPerUnit, 1.0f, 10.0f, 200.0f);
        if (ImGui::DragInt("Grid Size", (int*)&gridNum, 1, 2, 50)) {
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

    // Handle toggling skeleton visibility with S key
    static bool lastSState = false;
    bool currentSState = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    if (currentSState && !lastSState && !processUIWantCaptureMouse()) {
        showSkeleton = !showSkeleton;
    }
    lastSState = currentSState;

    // Handle pressing A key once
    static bool lastAState = false;
    bool currentAState = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    if (currentAState && !lastAState && !processUIWantCaptureMouse()) {
        createNewCurve();
    }
    lastAState = currentAState;

    // Handle pressing C key
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!processUIWantCaptureMouse()) {
            handleCurveSelection();
        }
    }

    // Handle pressing DEL key once
    static bool lastDELState = false;
    bool currentDELState = glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS;
    if (currentDELState && !lastDELState && !processUIWantCaptureMouse()) {
        deleteSelectedCurve();
    }
    lastDELState = currentDELState;

    // Handle smart smoothness with G key
    static bool lastGState = false;
    bool currentGState = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
    if (currentGState && !lastGState && !processUIWantCaptureMouse()) {
        handleSmartSmoothness();
    }
    lastGState = currentGState;

    if (processUIWantCaptureMouse() == false) {
        updateDraggablePoints();
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

    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
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

glm::vec2 App::screenToWorld(double mouseX, double mouseY) {
    mouseY = height - mouseY;

    glm::mat4 transform = moveOrigin * scaleToPixels;
    glm::mat4 inv = glm::inverse(transform);

    glm::vec4 world = inv * glm::vec4(mouseX, mouseY, 0.0f, 1.0f);
    return glm::vec2(world.x, world.y);
}

glm::vec2 App::worldToTransformed(const glm::vec2& worldPos) {
    glm::vec4 pos = transformMatrix * glm::vec4(worldPos.x, worldPos.y, 0.0f, 1.0f);
    return glm::vec2(pos.x, pos.y);
}

glm::vec2 App::transformedToWorld(const glm::vec2& transformedPos) {
    glm::mat4 inv = glm::inverse(transformMatrix);
    glm::vec4 pos = inv * glm::vec4(transformedPos.x, transformedPos.y, 0.0f, 1.0f);
    return glm::vec2(pos.x, pos.y);
}

void App::terminateGL() {
    glDeleteVertexArrays(1, &gridRenderData.VAO);
    glDeleteBuffers(1, &gridRenderData.VBO);
    glDeleteVertexArrays(1, &axesRenderData.VAO);
    glDeleteBuffers(1, &axesRenderData.VBO);
    glDeleteVertexArrays(1, &rotatePointRenderData.VAO);
    glDeleteBuffers(1, &rotatePointRenderData.VBO);

    for (auto& data : curveRenderDataList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }
    for (auto& data : skeletonLinesList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }
    for (auto& data : skeletonPointsList) {
        glDeleteVertexArrays(1, &data.VAO);
        glDeleteBuffers(1, &data.VBO);
    }

    delete shader;
    glfwTerminate();
}

void App::terminateImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void App::startTriangleAnimation() {
    if (isAnimating) {
        // If already animating, just reverse direction
        animateToTriangle = !animateToTriangle;
        animationTime = 0.0f;
        return;
    }

    isAnimating = true;
    animationTime = 0.0f;

    // Store original positions only if starting fresh animation
    if (animateToTriangle) {
        originalPoints.controlPoints.clear();
        for (const auto& curve : contourManager.curves) {
            originalPoints.controlPoints.push_back(curve->controlPoints);
        }
    }
}

void App::updateAnimation(float deltaTime) {
    animationTime += deltaTime;

    if (animationTime >= animationDuration) {
        animationTime = 0.0f;
        animateToTriangle = !animateToTriangle;

        // If returning to original, continue animation
        if (!animateToTriangle) {
            // Continue to animate back
        }
        else {
            // Finished full cycle, stop
            isAnimating = false;
            return;
        }
    }

    float t = animationTime / animationDuration;
    // Ease in-out
    t = t * t * (3.0f - 2.0f * t);

    std::size_t numCurves = contourManager.curves.size();
    if (numCurves == 0) return;

    // Simple triangle vertices
    glm::vec2 v0 = rotatePoint + glm::vec2{ 0.0f, 3.0f };     // Top
    glm::vec2 v1 = rotatePoint + glm::vec2{ -3.0f, -1.5f };   // Bottom left
    glm::vec2 v2 = rotatePoint + glm::vec2{ 3.0f, -1.5f };    // Bottom right
    std::array<glm::vec2, 3> verts = { v0, v1, v2 };

    // Distribute curves across 3 sides
    int curvesPerSide = numCurves / 3;
    int remainder = numCurves % 3;

    std::array<int, 3> curvesOnSide;
    for (int i = 0; i < 3; ++i) {
        curvesOnSide[i] = curvesPerSide + (i < remainder ? 1 : 0);
    }

    int curveIdx = 0;

    // For each side
    for (int i = 0; i < 3; ++i) {
        int numOnSide = curvesOnSide[i];
        for (int j = 0; j < numOnSide; ++j) {
            if (curveIdx >= numCurves) break;
            auto& curve = contourManager.curves[curveIdx];

            float t0 = static_cast<float>(j) / numOnSide;
            float t1 = static_cast<float>(j + 1) / numOnSide;

            glm::vec2 p0 = verts[i] + (verts[(i + 1) % 3] - verts[i]) * t0;
            glm::vec2 p3 = verts[i] + (verts[(i + 1) % 3] - verts[i]) * t1;
            glm::vec2 p1 = p0 + (p3 - p0) * 0.33f;
            glm::vec2 p2 = p0 + (p3 - p0) * 0.67f;

            if (animateToTriangle) {
                curve->controlPoints[0] = glm::mix(originalPoints.controlPoints[curveIdx][0], p0, t);
                curve->controlPoints[1] = glm::mix(originalPoints.controlPoints[curveIdx][1], p1, t);
                curve->controlPoints[2] = glm::mix(originalPoints.controlPoints[curveIdx][2], p2, t);
                curve->controlPoints[3] = glm::mix(originalPoints.controlPoints[curveIdx][3], p3, t);
            }
            else {
                curve->controlPoints[0] = glm::mix(p0, originalPoints.controlPoints[curveIdx][0], t);
                curve->controlPoints[1] = glm::mix(p1, originalPoints.controlPoints[curveIdx][1], t);
                curve->controlPoints[2] = glm::mix(p2, originalPoints.controlPoints[curveIdx][2], t);
                curve->controlPoints[3] = glm::mix(p3, originalPoints.controlPoints[curveIdx][3], t);
            }

            curveIdx++;
        }
    }

    updateCurvesBuffers();
    initDraggablePoints();
}

void App::resetToOriginal() {
    if (originalPoints.controlPoints.empty()) return;

    for (std::size_t i = 0; i < contourManager.curves.size(); ++i) {
        contourManager.curves[i]->controlPoints = originalPoints.controlPoints[i];
    }

    updateCurvesBuffers();
    initDraggablePoints();
}

void App::handleCurveSelection() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    glm::vec2 mouseWorld = screenToWorld(mouseX, mouseY);

    int curveIdx = findCurveAtPosition(mouseWorld);
    if (curveIdx >= 0) {
        selectedCurveIdx = curveIdx;
        updateCurvesBuffers(); // Update to highlight selected curve
    }
}

int App::findCurveAtPosition(const glm::vec2& worldPos) {
    const float threshold = 20.0f / pixelsPerUnit;

    for (std::size_t i = 0; i < contourManager.curves.size(); ++i) {
        const auto& curve = contourManager.curves[i];

        // Check all control points
        for (int j = 0; j < 4; ++j) {
            glm::vec2 pointTransformed = worldToTransformed(curve->controlPoints[j]);
            float dist = glm::length(worldPos - pointTransformed);
            if (dist < threshold) {
                return static_cast<int>(i);
            }
        }

        // Check points along the curve
        const int samples = 20;
        for (int s = 0; s <= samples; ++s) {
            float t = static_cast<float>(s) / samples;
            glm::vec2 pointOnCurve = curve->evaluate(t);
            glm::vec2 transformedPoint = worldToTransformed(pointOnCurve);
            float dist = glm::length(worldPos - transformedPoint);
            if (dist < threshold) {
                return static_cast<int>(i);
            }
        }
    }

    return -1;
}

void App::createNewCurve(){
    contourManager.addCurve(BezierCurve(
        { 1.0f, 0.0f }, { 2.0f, 1.0f }, { 3.0f, 1.0f }, { 4.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }, 50
    ));
    initDraggablePoints();
    selectedCurveIdx = contourManager.curves.size() - 1;
    updateCurvesBuffers();
}

void App::deleteSelectedCurve() {
    if (selectedCurveIdx >= 0 && selectedCurveIdx < contourManager.curves.size()) {
        contourManager.removeCurve(selectedCurveIdx);
        selectedCurveIdx = -1;
        initDraggablePoints();
        updateCurvesBuffers();
    }
}

void App::handleSmartSmoothness() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    glm::vec2 mouseWorld = screenToWorld(mouseX, mouseY);

    std::size_t curveIdx;
    bool isEnd;
    int result = findCurveEndpoint(mouseWorld, curveIdx, isEnd);

    if (result >= 0) {
        // Found an endpoint under mouse
        if (hoveredEndpointCurve >= 0 && hoveredEndpointCurve != static_cast<int>(curveIdx)) {
            // We have two different curves - apply smoothness!
            contourManager.makeC1Smooth(hoveredEndpointCurve, curveIdx, hoveredIsEnd, isEnd);
            updateCurvesBuffers();
            hoveredEndpointCurve = -1;
        }
        else {
            // Remember first curve endpoint
            hoveredEndpointCurve = static_cast<int>(curveIdx);
            hoveredIsEnd = isEnd;
        }
    }
}

int App::findCurveEndpoint(const glm::vec2& worldPos, std::size_t& curveIdx, bool& isEnd) {
    const float threshold = 20.0f / pixelsPerUnit;

    for (std::size_t i = 0; i < contourManager.curves.size(); ++i) {
        const auto& curve = contourManager.curves[i];

        // Check start point (P0)
        glm::vec2 p0Transformed = worldToTransformed(curve->controlPoints[0]);
        float distStart = glm::length(worldPos - p0Transformed);

        // Check end point (P3)
        glm::vec2 p3Transformed = worldToTransformed(curve->controlPoints[3]);
        float distEnd = glm::length(worldPos - p3Transformed);

        if (distStart < threshold) {
            curveIdx = i;
            isEnd = false;
            return 0;
        }

        if (distEnd < threshold) {
            curveIdx = i;
            isEnd = true;
            return 1;
        }
    }

    return -1;
}

