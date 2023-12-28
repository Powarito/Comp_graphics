#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Cosmic.h"


// MOVEMENT INFO
extern float mul;
extern float starOrbitRadiuses[];
extern float planetOrbitRadiuses[];
extern float moonOrbitRadiuses[];

extern glm::vec3 startStarPositions[];
extern glm::vec3 startPlanetPositions[];
extern glm::vec3 startMoonPositions[];

extern float starSpinSpeed[];
extern float planetSpinSpeed[];
extern float moonSpinSpeed[];

extern float starOrbitSpeed[];
extern float planetOrbitSpeed[];
extern float moonOrbitSpeed[];

extern glm::vec3 moonSpinAxis[];
extern glm::vec3 moonOrbitAxis[];