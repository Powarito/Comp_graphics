#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "auxiliary/ShaderProgram.h"
#include "auxiliary/Camera.h"
#include "auxiliary/Model.h"

#include "Cosmic.h"
#include "CosmicValues.h"
#include "Skybox.h"


// Window
/*constexpr*/ unsigned int SCR_WIDTH = 1600;
/*constexpr*/ unsigned int SCR_HEIGHT = 1200;
float aspectRatio = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);

int prevWindowModeButtonState = GLFW_RELEASE;
bool isInWindowMode = true;
int windowedWidth = SCR_WIDTH;
int windowedHeight = SCR_HEIGHT;
int windowedPosX = 0;
int windowedPosY = 0;

// Time and FPS
float deltaTime = 0.0f;
float lastTime = 0.0f;

// Mouse
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
int prevMouseButtonState = GLFW_RELEASE;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f * mul));

// Projections
// 'P' = perspective; 'O' = orthographic.
char usedProj = 'P';
glm::mat4 pProj, oProj;
float oProjDiv = 4.0f;

// Colors for background
constexpr float rgb = 255.0f;
float bgBlack[] = { 0.05f, 0.05f, 0.05f, 1.0f };
float bgBlue[]  = { 0.0f / rgb,  3.0f / rgb,  15.0f / rgb,  1.0f };
float bgGrey[]  = { 0.1f, 0.1f, 0.1f, 1.0f };

float* currBg = bgBlue;


constexpr bool clockwiseRotate = true;


struct LightProperties {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float constant;
	float linear;
	float quadratic;
};


// Before moving and drawing cosmic object, you should first move its central object around which it rotates, and so on.
// For example: move stars first, then planets around them, and then moons around these planets.
glm::mat4 moveCosmic(Cosmic &cosmic, float scaleObj = 1.0f);
void      drawCosmic(const ShaderProgram &objectSP, const ShaderProgram &stencilSP, Model *modelObj, bool drawOutline = false);

