#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

enum ShaderType {
    SCENE, DEPTH
};

class Shaders
{
public:
	GLuint pid;
    ShaderType type;

	Shaders(ShaderType type, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	~Shaders();

    // activate the shader
    void use()
    {
        glUseProgram(pid);
    }
// utility uniform functions
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(pid, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(pid, name.c_str()), value);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(pid, name.c_str()), 1, &value[0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(pid, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};

