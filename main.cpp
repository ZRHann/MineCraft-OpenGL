#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#define APIENTRY __stdcall
#define CALLBACK __stdcall
#include <GL/glu.h>

float windowWidth = 1600.0f, windowHeight = 900.0f;  // 窗口大小
float PI = acos(-1);

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

// 绘制正方体的函数
void drawCube(float x, float y, float z) {
    glBegin(GL_QUADS);

    // 前面
    glColor3f(1.0f, 0.0f, 0.0f); // 红色
    glVertex3f(x, y, z + 1.0f);
    glVertex3f(x + 1.0f, y, z + 1.0f);
    glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f);
    glVertex3f(x, y + 1.0f, z + 1.0f);

    // 后面
    glColor3f(0.0f, 1.0f, 0.0f); // 绿色
    glVertex3f(x, y, z);
    glVertex3f(x, y + 1.0f, z);
    glVertex3f(x + 1.0f, y + 1.0f, z);
    glVertex3f(x + 1.0f, y, z);

    // 左面
    glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
    glVertex3f(x, y, z);
    glVertex3f(x, y, z + 1.0f);
    glVertex3f(x, y + 1.0f, z + 1.0f);
    glVertex3f(x, y + 1.0f, z);

    // 右面
    glColor3f(1.0f, 1.0f, 0.0f); // 黄色
    glVertex3f(x + 1.0f, y, z);
    glVertex3f(x + 1.0f, y + 1.0f, z);
    glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f);
    glVertex3f(x + 1.0f, y, z + 1.0f);

    // 顶面
    glColor3f(1.0f, 0.0f, 1.0f); // 紫色
    glVertex3f(x, y + 1.0f, z);
    glVertex3f(x, y + 1.0f, z + 1.0f);
    glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f);
    glVertex3f(x + 1.0f, y + 1.0f, z);

    // 底面
    glColor3f(0.0f, 1.0f, 1.0f); // 青色
    glVertex3f(x, y, z);
    glVertex3f(x + 1.0f, y, z);
    glVertex3f(x + 1.0f, y, z + 1.0f);
    glVertex3f(x, y, z + 1.0f);

    glEnd();
}

// 设置透视投影
void setProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 设置透视投影 (视角, 宽高比, 近裁剪面, 远裁剪面)
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

// 摄像机类
class Camera {
public:
    // 摄像机位置
    float cameraX, cameraY, cameraZ;
    float cameraYaw, cameraPitch;

    const float cameraPosSpeed = 0.05f;  // 移动速度
    const float cameraAngleSpeed = 0.03f;  // 角度移动速度

    Camera(float x = 3.0f, float y = 3.0f, float z = 3.0f)
        : cameraX(x), cameraY(y), cameraZ(z), cameraYaw(0.0f), cameraPitch(0.0f) {}

    // 转换角度为弧度
    float radians(float degrees) {
        return degrees * (PI / 180.0f);
    }

    // 设置摄像机的LookAt视角
    void setLookAt() {
        // 转换角度为弧度
        float yawRad = radians(cameraYaw);
        float pitchRad = radians(cameraPitch);

        // 计算方向
        float frontX = cos(yawRad) * cos(pitchRad);
        float frontY = sin(pitchRad);
        float frontZ = sin(yawRad) * cos(pitchRad);

        // 摄像机目标点
        float centerX = cameraX + frontX;
        float centerY = cameraY + frontY;
        float centerZ = cameraZ + frontZ;

        // 使用 gluLookAt 设置视角
        gluLookAt(cameraX, cameraY, cameraZ,  // 摄像机位置
                  centerX, centerY, centerZ,  // 目标位置
                  0.0f, 1.0f, 0.0f);  // 上方向
    }

    // 更新摄像机的位置
    void updatePosition(GLFWwindow* window) {
        // 计算方向（基于yaw）
        float yawRad = radians(cameraYaw);
        float frontX = cos(yawRad);
        float frontZ = sin(yawRad);

        // 相对摄像头的前后、左右方向
        float rightX = frontZ;   // 右方向
        float rightZ = -frontX;  // 右方向

        // 向前/后移动（相对于摄像头方向）
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraX += frontX * cameraPosSpeed;
            cameraZ += frontZ * cameraPosSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraX -= frontX * cameraPosSpeed;
            cameraZ -= frontZ * cameraPosSpeed;
        }

        // 向左/右移动（相对于摄像头方向）
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraX += rightX * cameraPosSpeed;
            cameraZ += rightZ * cameraPosSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraX -= rightX * cameraPosSpeed;
            cameraZ -= rightZ * cameraPosSpeed;
        }

        // 向上/下移动（不受摄像头角度影响）
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cameraY += cameraPosSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cameraY -= cameraPosSpeed;
        }
    }

    // 鼠标回调函数
    void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        static float lastX = windowWidth / 2, lastY = windowHeight / 2;  // 初始化鼠标位置（假设窗口居中）
        
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;  // 注意y轴是反向的

        lastX = xpos;
        lastY = ypos;

        xoffset *= cameraAngleSpeed;
        yoffset *= cameraAngleSpeed;

        // 更新角度
        cameraYaw += xoffset;
        cameraPitch += yoffset;

        // 限制pitch的范围，避免过度旋转
        if (cameraPitch > 89.0f) {
            cameraPitch = 89.0f;
        }
        if (cameraPitch < -89.0f) {
            cameraPitch = -89.0f;
        }
    }
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // 从window获取 Camera 实例
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    // 调用 Camera 实例的 mouseCallback 方法
    camera->mouseCallback(window, xpos, ypos);
}

// 主循环
int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // 设置错误回调函数
    glfwSetErrorCallback(error_callback);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "3D Cube with GLFW", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置 OpenGL 上下文
    glfwMakeContextCurrent(window);

    // 启用垂直同步
    glfwSwapInterval(1);

    // 创建摄像机对象
    Camera camera;

    // 启用鼠标捕获和隐藏
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // 隐藏光标
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);  // 设置初始位置（窗口的中心）

    // 设置鼠标回调函数
    glfwSetWindowUserPointer(window, &camera);  // 将 Camera 对象的指针传给window
    glfwSetCursorPosCallback(window, mouse_callback);

    // 获取窗口大小
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // 设置视口和投影
    glViewport(0, 0, width, height);
    setProjection(width, height);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 处理键盘输入
        camera.updatePosition(window);

        // 渲染
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // 更新摄像机视角
        camera.setLookAt();

        // 绘制正方体
        drawCube(0.0f, 0.0f, 0.0f);

        // 交换缓冲区
        glfwSwapBuffers(window);

        // 处理事件
        glfwPollEvents();
    }

    // 清理和退出
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
