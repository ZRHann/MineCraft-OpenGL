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
    const float jumpSpeed = 11.0f; // 起跳速度(最大跳跃高度约1.25)
    float yaw;           // 偏航角
    float pitch;         // 俯仰角

    // 疾跑参数
    const float normalSpeed = 5.0f;     // 正常移动速度
    const float sprintSpeed = 10.0f;    // 疾跑速度
    bool isSprinting = false;           // 是否正在疾跑

    // 飞行参数
    bool isFlying = false;                    // 是否处于飞行状态
    float lastSpacePressTime = 0.0f;         // 上一次按下 Space 键的时间
    const float doubleClickTime = 0.3f;      // 双击判定时间间隔（秒）
    bool lastSpaceState = false;             // 上一帧 Space 键的状态

    const float mouseSensitivity = 0.03f; // 鼠标灵敏度

    float lastX, lastY;  // 上一帧鼠标位置

    bool keys[1024] = { false }; // 用于记录按键状态
    World& world;

    const int windowWidth, windowHeight;

    BlockType blockInHand;
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
        blockInHand = BLOCK_AIR;
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
                // 检测左 Ctrl 键
                if (key == GLFW_KEY_LEFT_CONTROL) {
                    isSprinting = true;
                }
            } else if (action == GLFW_RELEASE) {
                keys[key] = false;
                // 松开左 Ctrl 键
                if (key == GLFW_KEY_LEFT_CONTROL) {
                    isSprinting = false;
                }
            }
        }
        // updatePosition()
    }

    // 脚底y为整数 && 脚底有方块 && 垂直速度为0 -> true
    bool isOnGround() {
        const float eps = 0.01f;      // 精度
        // 脚底范围
        glm::vec3 minBound = position + glm::vec3(-0.3f, -1.62f, -0.3f);
        glm::vec3 maxBound = position + glm::vec3(0.3f, -1.62f, 0.3f);
        // 检查脚底y坐标是否接近整数
        if (std::abs(minBound.y - std::round(minBound.y)) > eps) {
            return false; 
        }
        
        // 获取四个角的坐标
        std::vector<glm::vec3> corners = {
            glm::vec3(minBound.x, minBound.y, minBound.z),
            glm::vec3(maxBound.x, minBound.y, minBound.z),
            glm::vec3(minBound.x, minBound.y, maxBound.z),
            glm::vec3(maxBound.x, minBound.y, maxBound.z)
        };

        // 检查四个角是否有方块支撑
        for (const auto& corner : corners) {
            int blockX = static_cast<int>(std::floor(corner.x));
            int blockY = static_cast<int>(std::round(corner.y)) - 1; 
            int blockZ = static_cast<int>(std::floor(corner.z));
            if (world.getBlock(blockX, blockY, blockZ) != BlockType::BLOCK_AIR) { // 脚下有方块
                return std::abs(velocity.y) < eps; // 检查垂直速度是否为0
            }
        }

        return false;
    }


    // 更新摄像机位置
    void updatePosition(float deltaTime) {
    // 根据是否疾跑来决定移动速度
    float currentSpeed = isSprinting ? sprintSpeed : normalSpeed;
    
    // WASD 移动
    if (keys[GLFW_KEY_W]) {
        position += front * currentSpeed * deltaTime;
    }
    if (keys[GLFW_KEY_S]) {
        position -= front * currentSpeed * deltaTime;
    }
    if (keys[GLFW_KEY_A]) {
        position -= right * currentSpeed * deltaTime;
    }
    if (keys[GLFW_KEY_D]) {
        position += right * currentSpeed * deltaTime;
    }

    // 检测双击 Space
   bool currentSpaceState = keys[GLFW_KEY_SPACE];
    
    // Space 键状态发生变化时
    if (currentSpaceState && !lastSpaceState) {
        float currentTime = glfwGetTime();
        if (currentTime - lastSpacePressTime < doubleClickTime) {
            // Double-click detected, toggle flight mode
            isFlying = !isFlying;
            velocity = glm::vec3(0.0f); // Reset velocity when toggling mode
        }
        lastSpacePressTime = currentTime;
    }
    lastSpaceState = currentSpaceState;

    // 计算潜在的方向
    glm::vec3 direction(0.0f);
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    if (keys[GLFW_KEY_W]) direction += front;        // 飞行模式下使用实际的前方向
    if (keys[GLFW_KEY_S]) direction -= front;
    if (keys[GLFW_KEY_A]) direction -= right;
    if (keys[GLFW_KEY_D]) direction += right;
    
    // 飞行模式下的垂直移动
    if (isFlying) {
        if (keys[GLFW_KEY_SPACE]) direction += worldUp;
        if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT]) direction -= worldUp;
    } else {
        // 非飞行模式下的跳跃
        if (keys[GLFW_KEY_SPACE] && isOnGround()) {
            velocity.y = jumpSpeed; // 初始化向上的速度
        }
    }

    // 如果有方向输入，更新水平速度
    if (glm::length(direction) > 0.0f) {
        direction = glm::normalize(direction);
        if (isFlying) {
            // 飞行模式下的移动
            velocity = direction * movementSpeed;
        } else {
            // 非飞行模式下的移动
            velocity.x = direction.x * movementSpeed;
            velocity.z = direction.z * movementSpeed;
        }
    } else {
        // 无输入时停止水平移动
        if (isFlying) {
            velocity = glm::vec3(0.0f);
        } else {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
    }

    // 垂直速度更新（重力&阻力影响）. 只在非飞行模式下应用重力
    if (!isFlying) {
        velocity.y = (velocity.y + gravity.y * deltaTime) * resistanceFactor;
    }

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
        // std::cout << "velocity: " << velocity.x << " " << velocity.y << " " << velocity.z << std::endl;
        // std::cout << "position: " << position.x << " " << position.y << " " << position.z << std::endl;
}

    // 切换手中方块
    void switchBlockInHand(){
        if (keys[GLFW_KEY_0]) blockInHand = BLOCK_AIR;
        if (keys[GLFW_KEY_1]) blockInHand = GRASS_BLOCK;
        if (keys[GLFW_KEY_2]) blockInHand = OAK_LOG;
        if (keys[GLFW_KEY_3]) blockInHand = OAK_LEAVES;
    }

    // 鼠标回调函数
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
            player->handleLeftClick();
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            Player* player = static_cast<Player*>(glfwGetWindowUserPointer(window));
            player->handRightClick();
        }
    }

    // 处理左键点击
    void handleLeftClick() {
        glm::vec3 blockHit;
        if (world.detectSelectedBlock(position, front, blockHit)) {
            world.updateBlock(static_cast<int>(blockHit.x), static_cast<int>(blockHit.y), static_cast<int>(blockHit.z), BlockType::BLOCK_AIR);
        }
    }

    // 处理右键点击
    void handRightClick() {
        glm::vec3 blockHit;
        if (world.findLastAirBlock(position, front, blockHit)){
            glm::vec3 playerBlockPos = glm::floor(position - glm::vec3(0.0f, 1.62f, 0.0f));// 获取玩家脚底所在的方块
            if (blockHit != playerBlockPos && blockHit != playerBlockPos + glm::vec3(0, 1, 0))
            {
                world.updateBlock(static_cast<int>(blockHit.x), static_cast<int>(blockHit.y), static_cast<int>(blockHit.z), blockInHand);
            }
        }
    }

    // 将摄像机绑定到窗口
    void attachToWindow(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
    }

    // 获取摄像机位置
    glm::vec3 getCameraPosition() const {
        return position;
    }

    // 获取摄像机前方向（光线方向）
    glm::vec3 getRayDirection() const {
        return front;
    }
};
