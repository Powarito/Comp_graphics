#pragma once

#include <glad/glad.h> // Include glad to activate the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class ShaderProgram {
public:
	unsigned int programID;

	// Constructor reads and builds the shader
	ShaderProgram(const char* vertexPath, const char* fragmentPath);

	// Use/activate the shader
	void use();

	// Utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
};