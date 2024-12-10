
#define APIENTRY __stdcall
#define CALLBACK __stdcall
#include <glad.h>
#include "FPSCounter.hpp"
#include "World.hpp"
#include "Camera.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
float windowWidth = 1600.0f, windowHeight = 900.0f;  // 窗口大小
int worldWidth = 256, worldHeight = 8, worldDepth = 256;  // 地图大小
const float PI = acos(-1);

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

// 主循环
int main() {
    srand(time(nullptr));  // 设置随机种子
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }
    
    // 设置错误回调函数
    glfwSetErrorCallback(error_callback);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "MineCraft_OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置 OpenGL 上下文
    glfwMakeContextCurrent(window);

    // 加载 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }

    // 打印 OpenGL 版本
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

    // 创建地图对象
    World world(worldWidth, worldHeight, worldDepth);
    world.generatePerlinMap();  // 生成随机地图

    // 创建摄像机对象
    Camera camera(glm::vec3(worldWidth / 2, worldHeight + 2, worldDepth / 2), world);


    // 创建 FPS 计数器 
    FPSCounter fpsCounter;

    // 启用鼠标捕获和隐藏
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // 隐藏光标
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);  // 设置初始位置（窗口的中心）

    // 设置鼠标回调函数
    camera.attachToWindow(window);

    // 获取窗口大小
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // 设置视口和投影
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;

    // 主循环
    while (!glfwWindowShouldClose(window)) {

        // 渲染
        glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 更新摄像机视角
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.1f, 100.0f);

        // 绘制地图        
        world.render(view, projection);

        // 绘制 FPS
        fpsCounter.update();
        fpsCounter.drawFPS(window);

        // 交换缓冲区
        glfwSwapBuffers(window);


        // 更新摄像机位置
        deltaTime = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();
        camera.updatePosition(deltaTime);  
        // 处理事件, 如键鼠输入
        glfwPollEvents();
    }

    // 清理和退出
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
