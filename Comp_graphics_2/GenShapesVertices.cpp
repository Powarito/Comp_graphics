#include "GenShapesVertices.h"

// Return surface of a 2-argument function y = _f(x, z) as vertices.
// The range of X and Z values is [-_halfRange; _halfRange].
// _resolution is amount of vertices along X and Z axis, so logically surface is a square grid of vertices with dimensions _resolution x _resolution.
// But in order not to use EBO, we take every square (4 vertices) from grid and represent it as 6 vertices (3 coords each one: 6 * 3 = 18 float values),
// So the size of returned vector is (_resolution - 1) * (_resolution - 1) * 18.
std::vector<float> genSurfVertices(float(*_f)(float, float), float _halfRange, unsigned int _resolution) {
	// Surface grid with (_resolution x _resolution) points.
	// In order not to use EBO, We take every square from grid and represent it as 6 vertices (3 coords each one: 6 * 3 = 18).
	std::vector<float> vertices((_resolution - 1) * (_resolution - 1) * 18);

	for (unsigned int i = 0; i < _resolution - 1; ++i) {
		for (unsigned int j = 0; j < _resolution - 1; ++j) {
			// Top-left
			float x1 = -_halfRange + static_cast<float>(i) / (_resolution - 1) * 2.0f * _halfRange;
			float z1 = -_halfRange + static_cast<float>(j) / (_resolution - 1) * 2.0f * _halfRange;
			float y1 = _f(x1, z1);

			// Top-right
			float x2 = -_halfRange + static_cast<float>(i + 1) / (_resolution - 1) * 2.0f * _halfRange;
			float z2 = -_halfRange + static_cast<float>(j) / (_resolution - 1) * 2.0f * _halfRange;
			float y2 = _f(x2, z2);

			// Bottom-left
			float x3 = -_halfRange + static_cast<float>(i) / (_resolution - 1) * 2.0f * _halfRange;
			float z3 = -_halfRange + static_cast<float>(j + 1) / (_resolution - 1) * 2.0f * _halfRange;
			float y3 = _f(x3, z3);

			// Bottom-right
			float x4 = -_halfRange + static_cast<float>(i + 1) / (_resolution - 1) * 2.0f * _halfRange;
			float z4 = -_halfRange + static_cast<float>(j + 1) / (_resolution - 1) * 2.0f * _halfRange;
			float y4 = _f(x4, z4);

			vertices.push_back(x1);
			vertices.push_back(y1);
			vertices.push_back(z1);

			vertices.push_back(x2);
			vertices.push_back(y2);
			vertices.push_back(z2);

			vertices.push_back(x3);
			vertices.push_back(y3);
			vertices.push_back(z3);

			vertices.push_back(x2);
			vertices.push_back(y2);
			vertices.push_back(z2);

			vertices.push_back(x3);
			vertices.push_back(y3);
			vertices.push_back(z3);

			vertices.push_back(x4);
			vertices.push_back(y4);
			vertices.push_back(z4);
		}
	}

	return vertices;
}

// Return surface of a cone as vertices.
// _radius is a redius of the base of the cone.
// _height is a height of the cone.
// _slices is an amount of polygons around vertical central axis of the cone
// (the base of the cone is actually a regular polygon with _slices edges and vertices).
// In order not to use EBO,
// Cone is _slices triangles, and each of three vertices of a triangle is 3 coordinates, so 3 * 3 = 9 float values for each side of the cone.
// P.S. Not including bottom surface.
std::vector<float> genConeVertices(float _radius, float _height, unsigned int _slices) {
	// Cone is _slices triangles, and each of three vertices of a triangle is 3 coordinates, so 3 * 3 = 9.
	// P.S. Not including bottom surface.
	std::vector<float> vertices(_slices * 9);

	for (unsigned int i = 0; i < _slices; ++i) {
		float theta = static_cast<float>(i) / static_cast<float>(_slices) * 2.0f * M_PI;
		float x = _radius * cos(theta);
		float z = _radius * sin(theta);

		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);

		theta = static_cast<float>(i + 1) / static_cast<float>(_slices) * 2.0f * M_PI;
		x = _radius * cos(theta);
		z = _radius * sin(theta);

		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);

		vertices.push_back(0.0f);
		vertices.push_back(_height);
		vertices.push_back(0.0f);

		// If you need to draw bottom surface of a cone, you can use first two points, and then point (0, 0, 0),
		// then add them to vartices.
		// This way, you will make bottom surface of a cone.
	}

	return vertices;
}

// Return surface of a cube as vertices.
// _edge is an edge of the cube.
// In order not to use EBO,
// the size of returned vector is 108 (6 sides * 6 vertices (each side is 2 triangles) * 3 coords each one = 108).
std::vector<float> genCubeVertices(float _side) {
	const float s2 = _side / 2;

	return {
		-s2, -s2, -s2,
		 s2, -s2, -s2,
		 s2,  s2, -s2,
		 s2,  s2, -s2,
		-s2,  s2, -s2,
		-s2, -s2, -s2,

		-s2, -s2,  s2,
		 s2, -s2,  s2,
		 s2,  s2,  s2,
		 s2,  s2,  s2,
		-s2,  s2,  s2,
		-s2, -s2,  s2,

		-s2,  s2,  s2,
		-s2,  s2, -s2,
		-s2, -s2, -s2,
		-s2, -s2, -s2,
		-s2, -s2,  s2,
		-s2,  s2,  s2,

		 s2,  s2,  s2,
		 s2,  s2, -s2,
		 s2, -s2, -s2,
		 s2, -s2, -s2,
		 s2, -s2,  s2,
		 s2,  s2,  s2,

		-s2, -s2, -s2,
		 s2, -s2, -s2,
		 s2, -s2,  s2,
		 s2, -s2,  s2,
		-s2, -s2,  s2,
		-s2, -s2, -s2,

		-s2,  s2, -s2,
		 s2,  s2, -s2,
		 s2,  s2,  s2,
		 s2,  s2,  s2,
		-s2,  s2,  s2,
		-s2,  s2, -s2,
	};
}