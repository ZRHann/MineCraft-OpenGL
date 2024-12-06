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

float windowWidth = 1600.0f, windowHeight = 900.0f;  // 窗口大小
int worldWidth = 160, worldHeight = 6, worldDepth = 160;  // 地图大小
const float PI = acos(-1);

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
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

class TextureManager {
public:
    std::unordered_map<int, GLuint> sideTextures;    // 缓存侧面纹理
    std::unordered_map<int, GLuint> topTextures;     // 缓存顶面纹理
    std::unordered_map<int, GLuint> bottomTextures;  // 缓存底面纹理

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
        GLuint textureID = loadTexture(filepath);
        sideTextures[blockType] = textureID;
        return textureID;
    }

    // 获取方块顶面的纹理
    GLuint getTopTexture(int blockType) {
        if (topTextures.find(blockType) != topTextures.end()) {
            return topTextures[blockType];
        }
        std::string filepath = "assets/grass_block_top_greened.png";
        GLuint textureID = loadTexture(filepath);
        topTextures[blockType] = textureID;
        return textureID;
    }

    // 获取方块底面的纹理
    GLuint getBottomTexture(int blockType) {
        if (bottomTextures.find(blockType) != bottomTextures.end()) {
            return bottomTextures[blockType];
        }
        std::string filepath = "assets/grass_block_top.png";
        GLuint textureID = loadTexture(filepath);
        bottomTextures[blockType] = textureID;
        return textureID;
    }
};

class WorldMap {
public:
    int width, height, depth; // 地图的最大尺寸
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组

    WorldMap(int w, int h, int d) : width(w), height(h), depth(d) {
        map.resize(width, std::vector<std::vector<int>>(height, std::vector<int>(depth, 0)));
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

    GLuint loadTexture(const char* path) {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return 0;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (nrChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return texture;
    }

    void drawCube(int x, int y, int z, int blockType) {
        // 定义不同面的颜色
        GLfloat sideColor[3] = {0.7f, 0.7f, 0.7f};  // 侧面颜色
        GLuint topTexture = textureManager.getTopTexture(blockType);
        GLuint sideTexture = textureManager.getSideTexture(blockType);  // 假设有侧面纹理
        GLuint bottomTexture = textureManager.getBottomTexture(blockType);  // 假设有底面纹理

        // 绘制正方体的各个面

        // 前面
        glBindTexture(GL_TEXTURE_2D, sideTexture); // 绑定前面纹理
        glBegin(GL_QUADS);
        glColor3fv(sideColor);
        glVertex3f(x, y, z + 1.0f);
        glVertex3f(x + 1.0f, y, z + 1.0f);
        glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f);
        glVertex3f(x, y + 1.0f, z + 1.0f);
        glEnd();

        // 后面
        glBindTexture(GL_TEXTURE_2D, sideTexture); // 绑定后面纹理
        glBegin(GL_QUADS);
        glColor3fv(sideColor);
        glVertex3f(x, y, z);
        glVertex3f(x, y + 1.0f, z);
        glVertex3f(x + 1.0f, y + 1.0f, z);
        glVertex3f(x + 1.0f, y, z);
        glEnd();

        // 左面
        glBindTexture(GL_TEXTURE_2D, sideTexture); // 绑定左面纹理
        glBegin(GL_QUADS);
        glColor3fv(sideColor);
        glVertex3f(x, y, z);
        glVertex3f(x, y, z + 1.0f);
        glVertex3f(x, y + 1.0f, z + 1.0f);
        glVertex3f(x, y + 1.0f, z);
        glEnd();

        // 右面
        glBindTexture(GL_TEXTURE_2D, sideTexture); // 绑定右面纹理
        glBegin(GL_QUADS);
        glColor3fv(sideColor);
        glVertex3f(x + 1.0f, y, z);
        glVertex3f(x + 1.0f, y + 1.0f, z);
        glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f);
        glVertex3f(x + 1.0f, y, z + 1.0f);
        glEnd();

        // 顶面
        glBindTexture(GL_TEXTURE_2D, topTexture); // 绑定顶面纹理
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y + 1.0f, z); // 左下角
        glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + 1.0f, z + 1.0f); // 左上角
        glTexCoord2f(1.0f, 1.0f); glVertex3f(x + 1.0f, y + 1.0f, z + 1.0f); // 右上角
        glTexCoord2f(1.0f, 0.0f); glVertex3f(x + 1.0f, y + 1.0f, z); // 右下角
        glEnd();

        // 底面
        glBindTexture(GL_TEXTURE_2D, bottomTexture); // 绑定底面纹理
        glBegin(GL_QUADS);
        glVertex3f(x, y, z);
        glVertex3f(x + 1.0f, y, z);
        glVertex3f(x + 1.0f, y, z + 1.0f);
        glVertex3f(x, y, z + 1.0f);
        glEnd();
    }




    // 绘制整个地图
    void drawMap() {
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                for (int z = 0; z < depth; ++z) {
                    int blockType = getBlock(x, y, z);
                    if (blockType == 1) {  // 使用1号方块
                        drawCube(x, y, z, blockType);  // 传递方块类型以选择正确的纹理
                    }
                }
            }
        }
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




void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // 从window获取 Camera 实例
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    // 调用 Camera 实例的 mouseCallback 方法
    camera->mouseCallback(window, xpos, ypos);
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

    // 启用垂直同步
    glfwSwapInterval(1);

    // 创建摄像机对象
    Camera camera(worldWidth / 2, worldHeight + 2, worldDepth / 2);

    // 创建地图对象
    WorldMap world(worldWidth, worldHeight, worldDepth);
    world.generatePerlinMap();  // 生成随机地图

    // 创建 FPS 计数器 
    FPSCounter fpsCounter;

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 处理键盘输入
        camera.updatePosition(window);

        // 渲染
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // 更新摄像机视角
        camera.setLookAt();

        // 绘制地图
        world.drawMap();

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
