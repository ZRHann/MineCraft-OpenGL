
#define APIENTRY __stdcall
#define CALLBACK __stdcall
#include <glad.h>
#include "FPSCounter.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "CrossHair.hpp"
#include "Skybox.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Inventory.hpp"
#include "DayTime.hpp"
// #define DEBUG
#ifdef DEBUG
#define DEBUG_LOG(x) std::cout << x << std::endl;
#else
#define DEBUG_LOG(x)
#endif
const int MSAA_LEVEL = 16;      // 可选值: 0, 1, 2, 4, 8, 16 (默认: 0, 禁用 MSAA)
const float ANISO_LEVEL = 16.0; // 可选值: 1.0, 2.0, 4.0, 8.0, 16.0 (默认: 1.0, 禁用各向异性过滤)
float windowWidth = 1600.0f, windowHeight = 900.0f;  // 窗口大小
int worldWidth = 600, worldHeight = 28, worldDepth = 600;  // 地图大小

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void printGraphicsInfo() {
    const GLubyte* vendor = glGetString(GL_VENDOR); // 显卡厂商
    const GLubyte* renderer = glGetString(GL_RENDERER); // 显卡渲染器
    const GLubyte* version = glGetString(GL_VERSION); // OpenGL版本
    const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // GLSL版本

    std::cout << "[INFO] Graphics Vendor: " << vendor << std::endl;
    std::cout << "[INFO] Graphics Renderer: " << renderer << std::endl;
    std::cout << "[INFO] OpenGL Version: " << version << std::endl;
    std::cout << "[INFO] GLSL Version: " << shadingLanguageVersion << std::endl;
}

void initOpenGLSettings() {
    // 深度测试
    glEnable(GL_DEPTH_TEST);

    // 背面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // 透明混合
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // MSAA 设置
    glEnable(GL_MULTISAMPLE); // 启用多重采样
    glSampleCoverage(1.0f, GL_FALSE); // 确保采样覆盖率为 100%
    std::cout << "[INFO] MSAA enabled with samples: " << MSAA_LEVEL << std::endl;

    // 各向异性过滤设置
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso); // 查询硬件支持的最大各向异性值
    GLfloat anisoLevel = (ANISO_LEVEL > maxAniso) ? maxAniso : ANISO_LEVEL;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisoLevel); // 设置各向异性过滤等级
    std::cout << "[INFO] Anisotropic filtering enabled with level: " << anisoLevel << std::endl;
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

    // 设置 MSAA 级别
    glfwWindowHint(GLFW_SAMPLES, MSAA_LEVEL); 

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

    // 打印显卡信息
    printGraphicsInfo();

    // 设置视口
    glViewport(0, 0, windowWidth, windowHeight);
    // 初始化 OpenGL 设置
    initOpenGLSettings();

    // 创建地图对象
    World world(worldWidth, worldHeight, worldDepth);
    world.generateWorldMap();  // 生成随机地图
    std::cout << "World generated!" << std::endl;

    


    // 创建 FPS 计数器 
    FPSCounter fpsCounter;

    CrossHair crossHair(windowWidth, windowHeight);
    Skybox skybox; 

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // 隐藏光标
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);  // 设置初始位置（窗口的中心）

    // 创建摄像机对象, 设置鼠标回调函数
    Player player(glm::vec3(worldWidth / 2, worldHeight + 2, worldDepth / 2), world, windowWidth, windowHeight);
    player.attachToWindow(window);

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    

    // 注册滚轮回调
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
        Inventory* inv = static_cast<Inventory*>(glfwGetWindowUserPointer(window));
        inv->scrollSlot(yOffset);
    });
    
    float lastFrameTime = glfwGetTime();
    float deltaTime = 0.0f;

    glm::vec3 selectedBlock(0.0f); // 存储选中的方块位置
    // 主循环
    while (!glfwWindowShouldClose(window)) {
        DEBUG_LOG("[DEBUG] Frame start");

        // 清空颜色缓冲和深度缓冲
        glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DEBUG_LOG("[DEBUG] Cleared buffers");

        // 更新摄像机视角
        glm::mat4 view = player.getViewMatrix();
        DEBUG_LOG("[DEBUG] Updated view matrix");

        // 投影矩阵，无穷远投影
        glm::mat4 projection = glm::infinitePerspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f);
        DEBUG_LOG("[DEBUG] Created projection matrix");

        // 绘制地图和准心        
        world.render(view, projection);
        DEBUG_LOG("[DEBUG] Rendered world");

        crossHair.render();
        DEBUG_LOG("[DEBUG] Rendered crosshair");

        // 获取摄像机位置和光线方向
        glm::vec3 cameraPos = player.getCameraPosition();
        glm::vec3 rayDir = player.getRayDirection();
        DEBUG_LOG("[DEBUG] Camera position: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z);

        // 检测选中的方块
        if (world.detectSelectedBlock(cameraPos, rayDir, selectedBlock)) {
            DEBUG_LOG("[DEBUG] Detected block at: " << selectedBlock.x << ", " << selectedBlock.y << ", " << selectedBlock.z);

            // 绘制线框
            world.renderWireframe(view, projection, selectedBlock);
            DEBUG_LOG("[DEBUG] Rendered wireframe");
        }

        // 绘制 FPS
        fpsCounter.update();
        fpsCounter.drawFPS(window);
        DEBUG_LOG("[DEBUG] Rendered FPS");

        // 天空盒
        skybox.render(view, projection);
        DEBUG_LOG("[DEBUG] Rendered skybox");

        // 时间
        DayTime::update(deltaTime);
        DEBUG_LOG("[DEBUG] Updated daytime");

        // 更新粒子系统
        world.particleSystem.update(deltaTime);
        world.particleSystem.render(view, projection);
        DEBUG_LOG("[DEBUG] Updated and rendered particle system");

        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        DEBUG_LOG("[DEBUG] Started ImGui frame");

        // 渲染物品栏
        player.inventory_render();
        DEBUG_LOG("[DEBUG] Rendered inventory");

        // 结束ImGui帧
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        DEBUG_LOG("[DEBUG] Finished ImGui frame");

        // 交换缓冲区
        glfwSwapBuffers(window);
        DEBUG_LOG("[DEBUG] Swapped buffers");

        // 更新摄像机位置
        deltaTime = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();
        player.updatePosition(deltaTime);
        DEBUG_LOG("[DEBUG] Updated player position");

        // 处理事件, 如键鼠输入
        glfwPollEvents();
        DEBUG_LOG("[DEBUG] Polled events");

        // 切换放置的方块
        player.switchBlockInHand();
        DEBUG_LOG("[DEBUG] Switched block in hand");

        DEBUG_LOG("[DEBUG] Frame end");
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 清理和退出
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
