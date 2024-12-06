#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
class Camera {
private:
    glm::vec3 position;  // 摄像机位置
    glm::vec3 front;     // 摄像机前方向
    glm::vec3 up;        // 摄像机上方向
    glm::vec3 right;     // 摄像机右方向
    glm::vec3 worldUp;   // 世界上方向

    float yaw;           // 偏航角
    float pitch;         // 俯仰角

    const float movementSpeed = 0.005f;   // 固定移动速度
    const float mouseSensitivity = 0.03f; // 鼠标灵敏度

    float lastX, lastY;  // 上一帧鼠标位置
    bool firstMouse;     // 第一次鼠标移动

public:
    Camera(glm::vec3 startPosition, float startYaw, float startPitch)
        : position(startPosition), yaw(startYaw), pitch(startPitch), firstMouse(true) {
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
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

    // 处理键盘输入
    void processKeyboardInput(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += front * movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= front * movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= right * movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += right * movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position += worldUp * movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            position -= worldUp * movementSpeed;
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

    // 将摄像机绑定到窗口
    void attachToWindow(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, mouseCallback);
    }
};