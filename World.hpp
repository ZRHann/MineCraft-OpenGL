#pragma once
#include "TextureManager.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite.h>
#include <vector>

enum BlockType {
    BLOCK_AIR,
    GRASS_BLOCK,
    OAK_LOG,
    OAK_LEAVES
};

// 32位随机数生成器
int rand32() {
    return rand() << 16 | rand();
}


class World {
public:
    const int vertexCountPerBlock = 36;
    const int attributesPerVertex = 6;
    const int blockStride = vertexCountPerBlock * attributesPerVertex;
    const int maxTreeHeight = 7; // 树木最大高度
    int worldWidth, worldHeight, worldDepth; // 地图的最大尺寸
    int worldSeed; // 地图种子
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组

    GLuint VAO, VBO;  // 用于存储地图顶点的 VAO 和 VBO
    Shader world_shader;    // 着色器

    World(int w, int h, int d) : worldWidth(w), worldHeight(h), worldDepth(d) {
        map.resize(worldWidth, std::vector<std::vector<int>>(worldHeight, std::vector<int>(worldDepth, 0)));


        // 初始化着色器、纹理
        world_shader.createProgram("shaders/World.vert", "shaders/World.frag");
        textureManager.loadTextureArray();
        world_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager.getTextureArrayID());
        world_shader.setUniform1i("textureArray", 0);
        
