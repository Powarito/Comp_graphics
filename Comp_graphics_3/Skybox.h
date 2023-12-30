#pragma once

#include <glad/glad.h>

#include "stb_image.h"

#include <iostream>
#include <string>
#include <vector>

extern float skyboxVertices[108];


unsigned int loadCubemap(const std::vector<std::string> &faces);