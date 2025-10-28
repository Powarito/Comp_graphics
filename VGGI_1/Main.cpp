#include <iostream>
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "auxiliary/ShaderProgram.h"
#include "core/App.h"
#include "core/Renderer.h"
#include "math/Transforms.h"
#include "geometry/GenerateGrid.h"
#include "geometry/GenerateTrochoid.h"

unsigned int scrWidth  = 1280;
unsigned int scrHeight = 1200;

glm::vec2 origin        = { 150.0f, 150.0f };
float pixelsPerUnit     = 50.0f;
std::size_t gridNum     = 20;

glm::vec3 bgColor       = { 1.0f, 1.0f, 1.0f };
glm::vec3 gridColor     = { 0.75f, 0.75f, 0.75f };
glm::vec3 AxisXColor    = { 1.0f, 0.0f, 0.0f };
glm::vec3 AxisYColor    = { 0.0f, 1.0f, 0.0f };
glm::vec3 TrochoidColor = { 0.9f, 0.2f, 1.0f };

float r_trochoid        = 1.0f;
float h_trochoid        = 1.0f;
float tMax              = 25.0f;
float dt                = 0.01f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
    App app(1280, 1200);
    app.Run();
    return 0;
}

int main2() {
    Renderer r;
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(scrWidth, scrHeight, "2D Grid and Axes", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glViewport(0, 0, scrWidth, scrHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shader program
    ShaderProgram shader("resources/shaders/shader.vert", "resources/shaders/shader.frag");

    // Generate grid and axes data
    std::vector<float> gridVertices = generateGridVertices(gridNum, gridColor);
    std::vector<float> axesVertices = generateAxesVertices(gridNum, AxisXColor, AxisYColor);

    unsigned int VAO[2], VBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    // Grid VAO/VBO
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Axes VAO/VBO
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(float), axesVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Generate throchoid
    std::vector<float> trochoidVertices = generateTrochoidVertices(
        r_trochoid,
        h_trochoid,
        tMax,
        dt,
        TrochoidColor
    );

    // Trochoid VAO/VBO
    unsigned int VAO_trochoid, VBO_trochoid;
    glGenVertexArrays(1, &VAO_trochoid);
    glGenBuffers(1, &VBO_trochoid);

    glBindVertexArray(VAO_trochoid);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_trochoid);
    glBufferData(GL_ARRAY_BUFFER, trochoidVertices.size() * sizeof(float), trochoidVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    double lastTime = glfwGetTime();
    int nbFrames = 0;

    float currentAngle = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        ++nbFrames;
        double deltaTime = (currentTime - lastTime) / nbFrames; // This is time elapsed per one frame

        if (currentTime - lastTime >= 1.0) { // If at least 1 second has lasted
            nbFrames = 0;
            lastTime += 1.0;
        }

        processInput(window);
        glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(scrWidth), 0.0f, static_cast<float>(scrHeight));
        glm::mat4 moveOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(origin, 0.0f));
        glm::mat4 scaleToPixels = glm::scale(glm::mat4(1.0f), glm::vec3(pixelsPerUnit, pixelsPerUnit, 1.0f));
        shader.setMat4("projection", projection);
        shader.setMat4("moveOrigin", moveOrigin);
        shader.setMat4("scaleToPixels", scaleToPixels);

        glm::vec2 O = { 1.0f, 2.0f }; // Новий початок координат
        glm::vec2 X = { 5.5f, 3.0f }; // Одиничний відрізок по осі X
        glm::vec2 Y = { 1.5f, 3.5f }; // Одиничний відрізок по осі Y

        glm::mat4 affineBasis = buildAffineBasis2D(O, X, Y);
        shader.setMat4("affineBasis", affineBasis);

        // Grid and axes
        shader.setMat4("rawModel", glm::mat4(1.0f));

        glBindVertexArray(VAO[0]);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 5);

        glBindVertexArray(VAO[1]);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, axesVertices.size() / 5);

        float angleSpeed = 90.0f;
        currentAngle += angleSpeed * deltaTime;
        if (currentAngle > 360.0f) {
            int divAngle = static_cast<int>(currentAngle / 360.0f);
            currentAngle -= divAngle * 360.0f;
        }

        // Trochoid
        glm::mat4 trochoidModel = computeTransformMatrix({ 3.0f, -1.0f }, { 3.0f, 0.0f }, currentAngle, { 1.0f, 1.0f }, false, false);
        shader.setMat4("rawModel", trochoidModel);

        glBindVertexArray(VAO_trochoid);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINE_STRIP, 0, trochoidVertices.size() / 5);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAO);
    glDeleteVertexArrays(1, &VAO_trochoid);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(1, &VBO_trochoid);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    scrWidth = width;
    scrHeight = height;

    glViewport(0, 0, width, height);
}

