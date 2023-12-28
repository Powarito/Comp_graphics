#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Cosmic {
	glm::vec3 position;
	float spinSpeedRad;

	float orbitRadius;
	float orbitSpeedRad;
	Cosmic *centralObject;

	glm::vec3 spinAxis;
	glm::vec3 orbitAxis;

	// If centralObject is nullptr, then you can just make it rotate around orbitAxis (or don't move at all)
	explicit inline Cosmic(
		const glm::vec3 &position, 
		float spinSpeedRad, 
		float orbitRadius, 
		float orbitSpeedRad, 
		Cosmic *centralObject,
		const glm::vec3 &spinAxis = glm::vec3(0.0f, 1.0f, 0.0f),
		const glm::vec3 &orbitAxis = glm::vec3(0.0f, 1.0f, 0.0f)
	) : 
		position(position), 
		spinSpeedRad(spinSpeedRad), 
		orbitRadius(orbitRadius), 
		orbitSpeedRad(orbitSpeedRad), 
		centralObject(centralObject),
		spinAxis(spinAxis),
		orbitAxis(orbitAxis)
	{}
};