#define APIENTRY __stdcall
#define CALLBACK __stdcall
#define STB_IMAGE_IMPLEMENTATION
#include "glad.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>
#include <FastNoiseLite.h>
#include <stb_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float windowWidth = 1600.0f, windowHeight = 900.0f;  // 窗口大小
int worldWidth = 64, worldHeight = 4, worldDepth = 64;  // 地图大小
const float PI = acos(-1);

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}


class Shader {
public:
    GLuint programID;

    Shader() : programID(0) {}

    // 加载着色器文件
    std::string loadShaderSource(const std::string& filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        if (file.is_open()) {
            buffer << file.rdbuf();
            file.close();
        } else {
            std::cerr << "Error: Unable to open shader file " << filepath << std::endl;
        }
        return buffer.str();
    }

    // 编译着色器
    GLuint compileShader(const std::string& source, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        // 检查编译是否成功
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Error compiling shader: " << infoLog << std::endl;
        }
        return shader;
    }

    // 创建着色器程序
    void createProgram(const std::string& vertexPath, const std::string& fragmentPath) {
        // 加载顶点着色器和片段着色器的源代码
        std::string vertexSource = loadShaderSource(vertexPath);
        std::string fragmentSource = loadShaderSource(fragmentPath);

        // 编译顶点着色器和片段着色器
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

        // 创建着色器程序并链接着色器
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);

        // 检查程序是否链接成功
        GLint success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(programID, 512, nullptr, infoLog);
            std::cerr << "Error linking program: " << infoLog << std::endl;
        }

        // 删除着色器，因为它们已经被链接到程序中
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // 使用着色器程序
    void use() {
        glUseProgram(programID);
    }

    // 获取uniform变量的位置
    GLint getUniformLocation(const std::string& name) {
        return glGetUniformLocation(programID, name.c_str());
    }

    // 设置uniform变量
    void setUniform1i(const std::string& name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }

    void setUniform1f(const std::string& name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }

    void setUniform3fv(const std::string& name, const GLfloat* value) {
        glUniform3fv(getUniformLocation(name), 1, value);
    }

    void setUniformMatrix4fv(const std::string& name, const GLfloat* matrix) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, matrix);
    }

    ~Shader() {
        if (programID) {
            glDeleteProgram(programID);
        }
    }
};



class Camera {
private:
    glm::vec3 position;  // 摄像机位置
    glm::vec3 front;     // 摄像机前方向
    glm::vec3 up;        // 摄像机上方向
    glm::vec3 right;     // 摄像机右方向
    glm::vec3 worldUp;   // 世界上方向

    float yaw;           // 偏航角
    float pitch;         // 俯仰角

    const float movementSpeed = 0.05f;   // 固定移动速度
    const float mouseSensitivity = 0.1f; // 鼠标灵敏度

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

class TextureManager {
public:
    std::unordered_map<int, GLuint> sideTextures;    // 缓存侧面纹理
    std::unordered_map<int, GLuint> topTextures;     // 缓存顶面纹理
    std::unordered_map<int, GLuint> bottomTextures;  // 缓存底面纹理

    GLuint loadColorTexture(const std::vector<float>& color) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 确保颜色向量有三个元素，分别代表RGB
        if (color.size() != 3) {
            std::cerr << "Invalid color vector size. Must contain exactly 3 elements (R, G, B)." << std::endl;
            return 0;
        }

        // 生成一个 1x1 的纹理，并设置为纯色
        unsigned char colorData[3] = {
            static_cast<unsigned char>(color[0] * 255),
            static_cast<unsigned char>(color[1] * 255),
            static_cast<unsigned char>(color[2] * 255)
        };

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, colorData);
        glGenerateMipmap(GL_TEXTURE_2D);

        return textureID;
    }



    GLuint loadTexture(const std::string& filepath) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            if (nrChannels == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else if (nrChannels == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
        }
        stbi_image_free(data);

        return textureID;
    }

    // 获取方块侧面的纹理
    GLuint getSideTexture(int blockType) {
        if (sideTextures.find(blockType) != sideTextures.end()) {
            return sideTextures[blockType];
        }
        std::string filepath = "assets/grass_block_side.png";
        // GLuint textureID = loadTexture(filepath);
        GLuint textureID = loadColorTexture({0.7f, 0.7f, 0.7f});
        sideTextures[blockType] = textureID;
        return textureID;
    }

    // 获取方块顶面的纹理
    GLuint getTopTexture(int blockType) {
        if (topTextures.find(blockType) != topTextures.end()) {
            return topTextures[blockType];
        }
        std::string filepath = "assets/grass_block_top_greened.png";
        // GLuint textureID = loadTexture(filepath);
        GLuint textureID = loadColorTexture({0.2f, 0.2f, 0.2f});
        topTextures[blockType] = textureID;
        return textureID;
    }

    // 获取方块底面的纹理
    GLuint getBottomTexture(int blockType) {
        if (bottomTextures.find(blockType) != bottomTextures.end()) {
            return bottomTextures[blockType];
        }
        std::string filepath = "assets/grass_block_top.png";
        // GLuint textureID = loadTexture(filepath);
        GLuint textureID = loadColorTexture({0.5f, 0.5f, 0.5f});
        bottomTextures[blockType] = textureID;
        return textureID;
    }
};