glm::vec3 getPerpendicularVector(const glm::vec3 &originalVector);
void updateProjections();


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);


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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL window", NULL, NULL);
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

	// Shaders
	ShaderProgram starShaderProgram("res\\shaders\\starShader.vert", "res\\shaders\\starShader.frag");
	ShaderProgram planetShaderProgram("res\\shaders\\planetShader.vert", "res\\shaders\\planetShader.frag");
	ShaderProgram stencilShaderProgram("res\\shaders\\starShader.vert", "res\\shaders\\stencilShader.frag");
	ShaderProgram skyboxShaderProgram("res\\shaders\\skyboxShader.vert", "res\\shaders\\skyboxShader.frag");
	
	// MOVEMENT INFO
	// Stars, planets, moons and their movement information.
	Cosmic stars[] = {
		Cosmic(startStarPositions[0], starSpinSpeed[0], starOrbitRadiuses[0], starOrbitSpeed[0], nullptr),
		Cosmic(startStarPositions[1], starSpinSpeed[1], starOrbitRadiuses[1], starOrbitSpeed[1], &stars[0], {0.0f, 1.0f, 0.0f}, {-1.0f, 4.0f, 1.0f}),
		Cosmic(startStarPositions[2], starSpinSpeed[2], starOrbitRadiuses[2], starOrbitSpeed[2], &stars[0]),
	};
	Cosmic planets[] = {
		Cosmic(startPlanetPositions[0], planetSpinSpeed[0], planetOrbitRadiuses[0], planetOrbitSpeed[0], &stars[0]),
		Cosmic(startPlanetPositions[1], planetSpinSpeed[1], planetOrbitRadiuses[1], planetOrbitSpeed[1], &stars[0]),
		Cosmic(startPlanetPositions[2], planetSpinSpeed[2], planetOrbitRadiuses[2], planetOrbitSpeed[2], &stars[0]),
		Cosmic(startPlanetPositions[3], planetSpinSpeed[3], planetOrbitRadiuses[3], planetOrbitSpeed[3], &stars[0]),
	};
	Cosmic moons[] = {
		Cosmic(startMoonPositions[0], moonSpinSpeed[0], moonOrbitRadiuses[0], moonOrbitSpeed[0], &planets[0], moonSpinAxis[0], moonOrbitAxis[0]),
		Cosmic(startMoonPositions[1], moonSpinSpeed[1], moonOrbitRadiuses[1], moonOrbitSpeed[1], &planets[1], moonSpinAxis[1], moonOrbitAxis[1]),
		Cosmic(startMoonPositions[2], moonSpinSpeed[2], moonOrbitRadiuses[2], moonOrbitSpeed[2], &planets[2], moonSpinAxis[2], moonOrbitAxis[2]),
		Cosmic(startMoonPositions[2], moonSpinSpeed[2], moonOrbitRadiuses[2], moonOrbitSpeed[3], &planets[2], moonSpinAxis[3], moonOrbitAxis[3]),
		Cosmic(startMoonPositions[2], moonSpinSpeed[2], moonOrbitRadiuses[2], moonOrbitSpeed[4], &planets[2], moonSpinAxis[4], moonOrbitAxis[4]),
		Cosmic(startMoonPositions[2], moonSpinSpeed[2], moonOrbitRadiuses[2], moonOrbitSpeed[5], &planets[2], moonSpinAxis[5], moonOrbitAxis[5]),
		Cosmic(startMoonPositions[2], moonSpinSpeed[2], moonOrbitRadiuses[2], moonOrbitSpeed[6], &planets[2], moonSpinAxis[6], moonOrbitAxis[6]),
	};
	stars[1].centralObject = &planets[3];


	// LIGHT COLOR INFO
	float whitenessFactor = 0.60f;
	float diff = 0.10f;
	float spec = 1.0f;
	float shininess = 8.0f;

	glm::vec3 lightColors[] = {
		{ 255.0f / rgb,  213.0f / rgb,  77.0f / rgb },
		{ 233.0f / rgb,  41.0f / rgb,  18.0f / rgb },
		{ 81.0f / rgb,  169.0f / rgb,  255.0f / rgb },
	};
	lightColors[0] = lightColors[0] * (1.0f - whitenessFactor) + whitenessFactor;
	lightColors[1] = lightColors[1]/* * (1.0f - whitenessFactor) + whitenessFactor*/;
	lightColors[2] = lightColors[2] * (1.0f - whitenessFactor) + whitenessFactor;

	// Light properties of stars
	LightProperties lightProps[] = {
		// ambient               diffuse         specular            constant  linear  quadratic
		{ lightColors[0] * diff, lightColors[0], lightColors[0] * spec,   mul, 0.014f, 0.0007f },
		{ lightColors[1] * diff, lightColors[1], lightColors[1] * spec,   mul, 0.14f,  0.07f },
		{ lightColors[2] * diff, lightColors[2], lightColors[2] * spec,   mul, 0.045f, 0.0075f },
	};


	// MODEL INFO
	// All loaded models
	Model loadedModels[] = {
		// Stars
		Model("res\\objects\\waltuh_star\\waltuh.obj"),
		Model("res\\objects\\lava_planet\\scene.gltf"),
		Model("res\\objects\\sirius\\scene.gltf"),
		// Planets
		Model("res\\objects\\south_waltuh\\south_waltuh.obj"),
		Model("res\\objects\\earth\\scene.gltf"),
		Model("res\\objects\\earth_waltuh\\earth_waltuh.obj"),
		Model("res\\objects\\cloudy_earth\\scene.gltf"),
		// Moons
		Model("res\\objects\\mug_root_bear\\mug.obj"),
		Model("res\\objects\\me_gusta_moon\\scene.gltf"),
		Model("res\\objects\\trex\\scene.gltf"),
	};

	// Loaded star, planet and moon models
	Model *starModels[] = {
		&loadedModels[0],
		&loadedModels[1],
		&loadedModels[2],
	};
	Model *planetModels[] = {
		&loadedModels[3],
		&loadedModels[4],
		&loadedModels[5],
		&loadedModels[6],
	};
	Model *moonModels[] = {
		&loadedModels[7],
		&loadedModels[8],
		&loadedModels[9],
		&loadedModels[9],
		&loadedModels[9],
		&loadedModels[9],
		&loadedModels[9],
	};

	float starScales[] = { 3.0f * mul, 0.35f * mul, 0.5f * mul, };
	float planetScales[] = { 1.0f * mul, 0.25f * mul, 1.0f * mul, 1.0f * mul, };
	float moonScales[] = { 0.30f * mul, 0.35f * mul, 0.002f * mul, 0.002f * mul, 0.002f * mul, 0.002f * mul, 0.002f * mul, };

	bool drawStarOutlines[] = { true, false, true };


	// Skybox cubemap texture
	std::string skyboxDir = "blue_sb";
	std::vector<std::string> skyboxFaces {
		"res\\skyboxes\\" + skyboxDir + "\\right.png",
		"res\\skyboxes\\" + skyboxDir + "\\left.png",
		"res\\skyboxes\\" + skyboxDir + "\\top.png",
		"res\\skyboxes\\" + skyboxDir + "\\bottom.png",
		"res\\skyboxes\\" + skyboxDir + "\\front.png",
		"res\\skyboxes\\" + skyboxDir + "\\back.png"
	};
	unsigned int cubemapTexture = loadCubemap(skyboxFaces);

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	
	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		processInput(window);

		float currentTime = static_cast<float>(glfwGetTime());
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Rendering clear commands
		glClearColor(currBg[0], currBg[1], currBg[2], currBg[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Update projections
		updateProjections();

		// Objects
		// Moving and drawing stars
		for (int i = 0; i < sizeof(starModels) / sizeof(Model*); ++i) {
			glm::mat4 model = moveCosmic(stars[i], starScales[i]);

			starShaderProgram.use();
			starShaderProgram.setMat4("model", model);
			starShaderProgram.setMat4("view", camera.GetViewMatrix());
			starShaderProgram.setMat4("projection", usedProj == 'P' ? pProj : oProj);

			stencilShaderProgram.use();
			stencilShaderProgram.setMat4("model", glm::scale(model, glm::vec3(1.025f)));
			stencilShaderProgram.setMat4("view", camera.GetViewMatrix());
			stencilShaderProgram.setMat4("projection", usedProj == 'P' ? pProj : oProj);
			stencilShaderProgram.setVec3("ourColor", (lightColors[i] - whitenessFactor) / (1.0f - whitenessFactor));

			drawCosmic(starShaderProgram, stencilShaderProgram, starModels[i], drawStarOutlines[i]);
		}

		planetShaderProgram.use();

		planetShaderProgram.setFloat("material.shininess", shininess);

		// Let's have only point lights - our stars
		planetShaderProgram.setInt("NR_DIR_LIGHTS", 0);
		planetShaderProgram.setInt("NR_POINT_LIGHTS", sizeof(starModels) / sizeof(Model*));
		planetShaderProgram.setInt("NR_SPOT_LIGHTS", 0);

		planetShaderProgram.setVec3("viewPos", camera.getPosition());

		// Update light info (position to be more precise) in planet fragment shader for correct lighting
		for (int i = 0; i < sizeof(starModels) / sizeof(Model*); ++i) {
			planetShaderProgram.setVec3("pointLights[" + std::to_string(i) + "].ambient",  lightProps[i].ambient);
			planetShaderProgram.setVec3("pointLights[" + std::to_string(i) + "].diffuse",  lightProps[i].diffuse);
			planetShaderProgram.setVec3("pointLights[" + std::to_string(i) + "].specular", lightProps[i].specular);

			planetShaderProgram.setVec3("pointLights[" + std::to_string(i) + "].position", stars[i].position);

			planetShaderProgram.setFloat("pointLights[" + std::to_string(i) + "].constant",  lightProps[i].constant);
			planetShaderProgram.setFloat("pointLights[" + std::to_string(i) + "].linear",    lightProps[i].linear);
			planetShaderProgram.setFloat("pointLights[" + std::to_string(i) + "].quadratic", lightProps[i].quadratic);
		}

		// Moving and drawing planets
		for (int i = 0; i < sizeof(planetModels) / sizeof(Model*); ++i) {
			glm::mat4 model = moveCosmic(planets[i], planetScales[i]);

			planetShaderProgram.use();
			planetShaderProgram.setMat4("model", model);
			planetShaderProgram.setMat4("view", camera.GetViewMatrix());
			planetShaderProgram.setMat4("projection", usedProj == 'P' ? pProj : oProj);
			planetShaderProgram.setMat3("NormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));

			drawCosmic(planetShaderProgram, planetShaderProgram, planetModels[i]);
		}

		// Moving and drawing moons
		for (int i = 0; i < sizeof(moonModels) / sizeof(Model*); ++i) {
			glm::mat4 model = moveCosmic(moons[i], moonScales[i]);

			planetShaderProgram.use();
			planetShaderProgram.setMat4("model", model);
			planetShaderProgram.setMat4("view", camera.GetViewMatrix());
			planetShaderProgram.setMat4("projection", usedProj == 'P' ? pProj : oProj);
			planetShaderProgram.setMat3("NormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));

			drawCosmic(planetShaderProgram, planetShaderProgram, moonModels[i]);
		}

		// Drawing skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShaderProgram.use();
		skyboxShaderProgram.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
		skyboxShaderProgram.setMat4("projection", usedProj == 'P' ? pProj : oProj);
		skyboxShaderProgram.setInt("skybox", 0);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// Check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Release all GLFW resources
	glfwTerminate();

	return 0;
}


// Everytime the size of window is changed, this callback-function is called
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	aspectRatio = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);

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
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && camera.getIsActive()) {
		camera.CameralootAt(glm::normalize(-camera.getPosition()));
	}

	// Right mosue button - toggle show cursor.
	int currentMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (currentMouseButtonState == GLFW_PRESS && prevMouseButtonState == GLFW_RELEASE) {
		// Right mouse button was just clicked
		int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);

		if (cursorMode == GLFW_CURSOR_NORMAL) {
			camera.setIsActive(true);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else if (cursorMode == GLFW_CURSOR_DISABLED) {
			camera.setIsActive(false);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	prevMouseButtonState = currentMouseButtonState;

	// F - toggle fullscreen/windowed mode of the window.
	int currentWindowModeButtonState = glfwGetKey(window, GLFW_KEY_F);
	if (currentWindowModeButtonState == GLFW_PRESS && prevWindowModeButtonState == GLFW_RELEASE) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		// F key was just clicked
		if (isInWindowMode) {
			glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
			glfwGetWindowPos(window, &windowedPosX, &windowedPosY);

			isInWindowMode = false;
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else {
			isInWindowMode = true;
			glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, GLFW_DONT_CARE);
		}
	}
	prevWindowModeButtonState = currentWindowModeButtonState;

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

	// '[' and ']' keys - - decrease or increase camera sensitivity respectively.
	// '\' key - set camera sensitivity to default.
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
		camera.setMouseSensitivity(camera.getMouseSensitivity() - 0.0002f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
		camera.setMouseSensitivity(camera.getMouseSensitivity() + 0.0002f);
	if (glfwGetKey(window, GLFW_KEY_BACKSLASH) == GLFW_PRESS)
		camera.setMouseSensitivity(cam::SENSITIVITY);

	// Check camera sensitivity boundaries.
	if (camera.getMouseSensitivity() > 1.0f)
		camera.setMouseSensitivity(1.0f);
	if (camera.getMouseSensitivity() < 0.0f)
		camera.setMouseSensitivity(0.0f);

	// '-' and '+' keys - decrease or increase camera speed respectively.
	// '0' key - set camera speed to default.
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
		camera.setMovementSpeed(camera.getMovementSpeed() + 0.1f);
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
		camera.setMovementSpeed(camera.getMovementSpeed() - 0.1f);
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
		camera.setMovementSpeed(cam::SPEED);

	// Check camera speed boundaries.
	if (camera.getMovementSpeed() > 30.0f)
		camera.setMovementSpeed(30.0f);
	if (camera.getMovementSpeed() < 0.0f)
		camera.setMovementSpeed(0.0f);
}


glm::mat4 moveCosmic(Cosmic &cosmic, float scaleObj) {
	glm::vec3 centerPos = { 0.0f, 0.0f, 0.0f };
	if (cosmic.centralObject)
		centerPos = { cosmic.centralObject->position };

	float currTime = lastTime;
	if (clockwiseRotate)
		currTime *= -1.0f;

	// 1) Make object rotate around central point of its orbit
	// 2) Rotate object around its orbit with the speed of object's orbitSpeedRad radians per second
	// 3) Circle the orbit at a distance of the orbit's radius
	// 4) Update cosmic object's position
	// 5) Spin cosmic object around its own axis with with the speed of object's spinSpeedRad radians per second
	// 6) Scale the cosmic object

	glm::mat4 model = glm::mat4(1.0f);
	/*  1) */ model = glm::translate(model, glm::vec3(centerPos.x, centerPos.y, centerPos.z));
	/*  2) */ model = glm::rotate(model, currTime * cosmic.orbitSpeedRad, cosmic.orbitAxis);
	/*  3) */ model = glm::translate(model, cosmic.orbitRadius * getPerpendicularVector(cosmic.orbitAxis));

	/*  4) */ cosmic.position = model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	/*  5) */ model = glm::rotate(model, currTime * cosmic.spinSpeedRad, cosmic.spinAxis);
	/*  6) */ model = glm::scale(model, glm::vec3(scaleObj));

	return model;
}

void drawCosmic(const ShaderProgram &objectSP, const ShaderProgram &stencilSP, Model *modelObj, bool drawOutline) {
	if (!drawOutline) {
		glStencilMask(0x00);

		objectSP.use();
		modelObj->Draw(objectSP);
	}
	else {
		// Every fragment which was affected by drawing the object will fill stencil buffer with ones
		// at the same positions where those fragments were drawn
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		objectSP.use();
		modelObj->Draw(objectSP);

		// Then we draw the same object (don't forget to scale it outside the function body by passing it to stencilSP shader program)
		// in the same place, but only those fragments, where stencil buffer doesn't have ones,
		// so that we don't overdraw our object.
		// Also disable writing to the stencil buffer during rendering outline
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);

		stencilSP.use();
		modelObj->Draw(stencilSP);

		// Finally, we clear stencil buffer with zeros
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);
	}
}


glm::vec3 getPerpendicularVector(const glm::vec3 &originalVector) {
	// An arbitrary vector supposed to not be parallel to the originalVector
	glm::vec3 arbitraryVector(-1.156f, -2.86f, 1.6f);

	// Compute the cross product to get a vector perpendicular to both
	glm::vec3 perpendicularVector = glm::cross(originalVector, arbitraryVector);

	// If the cross product is close to zero, it means the vectors are parallel.
	// Choose a different arbitrary vector.
	if (glm::length(perpendicularVector) < 0.001f) {
		arbitraryVector = glm::vec3(0.0f, 1.0f, 0.0f);
		perpendicularVector = glm::cross(originalVector, arbitraryVector);
	}

	// Normalize the resulting vector to make it a unit vector
	return glm::normalize(perpendicularVector);
}

void updateProjections() {
	pProj = glm::perspective(glm::radians(camera.getFov()), aspectRatio, 0.1f, 300.0f);
	oProj = glm::ortho(
		-camera.getFov() / oProjDiv,
		 camera.getFov() / oProjDiv,
		-camera.getFov() / oProjDiv / aspectRatio,
		 camera.getFov() / oProjDiv / aspectRatio,
		 0.1f, 300.0f
	);
}