#pragma once

#include <glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "stb_image_write.h"
#include <string>
#include <vector>




class Camera
{
    static glm::mat4 genView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up) {
        // Camera matrix
        glm::mat4 view = glm::lookAt(
            eye,                // Camera in World Space
            lookat,             // and looks at the origin
            up  // Head is up (set to 0,-1,0 to look upside-down)
        );

        return view;
    }

    static glm::mat4 genProj(float fov, int w, int h) {
        return glm::perspective(glm::radians(fov), (float)w / (float)h, 0.01f, 1000.0f);
    }


public:
    glm::mat4 projection, view;
    glm::vec3 position, direction, up;
    int w, h;
    float fov = 45.0f;
    float speed = 0.01f;


    Camera() {}

    Camera(
        glm::vec3 eye,
        glm::vec3 lookAt,
        glm::vec3 up, int w, int h) : position(eye), direction(lookAt - eye), up(up), w(w), h(h) {
        projection = genProj(fov, w, h);
        view = genView(eye, lookAt, up);
    }

    void processWindowResize(int w, int h);
    void updateViewMatrix();
    void processInput(GLFWwindow *window);
    void processCursorPos(double xpos, double ypos);
};