class WorldMap {
public:
    int width, height, depth; // 地图的最大尺寸
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组

    GLuint VAO, VBO;  // 用于存储地图顶点的 VAO 和 VBO
    Shader shader;    // 着色器

    WorldMap(int w, int h, int d) : width(w), height(h), depth(d) {
        map.resize(width, std::vector<std::vector<int>>(height, std::vector<int>(depth, 0)));
        shader.createProgram("vertex_shader.glsl", "fragment_shader.glsl");
        setupBuffers();  // 初始化顶点数据
    }

    ~WorldMap() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    // 设置某个位置的方块类型
    void setBlock(int x, int y, int z, int type) {
        if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth) {
            map[x][y][z] = type;
        }
    }

    // 获取某个位置的方块类型
    int getBlock(int x, int y, int z) const {
        if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth) {
            return map[x][y][z];
        }
        return -1; // 如果越界则返回-1
    }

    // 生成随机地图
    void generatePerlinMap() {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.04f);  // 设置频率, 越低越平滑
        int seed = rand();
        std::cout << "Map Seed: " << seed << std::endl;
        noise.SetSeed(seed);  // 设置种子

        for (int x = 0; x < width; ++x) {
            for (int z = 0; z < depth; ++z) {
                float noiseValue = noise.GetNoise((float)x, (float)z);  // 获取噪声值 [-1, 1]

                // 映射噪声值到 [0, 1]
                float normalizedNoise = (noiseValue + 1.0f) / 2.0f;

                // 根据映射后的噪声值计算高度
                int terrainHeight = (int)(normalizedNoise * height) + 1;  // 缩放到实际高度

                // 设置方块
                for (int y = 0; y < height; ++y) {
                    if (y < terrainHeight) {
                        setBlock(x, y, z, 1);  // 地面方块
                    } else {
                        setBlock(x, y, z, 0);  // 空地
                    }
                }
            }
        }
    }

    // 初始化缓冲区
    void setupBuffers() {
        std::vector<float> vertices;

        // 遍历地图生成所有方块的顶点数据
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                for (int z = 0; z < depth; ++z) {
                    if (getBlock(x, y, z) == 1) {
                        addCubeVertices(vertices, x, y, z);
                    }
                }
            }
        }

        // 生成 VAO 和 VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        // 顶点位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 纹理坐标属性
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    // 为一个方块添加顶点
    void addCubeVertices(std::vector<float>& vertices, int x, int y, int z) {
        float cubeVertices[] = {
            // Positions          // Texture Coords
            x,     y,     z,     0.0f, 0.0f,
            x + 1, y,     z,     1.0f, 0.0f,
            x + 1, y + 1, z,     1.0f, 1.0f,
            x,     y + 1, z,     0.0f, 1.0f,

            x,     y,     z + 1, 0.0f, 0.0f,
            x + 1, y,     z + 1, 1.0f, 0.0f,
            x + 1, y + 1, z + 1, 1.0f, 1.0f,
            x,     y + 1, z + 1, 0.0f, 1.0f,
        };

        vertices.insert(vertices.end(), std::begin(cubeVertices), std::end(cubeVertices));
    }

    // 渲染地图
    void render(const glm::mat4& view, const glm::mat4& projection) {
        shader.use();

        // 设置视图和投影矩阵
        shader.setUniformMatrix4fv("view", glm::value_ptr(view));
        shader.setUniformMatrix4fv("projection", glm::value_ptr(projection));

        glBindVertexArray(VAO);

        // 根据方块数量绘制
        glDrawArrays(GL_QUADS, 0, width * height * depth * 24);

        glBindVertexArray(0);
    }
};




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

    // 启用垂直同步
    glfwSwapInterval(1);

    // 创建摄像机对象
    Camera camera(glm::vec3(worldWidth / 2, worldHeight + 2, worldDepth / 2), -90.0f, 0.0f);

    // 创建地图对象
    WorldMap world(worldWidth, worldHeight, worldDepth);
    world.generatePerlinMap();  // 生成随机地图

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
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 处理键盘输入
        camera.processKeyboardInput(window);

        // 渲染
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

        // 处理事件
        glfwPollEvents();
    }

    // 清理和退出
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
