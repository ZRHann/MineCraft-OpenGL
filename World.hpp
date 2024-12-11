#pragma once
#include "TextureManager.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite.h>
#include <vector>
class World {
public:
    int worldWidth, worldHeight, worldDepth; // 地图的最大尺寸
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组

    GLuint VAO, VBO;  // 用于存储地图顶点的 VAO 和 VBO
    Shader world_shader;    // 着色器

    World(int w, int h, int d) : worldWidth(w), worldHeight(h), worldDepth(d) {
        map.resize(worldWidth, std::vector<std::vector<int>>(worldHeight, std::vector<int>(worldDepth, 0)));
        world_shader.createProgram("shaders/World.frag", "shaders/World.vert");
        textureManager.loadTextureArray();
        world_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager.getTextureArrayID());
        world_shader.setUniform1i("textureArray", 0);
        
    }

    ~World() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    // 设置某个位置的方块类型
    void setBlock(int x, int y, int z, int type) {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight && z >= 0 && z < worldDepth) {
            map[x][y][z] = type;
        }
    }

    // 获取某个位置的方块类型
    int getBlock(int x, int y, int z) const {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight && z >= 0 && z < worldDepth) {
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
        std::cout << "World Seed: " << seed << std::endl;
        noise.SetSeed(seed);  // 设置种子

        for (int x = 0; x < worldWidth; ++x) {
            for (int z = 0; z < worldDepth; ++z) {
                float noiseValue = noise.GetNoise((float)x, (float)z);  // 获取噪声值 [-1, 1]

                // 映射噪声值到 [0, 1]
                float normalizedNoise = (noiseValue + 1.0f) / 2.0f;

                // 根据映射后的噪声值计算高度
                int terrainHeight = (int)(normalizedNoise * worldHeight) + 1;  // 缩放到实际高度

                // 设置方块
                for (int y = 0; y < worldHeight; ++y) {
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
        int blockType = 2;  //方块类型：1.草 2.原木 3.树叶(但是灰色实体)

        // 遍历地图生成所有方块的顶点数据
        for (int x = 0; x < worldWidth; ++x) {
            for (int y = 0; y < worldHeight; ++y) {
                for (int z = 0; z < worldDepth; ++z) {
                    if (getBlock(x, y, z) == 1) {
                        addCubeVertices(vertices, x, y, z, blockType);
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

    void addCubeVertices(std::vector<float>& vertices, float x, float y, float z, int blockType) {
        TextureType textureTypeTop;
        TextureType textureTypeSide; 
        TextureType textureTypeBottom; 

        // 草方块
        if (blockType == 1){
            textureTypeTop = TextureType::GRASS_BLOCK_TOP;
            textureTypeSide = TextureType::GRASS_BLOCK_SIDE;
            textureTypeBottom = TextureType::GRASS_BLOCK_BOTTOM;
        }

        // 原木方块
        if (blockType == 2){
            textureTypeTop = TextureType::OAK_LOG_TOP;
            textureTypeSide = TextureType::OAK_LOG_SIDE;
            textureTypeBottom = TextureType::OAK_LOG_TOP;
        }

        // 树叶
        if (blockType == 3){
            textureTypeTop = TextureType::OAK_LOG_LEAVES;
            textureTypeSide = TextureType::OAK_LOG_LEAVES;
            textureTypeBottom = TextureType::OAK_LOG_LEAVES;
        }

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
        world_shader.use();

        // 设置视图和投影矩阵
        world_shader.setUniformMatrix4fv("view", glm::value_ptr(view));
        world_shader.setUniformMatrix4fv("projection", glm::value_ptr(projection));

        glBindVertexArray(VAO);

        // 使用三角形模式绘制
        glDrawArrays(GL_TRIANGLES, 0, 36*worldWidth*worldHeight*worldDepth); // 每个方块有6个顶点，改为36个顶点

        glBindVertexArray(0);
    }

    // 检测选中的方块
    bool detectSelectedBlock(const glm::vec3& cameraPos, const glm::vec3& rayDir, glm::vec3& blockHit) {
        float maxDistance = 7.0f; // 最大检测距离
        float step = 0.1f;          // 每步的移动距离

        for (float distance = 0.0f; distance < maxDistance; distance += step) {
            glm::vec3 currentPos = cameraPos + distance * rayDir;
            int x = static_cast<int>(currentPos.x);
            int y = static_cast<int>(currentPos.y);
            int z = static_cast<int>(currentPos.z);

            if (getBlock(x, y, z) > 0) {
                blockHit = glm::vec3(x, y, z);
                std::cout << "Hit distance: " << distance << std::endl;
                std::cout << "Block hit: " << x << ", " << y << ", " << z << std::endl;
                std::cout << "Block type: " << getBlock(x, y, z) << std::endl;
                return true;
            }
        }

        return false;
    }

    // 删除方块
    void deleteBlock(int x, int y, int z) {
        setBlock(x, y, z, 0); // 设置为空地
        setupBuffers();       // 重新生成缓冲区
    }
};