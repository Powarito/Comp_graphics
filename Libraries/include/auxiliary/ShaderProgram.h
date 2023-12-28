#pragma once

#include <glad/glad.h> // Include glad to activate the required OpenGL headers
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


namespace sp {
	constexpr size_t logSize = 1024;
}


class ShaderProgram {
public:
	unsigned int programID;

	// Constructor reads and builds the shader
	ShaderProgram(const char* vertexPath, const char* fragmentPath);

	// Use/activate the shader
	inline void use() const {
		glUseProgram(programID);
	}

	// Utility uniform functions
	inline void setBool(const std::string &name, bool value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
	}

	inline void setInt(const std::string &name, int value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
	}

	inline void setFloat(const std::string &name, float value) const {
		glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
	}

    inline void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
    }
    inline void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
    }
    
    inline void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
    }
    inline void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
    }
    
    inline void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
    }
    inline void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
    }
    
    inline void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    inline void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    inline void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};