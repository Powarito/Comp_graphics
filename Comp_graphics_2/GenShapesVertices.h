#pragma once

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

// All returned vertices don't need EBO.

// Return surface of a 2-argument function y = _f(x, z) as vertices.
// The range of X and Z values is [-_halfRange; _halfRange].
// _resolution is amount of vertices along X and Z axis, so logically surface is a square grid of vertices with dimensions _resolution x _resolution.
// But in order not to use EBO, we take every square (4 vertices) from grid and represent it as 6 vertices (3 coords each one: 6 * 3 = 18 float values),
// So the size of returned vector is (_resolution - 1) * (_resolution - 1) * 18.
std::vector<float> genSurfVertices(float(*_f)(float, float), float _halfRange, unsigned int _resolution);

// Return surface of a cone as vertices.
// _radius is a redius of the base of the cone.
// _height is a height of the cone.
// _slices is an amount of polygons around vertical central axis of the cone
// (the base of the cone is actually a regular polygon with _slices edges and vertices).
// In order not to use EBO,
// Cone is _slices triangles, and each of three vertices of a triangle is 3 coordinates, so 3 * 3 = 9 float values for each side of the cone.
// P.S. Not including bottom surface.
std::vector<float> genConeVertices(float _radius, float _height, unsigned int _slices);

// Return surface of a cube as vertices.
// _edge is an edge of the cube.
// In order not to use EBO,
// the size of returned vector is 108 (6 sides * 6 vertices (each side is 2 triangles) * 3 coords each one = 108).
std::vector<float> genCubeVertices(float _edge);