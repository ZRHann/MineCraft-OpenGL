#pragma once
#include "World.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>


class Player {
private:
    glm::vec3 position;  // 摄像机位置
    glm::vec3 front;     // 摄像机前方向
    glm::vec3 up;        // 摄像机上方向
    glm::vec3 right;     // 摄像机右方向
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);   // 世界上方向
    glm::vec3 velocity = glm::vec3(0.0f);  // 玩家速度
    const glm::vec3 gravity = glm::vec3(0.0f, -32.0f, 0.0f); // 重力加速度
    const float movementSpeed = 7.0f;  // 水平移动速度
    const float resistanceFactor  = 0.98f; // 空气阻力
    float yaw;           // 偏航角
    float pitch;         // 俯仰角

    const float mouseSensitivity = 0.03f; // 鼠标灵敏度

    float lastX, lastY;  // 上一帧鼠标位置

    bool keys[1024] = { false }; // 用于记录按键状态
    World& world;

    const int windowWidth, windowHeight;
public:
    Player(glm::vec3 startPosition, World& world, int width, int height) 
        : position(startPosition) 
        , world(world) 
        , windowWidth(width)
        , windowHeight(height) {
        yaw = -90.0f;
        pitch = 0.0f;
        lastX = width / 2;
        lastY = height / 2;
        updatePlayerVectors();
    }

    // 更新摄像机的前、右、上方向向量
    void updatePlayerVectors() {
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

        updatePlayerVectors();
    }

    // 鼠标回调函数（供GLFW调用）
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
        if (player)
            player->processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    // GLFW 键盘回调函数
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
        if (player)
            player->processKeyInput(key, action);
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
        
        // 计算潜在的方向
        glm::vec3 direction(0.0f);
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        if (keys[GLFW_KEY_W]) direction += horizontalFront;
        if (keys[GLFW_KEY_S]) direction -= horizontalFront;
        if (keys[GLFW_KEY_A]) direction -= horizontalRight;
        if (keys[GLFW_KEY_D]) direction += horizontalRight;

        // 如果有方向输入，更新水平速度
        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
            velocity.x = direction.x * movementSpeed;
            velocity.z = direction.z * movementSpeed;
        } else {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }

        // 垂直速度更新（重力&阻力影响）
        velocity.y = (velocity.y + gravity.y * deltaTime) * resistanceFactor;

        // 基于速度计算潜在的新位置
        glm::vec3 potentialPosition = position + velocity * deltaTime;

        // 逐方向碰撞检测
        glm::vec3 nextPosition = position;

        // X方向
        nextPosition.x = potentialPosition.x;
        glm::vec3 minBound = nextPosition - glm::vec3(0.3f, 1.62f, 0.3f);
        glm::vec3 maxBound = nextPosition + glm::vec3(0.3f, 0.18f, 0.3f);
        if (world.isColliding(minBound, maxBound)) {
            nextPosition.x = position.x; // 恢复原位置
            velocity.x = 0.0f;           // 停止X方向速度
        }

        // Z方向
        nextPosition.z = potentialPosition.z;
        minBound = nextPosition - glm::vec3(0.3f, 1.62f, 0.3f);
        maxBound = nextPosition + glm::vec3(0.3f, 0.18f, 0.3f);
        if (world.isColliding(minBound, maxBound)) {
            nextPosition.z = position.z; // 恢复原位置
            velocity.z = 0.0f;           // 停止Z方向速度
        }

        // Y方向
        nextPosition.y = potentialPosition.y;
        minBound = nextPosition - glm::vec3(0.3f, 1.62f, 0.3f);
        maxBound = nextPosition + glm::vec3(0.3f, 0.18f, 0.3f);
        if (world.isColliding(minBound, maxBound)) {
            if (velocity.y < 0.0f) { // 如果正在下降
                velocity.y = 0.0f;  // 停止Y方向速度
            }
            nextPosition.y = position.y; // 恢复原位置
        }

        // 更新最终位置
        position = nextPosition;
    }

    // 鼠标回调函数
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
            player->handleLeftClick();
        }
    }

    // 处理左键点击
    void handleLeftClick() {
        glm::vec3 blockHit;
        if (world.detectSelectedBlock(position, front, blockHit)) {
            world.updateBlock(static_cast<int>(blockHit.x), static_cast<int>(blockHit.y), static_cast<int>(blockHit.z), BlockType::BLOCK_AIR);
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