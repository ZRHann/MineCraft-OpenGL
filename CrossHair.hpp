#pragma once
#include "Shader.hpp"
#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

class CrossHair {
private:
    GLuint VAO, VBO;                // 顶点数组对象和顶点缓冲对象
    Shader cross_hair_shader;       // 使用现有的 Shader 类
    int windowWidth, windowHeight;
    float crossHairSize;            // 准心大小（屏幕比例）

    // 更新准心顶点数据
    void updateVertices() {
        // 根据屏幕宽高比调整准心的顶点数据
        float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
        float halfSize = crossHairSize / 2.0f;

        float vertices[] = {
            -halfSize,  0.0f,         // 左侧水平线
             halfSize,  0.0f,
             0.0f,     -halfSize * aspectRatio, // 下方垂直线
             0.0f,      halfSize * aspectRatio,
        };

        // 更新 VBO 数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

public:
    // 构造函数：传入窗口宽高
    CrossHair(int width, int height)
        : windowWidth(width), windowHeight(height) {
        crossHairSize = 0.02f; // 默认准心大小

        // 初始化着色器
        std::string vertexPath = "shaders/CrossHair.vert";
        std::string fragmentPath = "shaders/CrossHair.frag";
        cross_hair_shader.createProgram(vertexPath, fragmentPath);
        
        // 创建 VAO 和 VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        // 初始化 VBO 数据（占位符，稍后更新）
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // 初始化准心顶点数据
        updateVertices();
    }

    // 渲染准心
    void render() {
        // 使用着色器
        cross_hair_shader.use();

        // 渲染准心
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 4); // 两条线，共 4 个顶点
        glBindVertexArray(0);
    }

    // 更新窗口尺寸（可在窗口大小变化时调用）
    void updateWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
        updateVertices(); // 更新准心顶点数据
    }

    // 更新准心大小
    void updateSize(float size) {
        crossHairSize = size;
        updateVertices(); // 更新准心顶点数据
    }

    // 析构函数：释放资源
    ~CrossHair() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};
