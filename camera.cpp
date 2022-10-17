//
// Created by Mallikarjun Swamy on 10/15/22.
//


#include "camera.h"


void Camera::processWindowResize(int w, int h) {
    projection = genProj(fov, w, h);
}

void Camera::updateViewMatrix() {
    this->view = glm::lookAt(this->position, this->position + this->direction, this->up);

}


void Camera::processInput(GLFWwindow *window) {

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        GLint w, h;
        glfwGetWindowSize(window, &w, &h);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        std::vector<uint8_t> pixels(3 * w * h);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        for(int line = 0; line != h/2; ++line) {
            std::swap_ranges(pixels.begin() + 3 * w * line,
                             pixels.begin() + 3 * w * (line+1),
                             pixels.begin() + 3 * w * (h-line-1));
        }

        int components = 3;
        stbi_write_png("out.png", w, h, components, pixels.data(), 3 * w);

    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        this->position += this->direction * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        this->position -= glm::cross(this->direction, this->up) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        this->position -= this->direction * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        this->position += glm::cross(this->direction, this->up) * speed;
    }

    updateViewMatrix();
}

void Camera::processCursorPos(double xpos, double ypos) {
    static bool bFirstCall = true;
    static glm::dvec2 lastCursorPos;

    const glm::dvec2 cursorPos(xpos, ypos);
    if (bFirstCall)
    {
        lastCursorPos = cursorPos;
        bFirstCall = false;
    }

    constexpr float sensitivity = 8e-2f;
    const float xoffset = static_cast<float>(lastCursorPos.x - cursorPos.x) * sensitivity;
    const float yoffset = static_cast<float>(lastCursorPos.y - cursorPos.y) * sensitivity;
    lastCursorPos = cursorPos;

    //! create quaternion matrix with up vector and yaw angle.
    auto yawQuat	= glm::angleAxis(glm::radians(xoffset), this->up);
    //! create quaternion matrix with right vector and pitch angle.
    auto pitchQuat	= glm::angleAxis(glm::radians(yoffset), glm::cross(this->direction, this->up));

    this->direction = glm::normalize(yawQuat * pitchQuat) * this->direction;
    updateViewMatrix();
}




