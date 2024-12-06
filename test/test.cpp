#include "glad.h"
#include "../Camera.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>


// 顶点着色器代码
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    out vec3 vertexColor;  // 传递给片段着色器的颜色
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main()
    {
        gl_Position =  projection * view * vec4(aPos, 1.0);
        vertexColor = aColor;  // 将颜色传递给片段着色器
    }
)";

// 片段着色器代码
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 vertexColor;  // 从顶点着色器传来的颜色
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(vertexColor, 1.0); // 使用颜色
    }
)";

// 将立方体的顶点添加到 `vertices` 向量中
void addCubeVertices(std::vector<float>& vertices, float x, float y, float z) {
    // 每个面由两个三角形组成，共6个顶点
    x /= 100;
    y /= 100;
    z /= 100;
    
    // 顏色的设置：每个面一种颜色
    float colors[6][3] = {
        {1.0f, 0.0f, 0.0f}, // 红色
        {0.0f, 1.0f, 0.0f}, // 绿色
        {0.0f, 0.0f, 1.0f}, // 蓝色
        {1.0f, 1.0f, 0.0f}, // 黄色
        {1.0f, 0.0f, 1.0f}, // 紫色
        {0.0f, 1.0f, 1.0f}  // 青色
    };

    // 每个面由两个三角形组成，总共36个顶点，每个顶点包含位置和颜色信息
    float cubeVertices[] = {
        // Front face (红色)
        x,     y,     z,     1.0f, 0.0f, 0.0f,
        x + 1, y,     z,     1.0f, 0.0f, 0.0f,
        x + 1, y + 1, z,     1.0f, 0.0f, 0.0f,
        x,     y + 1, z,     1.0f, 0.0f, 0.0f,
        x,     y,     z,     1.0f, 0.0f, 0.0f, // Repeat first vertex for second triangle
        x + 1, y + 1, z,     1.0f, 0.0f, 0.0f, // Repeat third vertex for second triangle

        // Back face (绿色)
        x,     y,     z + 1, 0.0f, 1.0f, 0.0f,
        x + 1, y,     z + 1, 0.0f, 1.0f, 0.0f,
        x + 1, y + 1, z + 1, 0.0f, 1.0f, 0.0f,
        x,     y + 1, z + 1, 0.0f, 1.0f, 0.0f,
        x,     y,     z + 1, 0.0f, 1.0f, 0.0f, // Repeat first vertex for second triangle
        x + 1, y + 1, z + 1, 0.0f, 1.0f, 0.0f, // Repeat third vertex for second triangle

        // Left face (蓝色)
        x,     y,     z + 1, 0.0f, 0.0f, 1.0f,
        x,     y,     z,     0.0f, 0.0f, 1.0f,
        x,     y + 1, z,     0.0f, 0.0f, 1.0f,
        x,     y + 1, z + 1, 0.0f, 0.0f, 1.0f,
        x,     y,     z + 1, 0.0f, 0.0f, 1.0f, // Repeat first vertex for second triangle
        x,     y + 1, z,     0.0f, 0.0f, 1.0f, // Repeat third vertex for second triangle

        // Right face (黄色)
        x + 1, y,     z,     1.0f, 1.0f, 0.0f,
        x + 1, y,     z + 1, 1.0f, 1.0f, 0.0f,
        x + 1, y + 1, z + 1, 1.0f, 1.0f, 0.0f,
        x + 1, y + 1, z,     1.0f, 1.0f, 0.0f,
        x + 1, y,     z,     1.0f, 1.0f, 0.0f, // Repeat first vertex for second triangle
        x + 1, y + 1, z + 1, 1.0f, 1.0f, 0.0f, // Repeat third vertex for second triangle

        // Top face (紫色)
        x,     y + 1, z,     1.0f, 0.0f, 1.0f,
        x + 1, y + 1, z,     1.0f, 0.0f, 1.0f,
        x + 1, y + 1, z + 1, 1.0f, 0.0f, 1.0f,
        x,     y + 1, z + 1, 1.0f, 0.0f, 1.0f,
        x,     y + 1, z,     1.0f, 0.0f, 1.0f, // Repeat first vertex for second triangle
        x + 1, y + 1, z + 1, 1.0f, 0.0f, 1.0f, // Repeat third vertex for second triangle

        // Bottom face (青色)
        x,     y,     z,     0.0f, 1.0f, 1.0f,
        x + 1, y,     z,     0.0f, 1.0f, 1.0f,
        x + 1, y,     z + 1, 0.0f, 1.0f, 1.0f,
        x,     y,     z + 1, 0.0f, 1.0f, 1.0f,
        x,     y,     z,     0.0f, 1.0f, 1.0f, // Repeat first vertex for second triangle
        x + 1, y,     z + 1, 0.0f, 1.0f, 1.0f, // Repeat third vertex for second triangle
    };

    vertices.insert(vertices.end(), std::begin(cubeVertices), std::end(cubeVertices));
}

// 编译着色器
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void printMat4(const glm::mat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[i][j] << " "; // 打印每个元素
        }
        std::cout << std::endl; // 每打印一行后换行
    }
}

int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1600, 900, "OpenGL Cube", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW window creation failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置上下文
    glfwMakeContextCurrent(window);

    // 加载 OpenGL 函数指针（不使用 GLEW）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // 隐藏光标
    glfwSetCursorPos(window, 1600 / 2, 900 / 2);  // 设置初始位置（窗口的中心）
    glEnable(GL_DEPTH_TEST);
    // 编译着色器程序
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // 创建 VAO 和 VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    std::vector<float> vertices;
    addCubeVertices(vertices, 16.0f, 16.0f, 16.0f);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 位置
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 颜色
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // 获取 uniform 位置
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");

    Camera camera(glm::vec3(5.0f, 5.0f, 5.0f), 90.0f, 0.0f);
    camera.attachToWindow(window);
    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);
        camera.processKeyboardInput(window);
        glm::mat4 view = camera.getViewMatrix();
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 绘制正方体
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 绘制36个顶点，组成立方体的6个面

        // 交换缓冲区
        glfwSwapBuffers(window);

        // 处理事件
        glfwPollEvents();
    }

    // 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
