#define APIENTRY __stdcall
#define CALLBACK __stdcall
#include <glad.h>
#include "TextureManager.hpp"
#include "FPSCounter.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <FastNoiseLite.h>
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
        textureManager.loadTextureArray();
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager.getTextureArrayID());
        shader.setUniform1i("textureArray", 0);
        
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
        int seed = rand()*rand();
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
        setupBuffers();
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 纹理坐标属性
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 材质信息属性
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void addCubeVertices(std::vector<float>& vertices, float x, float y, float z) {
        TextureType textureTypeTop = TextureType::GRASS_BLOCK_TOP;
        TextureType textureTypeSide = TextureType::GRASS_BLOCK_SIDE;
        TextureType textureTypeBottom = TextureType::GRASS_BLOCK_BOTTOM;

        // 每个面由两个三角形组成，总共36个顶点，每个顶点包含位置、纹理坐标和材质信息
        float cubeVertices[] = {
            // Front face (侧面纹理)
            x,     y,     z,     0.0f, 0.0f, float(textureTypeSide),
            x,     y + 1, z,     0.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z,     1.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z,     1.0f, 1.0f, float(textureTypeSide),
            x + 1, y,     z,     1.0f, 0.0f, float(textureTypeSide),
            x,     y,     z,     0.0f, 0.0f, float(textureTypeSide),

            // Back face (侧面纹理)
            x,     y,     z + 1, 0.0f, 0.0f, float(textureTypeSide),
            x + 1, y,     z + 1, 1.0f, 0.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z + 1, 0.0f, 1.0f, float(textureTypeSide),
            x,     y,     z + 1, 0.0f, 0.0f, float(textureTypeSide),

            // Left face (侧面纹理)
            x,     y,     z + 1, 0.0f, 0.0f, float(textureTypeSide),
            x,     y + 1, z + 1, 0.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z,     1.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z,     1.0f, 1.0f, float(textureTypeSide),
            x,     y,     z,     1.0f, 0.0f, float(textureTypeSide),
            x,     y,     z + 1, 0.0f, 0.0f, float(textureTypeSide),

            // Right face (侧面纹理)
            x + 1, y,     z,     0.0f, 0.0f, float(textureTypeSide),
            x + 1, y + 1, z,     0.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x + 1, y,     z + 1, 1.0f, 0.0f, float(textureTypeSide),
            x + 1, y,     z,     0.0f, 0.0f, float(textureTypeSide),

            // Top face (顶部纹理)
            x,     y + 1, z,     0.0f, 0.0f, float(textureTypeTop),
            x,     y + 1, z + 1, 0.0f, 1.0f, float(textureTypeTop),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeTop),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeTop),
            x + 1, y + 1, z,     1.0f, 0.0f, float(textureTypeTop),
            x,     y + 1, z,     0.0f, 0.0f, float(textureTypeTop),

            // Bottom face (底部纹理)
            x,     y,     z,     0.0f, 0.0f, float(textureTypeBottom),
            x + 1, y,     z,     1.0f, 0.0f, float(textureTypeBottom),
            x + 1, y,     z + 1, 1.0f, 1.0f, float(textureTypeBottom),
            x + 1, y,     z + 1, 1.0f, 1.0f, float(textureTypeBottom),
            x,     y,     z + 1, 0.0f, 1.0f, float(textureTypeBottom),
            x,     y,     z,     0.0f, 0.0f, float(textureTypeBottom),
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

        // 使用三角形模式绘制
        glDrawArrays(GL_TRIANGLES, 0, 36*worldWidth*worldHeight*worldDepth); // 每个方块有6个顶点，改为36个顶点

        glBindVertexArray(0);
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