        srand(time(nullptr));
        worldSeed = rand32();
        srand(worldSeed);
        std::cout << "World Seed: " << worldSeed << std::endl;
    }

    ~World() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    // 设置某个位置的方块类型
    void setBlock(int x, int y, int z, BlockType type) {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight && z >= 0 && z < worldDepth) {
            map[x][y][z] = type;
        }
    }

    // 获取某个位置的方块类型
    BlockType getBlock(int x, int y, int z) const {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight && z >= 0 && z < worldDepth) {
            return static_cast<BlockType>(map[x][y][z]);
        }
        return BlockType::BLOCK_AIR;
    }

    // 生成随机地图
    void generateWorldMap() {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.04f);  // 设置频率, 越低越平滑
        int perlinSeed = rand32();
        noise.SetSeed(perlinSeed);  // 设置种子

        for (int x = 0; x < worldWidth; ++x) {
            for (int z = 0; z < worldDepth; ++z) {
                float noiseValue = noise.GetNoise((float)x, (float)z);  // 获取噪声值 [-1, 1]

                // 映射噪声值到 [0, 1]
                float normalizedNoise = (noiseValue + 1.0f) / 2.0f;

                // 根据映射后的噪声值计算高度
                int terrainHeight = (int)(normalizedNoise * (worldHeight - maxTreeHeight - 1)) + 1; // 高度范围 [1, worldHeight - maxTreeHeight]
                terrainHeight = std::max(terrainHeight, 1);  // 最小高度为1
                // 设置方块
                for (int y = 0; y < worldHeight; ++y) {
                    if (y < terrainHeight) {
                        setBlock(x, y, z, BlockType::GRASS_BLOCK);  // 地面方块
                    } else {
                        setBlock(x, y, z, BlockType::BLOCK_AIR);  // 空地
                    }
                }
                // 随机生成树木
                const float treeDensity = 0.001f;
                if (terrainHeight > 1 && rand32() % 10000 < treeDensity * 10000) {
                    if (canPlaceTree(x, z)) {
                        placeTree(x, terrainHeight, z);
                    }
                }
            }
        }
        setupBuffers();
    }

    // 初始化VAO VBO缓冲区
    void setupBuffers() {
        std::vector<float> vertices;

        // 遍历地图生成所有方块的顶点数据
        for (int x = 0; x < worldWidth; ++x) {
            for (int y = 0; y < worldHeight; ++y) {
                for (int z = 0; z < worldDepth; ++z) {
                    std::vector<float> cubeVertices = getCubeVertices(x, y, z, getBlock(x, y, z));
                    vertices.insert(vertices.end(), cubeVertices.begin(), cubeVertices.end());
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

    std::vector<float> getCubeVertices(float x, float y, float z, int blockType) {
        std::vector<float> cubeVertices(blockStride);

        TextureType textureTypeTop;
        TextureType textureTypeSide; 
        TextureType textureTypeBottom; 

        if (blockType == BlockType::BLOCK_AIR) {
            textureTypeTop = TextureType::TEXTURE_AIR;
            textureTypeSide = TextureType::TEXTURE_AIR;
            textureTypeBottom = TextureType::TEXTURE_AIR;
        }
        if (blockType == BlockType::GRASS_BLOCK) { // 草方块
            textureTypeTop = TextureType::GRASS_BLOCK_TOP;
            textureTypeSide = TextureType::GRASS_BLOCK_SIDE;
            textureTypeBottom = TextureType::GRASS_BLOCK_BOTTOM;
        }
        else if (blockType == BlockType::OAK_LOG){ // 原木方块
            textureTypeTop = TextureType::OAK_LOG_TOP;
            textureTypeSide = TextureType::OAK_LOG_SIDE;
            textureTypeBottom = TextureType::OAK_LOG_TOP;
        }
        else if (blockType == BlockType::OAK_LEAVES) { // 树叶方块
            textureTypeTop = TextureType::OAK_LOG_LEAVES;
            textureTypeSide = TextureType::OAK_LOG_LEAVES;
            textureTypeBottom = TextureType::OAK_LOG_LEAVES;
        }

        // 每个面由两个三角形组成，总共36个顶点，每个顶点包含位置、纹理坐标和材质信息
        cubeVertices = {
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
        return cubeVertices;
    }

    bool canPlaceTree(int x, int z) {
        int treeSpacing = 5; // 树木间隔

        // 遍历以 (x, z) 为中心的间隔范围
        for (int dx = -treeSpacing; dx <= treeSpacing; ++dx) {
            for (int dz = -treeSpacing; dz <= treeSpacing; ++dz) {
                if (x + dx >= 0 && x + dx < worldWidth && z + dz >= 0 && z + dz < worldDepth) {
                    // 遍历 y 轴，检查整列是否有树干
                    for (int y = 0; y < worldHeight; ++y) {
                        if (getBlock(x + dx, y, z + dz) == BlockType::OAK_LOG || 
                            getBlock(x + dx, y, z + dz) == BlockType::OAK_LEAVES) { 
                            return false; // 如果检测到树干或树叶，返回 false
                        }
                    }
                }
            }
        }
        return true;
    }

    /*
        生成树木
        x, z: 树木的位置
        baseHeight: 树底的高度
        树的总高度: 4/5/6/7
        树干高度为树的总高度 - 1
    */
    void placeTree(int x, int baseHeight, int z) {
        int treeHeight = 4 + rand() % 4; // 树干高度随机在 4 到 7 之间
        for (int y = baseHeight; y < baseHeight + treeHeight - 1 && y < worldHeight; ++y) {
            setBlock(x, y, z, BlockType::OAK_LOG); // 树干用类型 2 表示
        }
    }

    // 渲染地图
    void render(const glm::mat4& view, const glm::mat4& projection) {
        world_shader.use();

        // 设置视图和投影矩阵
        world_shader.setUniformMatrix4fv("view", glm::value_ptr(view));
        world_shader.setUniformMatrix4fv("projection", glm::value_ptr(projection));

        glBindVertexArray(VAO);

        // 使用三角形模式绘制
        glDrawArrays(GL_TRIANGLES, 0, 36*worldWidth*worldHeight*worldDepth); // 每个方块有6*6=36个顶点

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
                return true;
            }
        }

        return false;
    }

    void updateBlock(int x, int y, int z, BlockType type) {
        setBlock(x, y, z, type); // 设置为空地
        std::vector<float> blockVertices = getCubeVertices(x, y, z, type);

        int blockIndex = x * worldHeight * worldDepth + y * worldDepth + z; // 方块索引
        int offset = blockIndex * blockStride * sizeof(float); // 偏移字节数
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, offset, blockVertices.size() * sizeof(float), blockVertices.data());
        glBindVertexArray(0);
    }
};