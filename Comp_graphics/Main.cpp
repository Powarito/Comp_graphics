#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//#include "ShaderProgram.h"


float aspectRatio = 0.0f;
int currentWindowWidth = 0;
int currentWindowHeight = 0;

float xOffset = 0.0f;
float yOffset = 0.0f;
float moveBy = 0.0f;

const float bgColor[] = { 0.3f, 0.3f, 0.9f, 1.0f };
const float entityColor[] = { 1.0f, 0.6f, 0.14f, 1.0f };

const size_t logSizeChars = 1024;


float vertices[] = {
	// Cords: x, y
	-0.25f,   0.35f,   // head
	-0.10f,   0.35f,   // head
	-0.10f,   0.50f,   // head + neck
	 0.00f,   0.40f,   // neck
	 0.00f,   0.20f,   // neck + bottom neck
	-0.10f,   0.30f,   // neck + bottom neck
	-0.20f,   0.20f,   // bottom neck
	-0.10f,   0.10f,   // bottom neck + wing + torso
	 0.10f,   0.30f,   // wing
	 0.30f,   0.10f,   // wing
	 0.20f,   0.10f,   // torso
	 0.20f,  -0.20f,   // torso
	 0.125f, -0.125f,  // leg
	 0.275f, -0.275f,  // leg
	 0.125f, -0.425f,  // leg
	 0.20f,  -0.35f,   // foot
	 0.20f,  -0.50f,   // foot
	 0.05f,  -0.50f,   // foot
};

unsigned int indices[] = {
	0,  1,  2,   // head
	2,  3,  4,   // neck
	4,  5,  2,   // neck
	4,  5,  6,   // bottom neck
	4,  7,  6,   // bottom neck
	7,  8,  9,   // wing
	7,  10, 11,  // torso
	12, 13, 14,  // leg
	15, 16, 17   // foot
};

unsigned int lineIndices[] = {
	0,  1,    // head
	1,  2,    // head
	2,  0,    // head
	2,  3,    // neck
	3,  4,    // neck
	4,  5,    // neck + bottom neck
	5,  2,    // neck
	5,  6,    // bottom neck
	6,  7,    // bottom neck
	7,  8,    // wing + bottom neck
	8,  9,    // wing
	9,  7,    // wing + torso
	7,  11,   // torso
	10, 11,   // torso
	12, 13,   // leg
	13, 14,   // leg
	12, 14,   // leg
	15, 16,   // foot
	16, 17,   // foot
	15, 17    // foot
};


const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"uniform float aspectRatio;\n"
"uniform float xOffset;\n"
"uniform float yOffset;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(\n"
"        xOffset + aPos.x * aspectRatio, \n"
"        yOffset + aPos.y, \n"
"        0.0, \n"
"        1.0 \n"
"    ); \n"
"}\0";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 aColor;\n"
"void main()\n"
"{\n"
"    FragColor = aColor;\n"
"}\0";


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void checkShaderCompiling(unsigned int shader, char* shaderInfoLog, size_t logSize = logSizeChars);
void checkShaderProgramLinking(unsigned int shader, char* shaderInfoLog, size_t logSize = logSizeChars);


int main() {
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
	GLFWwindow* window = glfwCreateWindow(
		vidmode->width,
		vidmode->height,
		"Computer graphics", 
		//glfwGetPrimaryMonitor(), // Use this parameter instead of NULL from below if you want FULLSCREEN mode
		NULL,
		NULL
	);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all pointers to OpenGL-functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	// Initializing shaders and linking them to shader programs
	char infoLog[logSizeChars];

	unsigned int vertexShader, fragmentShader, shaderProgram;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	checkShaderCompiling(vertexShader, infoLog);
	checkShaderCompiling(fragmentShader, infoLog);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	checkShaderProgramLinking(shaderProgram, infoLog);

	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	// Configuring VAOs
	unsigned int VBO, VAOs[2], EBOs[2];
	glGenVertexArrays(2, VAOs);
	glGenBuffers(1, &VBO);
	glGenBuffers(2, EBOs);

	glBindVertexArray(VAOs[0]); // VAOs[0] is for orange triangles (main body of entity)

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	glBindVertexArray(VAOs[1]); // VAOs[1] is for black lines (edges of entity's body)

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lineIndices), lineIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(0);

	
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	glLineWidth(5);

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		processInput(window);

		// Evaluating time needed to render one frame.
		// It will be used for moving entity in window.
		double currentTime = glfwGetTime();
		++nbFrames;
		moveBy = (currentTime - lastTime) / nbFrames; // This is time elapsed per one frame

		if (currentTime - lastTime >= 1.0) { // If at least 1 second has lasted
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Rendering commands
		glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwGetWindowSize(window, &currentWindowWidth, &currentWindowHeight);
		aspectRatio = static_cast<float>(currentWindowHeight) / currentWindowWidth;

		int vertexRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");
		int vertexXOffsetLocation = glGetUniformLocation(shaderProgram, "xOffset");
		int vertexYOffsetLocation = glGetUniformLocation(shaderProgram, "yOffset");
		int fragmentColorLocation = glGetUniformLocation(shaderProgram, "aColor");

		glUseProgram(shaderProgram);

		glUniform1f(vertexRatioLocation, aspectRatio);
		glUniform1f(vertexXOffsetLocation, xOffset);
		glUniform1f(vertexYOffsetLocation, yOffset);
		glUniform4fv(fragmentColorLocation, 1, entityColor);

		// Drawing main orange body
		glBindVertexArray(VAOs[0]);
		glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);

		// Drawing black edges
		glBindVertexArray(VAOs[1]);
		glUniform4f(fragmentColorLocation, 0.0f, 0.0f, 0.0f, 0.0f);
		glDrawElements(GL_LINES, 40, GL_UNSIGNED_INT, 0);

		// Check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(2, VAOs);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(2, EBOs);

	// Release all GLFW resources
	glfwTerminate();

	return 0;
}


// Everytime the size of window is changed, this callback-function is called
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// Process all user input
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) { yOffset += moveBy; }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) { xOffset -= moveBy; }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) { yOffset -= moveBy; }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { xOffset += moveBy; }
}

// Check if shader has been successfully compiled
void checkShaderCompiling(unsigned int shader, char* shaderInfoLog, size_t logSize) {
	int shaderSuccess;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderSuccess);

	if (!shaderSuccess) {
		glGetShaderInfoLog(shader, logSize, NULL, shaderInfoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << shaderInfoLog << std::endl;
	}
}

// Check if shaders in shader program have been successfully linked
void checkShaderProgramLinking(unsigned int shaderProgram, char* shaderProgramInfoLog, size_t logSize) {
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderProgram, logSize, NULL, shaderProgramInfoLog);
		std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << shaderProgramInfoLog << std::endl;
	}
}