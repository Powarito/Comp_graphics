#include "CosmicValues.h"

// MOVEMENT INFO
float mul = 1.0f;
float starOrbitRadiuses[] = { 0.0f * mul, 2.5f * mul, 34.0f * mul, };
float planetOrbitRadiuses[] = { 9.0f * mul, 15.0f * mul, 22.0f * mul, 28.0f * mul, };
float moonOrbitRadiuses[]   = { 2.0f * mul, 2.5f * mul, 2.0f * mul, };

glm::vec3 startStarPositions[] = {
	glm::vec3(starOrbitRadiuses[0],  0.0f,  0.0f),
	glm::vec3(starOrbitRadiuses[1],  0.0f,  0.0f),
	glm::vec3(starOrbitRadiuses[2],  0.0f,  0.0f),
};
glm::vec3 startPlanetPositions[] = {
	glm::vec3(planetOrbitRadiuses[0],  0.0f,  0.0f),
	glm::vec3(planetOrbitRadiuses[1],  0.0f,  0.0f),
	glm::vec3(planetOrbitRadiuses[2],  0.0f,  0.0f),
	glm::vec3(planetOrbitRadiuses[3],  0.0f,  0.0f),
};
glm::vec3 startMoonPositions[] = {
	glm::vec3(moonOrbitRadiuses[0],  0.0f,  0.0f),
	glm::vec3(moonOrbitRadiuses[1],  0.0f,  0.0f),
	glm::vec3(moonOrbitRadiuses[2],  0.0f,  0.0f),
};

float starSpinSpeed[] = {
	glm::radians(20.0f),
	glm::radians(30.0f),
	glm::radians(40.0f),
};
float planetSpinSpeed[] = {
	glm::radians(35.0f),
	glm::radians(15.0f),
	glm::radians(50.0f),
	glm::radians(-10.0f),
};
float moonSpinSpeed[] = {
	glm::radians(-40.0f),
	glm::radians(20.0f),
	glm::radians(0.0f),
};

float starOrbitSpeed[] = {
	glm::radians(0.0f),
	glm::radians(50.0f),
	glm::radians(25.0f),
};
float planetOrbitSpeed[] = {
	glm::radians(35.0f),
	glm::radians(25.0f),
	glm::radians(20.0f),
	glm::radians(15.0f),
};
float moonOrbitSpeed[] = {
	glm::radians(20.0f),
	glm::radians(15.0f),
	glm::radians(17.0f),
	glm::radians(22.0f),
	glm::radians(27.0f),
	glm::radians(32.0f),
	glm::radians(37.0f),
};
//float starOrbitSpeed[] = { // not orbiting
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//};
//float planetOrbitSpeed[] = { // not orbiting
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//};
//float moonOrbitSpeed[] = { // not orbiting
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//	glm::radians(0.0f),
//};


glm::vec3 moonSpinAxis[] = {
	glm::vec3( 1.0f, 4.0f, 1.0f),
	glm::vec3( 0.0f, 1.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
};

glm::vec3 moonOrbitAxis[] = {
	glm::vec3( 1.0f, 5.0f, 0.0f),
	glm::vec3( 0.0f, 1.0f, 0.0f),
	glm::vec3(-1.0f, 1.0f, 0.0f),
	glm::vec3( 0.0f, 1.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-0.0f, 0.0f, 1.0f),
	glm::vec3( 1.0f, 0.0f, 1.0f),
};