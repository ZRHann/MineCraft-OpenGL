#pragma once
#include "World.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>


class Camera {
private:
    glm::vec3 position;  // 摄像机位置
    glm::vec3 front;     // 摄像机前方向
    glm::vec3 up;        // 摄像机上方向
    glm::vec3 right;     // 摄像机右方向
    glm::vec3 worldUp;   // 世界上方向

    float yaw;           // 偏航角
    float pitch;         // 俯仰角

    const float movementSpeed = 4.0f;   // 固定移动速度
    const float mouseSensitivity = 0.03f; // 鼠标灵敏度

    float lastX, lastY;  // 上一帧鼠标位置
    bool firstMouse;     // 第一次鼠标移动

    bool keys[1024] = { false }; // 用于记录按键状态
    World& world;
public:
    Camera(glm::vec3 startPosition, World& world) 
        : position(startPosition) 
        , world(world) {
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        yaw = -90.0f;
        pitch = 0.0f;
        firstMouse = true;
        updateCameraVectors();
    }

    // 更新摄像机的前、右、上方向向量
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    // 获取视图矩阵
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    // 处理鼠标移动输入
    void processMouseMovement(float xpos, float ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos; // 反转y坐标
        lastX = xpos;
        lastY = ypos;

        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset;

        // 限制俯仰角范围
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateCameraVectors();
    }

    // 鼠标回调函数（供GLFW调用）
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (camera)
            camera->processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    // GLFW 键盘回调函数
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (camera)
            camera->processKeyInput(key, action);
    }

    // 处理键盘事件，记录按键状态
    void processKeyInput(int key, int action) {
        if (key >= 0 && key < 1024) {
            if (action == GLFW_PRESS) {
                keys[key] = true;
            } else if (action == GLFW_RELEASE) {
                keys[key] = false;
            }
        }
        // updatePosition();
    }

    // 更新摄像机位置
    void updatePosition(float deltaTime) {
        // std::cout << "w: " << keys[GLFW_KEY_W] << " s: " << keys[GLFW_KEY_S] << " a: " << keys[GLFW_KEY_A] << " d: " << keys[GLFW_KEY_D] << std::endl;
        if (keys[GLFW_KEY_W]) {
            position += front * movementSpeed * deltaTime;
        }
        if (keys[GLFW_KEY_S]) {
            position -= front * movementSpeed * deltaTime;
        }
        if (keys[GLFW_KEY_A]) {
            position -= right * movementSpeed * deltaTime;
        }
        if (keys[GLFW_KEY_D]) {
            position += right * movementSpeed * deltaTime;
        }
        if (keys[GLFW_KEY_SPACE]) {
            position += worldUp * movementSpeed * deltaTime;
        }
        if (keys[GLFW_KEY_LEFT_SHIFT]) {
            position -= worldUp * movementSpeed * deltaTime;
        }
    }

    // 鼠标回调函数
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
            camera->handleLeftClick();
        }
    }

    // 处理左键点击
    void handleLeftClick() {
        glm::vec3 blockHit;
        if (world.detectSelectedBlock(position, front, blockHit)) {
            world.deleteBlock(static_cast<int>(blockHit.x), static_cast<int>(blockHit.y), static_cast<int>(blockHit.z));
        }
    }

    // 将摄像机绑定到窗口
    void attachToWindow(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
    }


    
};
