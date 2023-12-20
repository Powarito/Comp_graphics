#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "auxiliary/ShaderProgram.h"
#include "auxiliary/Camera.h"

#include "GenShapesVertices.h"


/*constexpr*/ unsigned int SCR_WIDTH = 1600;
/*constexpr*/ unsigned int SCR_HEIGHT = 1200;

// Time and FPS
float deltaTime = 0.0f;
float lastTime = 0.0f;

// Mouse
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Camera
Camera camera(glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);

// Projections
// 'P' = perspective; 'O' = orthographic.
char usedProj = 'P';

// Colors for objects
float bgColor[]   = { 0.1f, 0.1f, 0.1f, 1.0f };
float edgeColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float surfColor[] = { 1.0f, 0.3f, 0.3f, 1.0f };
float coneColor[] = { 0.25f, 0.88f, 1.0f, 1.0f };
float cubeColor[] = { 0.0f, 1.0f, 0.65f, 1.0f };

// A function of two variables (Y and Z are swapped, because "up" axis of OpenGL world is Y).
float F(float _X, float _Z) {
	return sin(_X) * sqrt(_Z);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);


int main(int argc, char* argv[]) {
	// Initializing GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Creating window
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	SCR_WIDTH  = vidmode->width;
	SCR_HEIGHT = vidmode->height;

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics Lab 2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all pointers to OpenGL-functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Shaders with projections + colors + textures
	ShaderProgram shaderProgram("shaders/vertShader.vert", "shaders/fragShader.frag");

	std::vector<float> surfaceVertices, coneVertices, cubeVertices;
	surfaceVertices = genSurfVertices(F, 2.0f * M_PI, 1000);
	coneVertices = genConeVertices(1.5f, 3.5f, 25);
	cubeVertices = genCubeVertices(2.0f);

	unsigned int VBO[3], VAO[3];
	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);

	// Surface vertices
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, surfaceVertices.size() * sizeof(float), surfaceVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Cone vertices
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, coneVertices.size() * sizeof(float), coneVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Cube vertices
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(shaderProgram.programID);

	glEnable(GL_DEPTH_TEST);

	glLineWidth(3);

	// Mouse start position
	lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;


	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		processInput(window);

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Rendering commands
		glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram.programID);


		// Every object has its own model matrix. Move cone and cube.
		std::vector<glm::mat4> model(3, glm::mat4(1.0f));
		model[1] = glm::translate(model[1], glm::vec3(2.5f, -1.25f, -2.5f));
		model[2] = glm::translate(model[2], glm::vec3(-2.5f, 0.0f, 2.5f));
		
		// Creating perspective and orthographic projections.
		// perspective perspProj is default projection.		
		float scrAspectRatio = static_cast<float>(SCR_HEIGHT) / SCR_WIDTH;
		glm::mat4 perspProj, orthoProj;
		perspProj = glm::perspective(glm::radians(camera.getFov()), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
		orthoProj = glm::ortho(
			-camera.getFov() / 4.0f, 
			 camera.getFov() / 4.0f, 
			-camera.getFov() / 4.0f * scrAspectRatio, 
			 camera.getFov() / 4.0f * scrAspectRatio,
			 0.1f, 100.0f
		);


		int modelLoc = glGetUniformLocation(shaderProgram.programID, "model");
		int viewLoc = glGetUniformLocation(shaderProgram.programID, "view");
		int projectionLoc = glGetUniformLocation(shaderProgram.programID, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model[0]));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(usedProj == 'P' ? perspProj : orthoProj));

		
		// Draw every object with its own color and position.
		// Surface
		glBindVertexArray(VAO[0]);
		int colorLocation = glGetUniformLocation(shaderProgram.programID, "ourColor");
		glUniform4fv(colorLocation, 1, surfColor);
		glDrawArrays(GL_TRIANGLES, 0, surfaceVertices.size() / 3);

		// Cone
		glBindVertexArray(VAO[1]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model[1]));
		glUniform4fv(colorLocation, 1, coneColor);
		glDrawArrays(GL_TRIANGLES, 0, coneVertices.size() / 3);

		glUniform4fv(colorLocation, 1, edgeColor);
		glDrawArrays(GL_LINE_STRIP, 0, coneVertices.size() / 3);

		// Cube
		glBindVertexArray(VAO[2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model[2]));
		glUniform4fv(colorLocation, 1, cubeColor);
		glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 3);

		glUniform4fv(colorLocation, 1, edgeColor);
		glDrawArrays(GL_LINE_STRIP, 0, cubeVertices.size() / 3);


		// Check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(3, VAO);
	glDeleteBuffers(3, VBO);

	// Release all GLFW resources
	glfwTerminate();

	return 0;
}


// Everytime the size of window is changed, this callback-function is called
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(yOffset);
}

// Process all user input through keyboard
void processInput(GLFWwindow* window) {
	// ESC - close window.
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// 'P' - set perspective projection; 'O' - set orthographic projection.
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		usedProj = 'P';
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		usedProj = 'O';

	// 'Z' - set camera fov to default.
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		camera.setFov(cam::DEF_FOV);
	}

	// Left mouse button - focus camera on some point in the world. By default - on the world center.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		camera.CameralootAt(glm::normalize(-camera.getPosition()));
	}

	// If L. or R.Shift is pressed, then camera will move 2x times faster
	float tempDeltaTime = deltaTime;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
		tempDeltaTime *= 2;
	}

	// WASD or arrows - move camera.
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, tempDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, tempDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::LEFT, tempDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, tempDeltaTime);

	// Space and L.Ctrl - move camera up and down (in world by default, but you can change this in Camera class).
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::UP, tempDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::DOWN, tempDeltaTime);

	// '-' and '+' keys - decrease or increase camera speed respectively.
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
		camera.setMovementSpeed(camera.getMovementSpeed() + 0.1f);
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
		camera.setMovementSpeed(camera.getMovementSpeed() - 0.1f);

	// Check camera speed boundaries.
	if (camera.getMovementSpeed() > 30.0f)
		camera.setMovementSpeed(30.0f);
	if (camera.getMovementSpeed() < 0.0f)
		camera.setMovementSpeed(0.0f);

	// '0' key - set camera speed to default.
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		camera.setMovementSpeed(cam::SPEED);
	}
}