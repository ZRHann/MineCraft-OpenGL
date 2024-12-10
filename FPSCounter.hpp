#pragma once
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>
class FPSCounter {
private:
    double lastTime;
    int frameCount;
    float fps;

public:
    FPSCounter() : lastTime(0.0), frameCount(0), fps(0.0f) {}

    // 更新 FPS
    void update() {
        frameCount++;
        double currentTime = glfwGetTime();
        
        if (currentTime - lastTime >= 1.0) {
            fps = frameCount / (float)(currentTime - lastTime);
            lastTime = currentTime;
            frameCount = 0;
        }
    }

    // 获取 FPS
    float getFPS() const {
        return fps;
    }

    // 在窗口标题显示 FPS，保留一位小数
    void drawFPS(GLFWwindow* window) const {
        std::stringstream fpsText;
        fpsText << std::fixed << std::setprecision(1);  // 设置精度为 1 位小数
        fpsText << "MineCraft_OpenGL FPS: " << fps;
        
        // 设置窗口标题显示 FPS
        glfwSetWindowTitle(window, fpsText.str().c_str());
    }
};