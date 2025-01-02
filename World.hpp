#pragma once
#include "TextureManager.hpp"
#include "Shader.hpp"
#include "GeometryShader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite.h>
#include <vector>
#include "Block.hpp"
#include "ParticleSystem.hpp"
#include "DayTime.hpp"
#include "Wireframe.hpp"

// 32位随机数生成器
int rand32() {
    return rand() << 16 | rand();
}

class World {
public:
    const int vertexCountPerBlock = 24;
    const int attributesPerVertex = 6;
    const int blockStride = vertexCountPerBlock * attributesPerVertex;
    const int maxTreeHeight = 7; // 树木最大高度
    int worldWidth, worldHeight, worldDepth; // 地图的最大尺寸
    int worldSeed; // 地图种子
    ParticleSystem particleSystem; // 粒子系统
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组

    GLuint VAO, VBO, EBO;  
    Shader world_shader;    // 着色器

    Wireframe wireframe;

    std::vector<unsigned int> blockIndices = {
        2, 1, 0, 3, 2, 0,  // Front face
        4, 5, 6, 4, 6, 7,  // Back face
        10, 9, 8, 11, 10, 8,  // Left face
        12, 13, 14, 12, 14, 15,  // Right face
        18, 17, 16, 19, 18, 16,  // Top face
        20, 21, 22, 20, 22, 23   // Bottom face
    };

    World(int w, int h, int d) : worldWidth(w), worldHeight(h), worldDepth(d),particleSystem(textureManager) {
        map.resize(worldWidth, std::vector<std::vector<int>>(worldHeight, std::vector<int>(worldDepth, 0)));


        // 初始化着色器、纹理
        world_shader.createProgram("shaders/World.vert", "shaders/World.frag");
        textureManager.loadTextureArray();
        world_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager.getTextureArrayID());
        world_shader.setUniform1i("textureArray", 0);
        
        // 生成随机种子
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
                // 设置草方块
                for (int y = 0; y < worldHeight; ++y) {
                    if (y < terrainHeight) {
                        setBlock(x, y, z, BlockType::GRASS_BLOCK);  // 地面方块
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

    // 初始化VAO VBO EBO
    void setupBuffers() {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 遍历地图生成所有方块的顶点数据
        for (int x = 0; x < worldWidth; ++x) {
            for (int y = 0; y < worldHeight; ++y) {
                for (int z = 0; z < worldDepth; ++z) {
                    std::vector<float> cubeVertices = getCubeVertices(x, y, z, getBlock(x, y, z));
                    vertices.insert(vertices.end(), cubeVertices.begin(), cubeVertices.end());

                    // 每个方块需要添加的索引
                    int indexOffset = vertices.size() / attributesPerVertex - 24;

                    for (auto& index : blockIndices) {
                        indices.push_back(index + indexOffset);
                    }
                    
                }
            }
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 顶点缓冲区
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // 索引缓冲区
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // 设置顶点属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributesPerVertex * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, attributesPerVertex * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, attributesPerVertex * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    std::vector<float> getCubeVertices(float x, float y, float z, int blockType) {
        TextureType textureTypeTop;
        TextureType textureTypeSide; 
        TextureType textureTypeBottom;

        if (blockType == BlockType::BLOCK_AIR) { // 空气方块
            textureTypeTop = TextureType::TEXTURE_AIR;
            textureTypeSide = TextureType::TEXTURE_AIR;
            textureTypeBottom = TextureType::TEXTURE_AIR;
        }
        else if (blockType == BlockType::GRASS_BLOCK) { // 草方块
            textureTypeTop = TextureType::GRASS_BLOCK_TOP;
            textureTypeSide = TextureType::GRASS_BLOCK_SIDE;
            textureTypeBottom = TextureType::GRASS_BLOCK_BOTTOM;
        }
        else if (blockType == BlockType::OAK_LOG){ // 圆木方块
            textureTypeTop = TextureType::OAK_LOG_TOP;
            textureTypeSide = TextureType::OAK_LOG_SIDE;
            textureTypeBottom = TextureType::OAK_LOG_TOP;
        }
        else if (blockType == BlockType::OAK_LEAVES) { // 树叶方块
            textureTypeTop = TextureType::OAK_LOG_LEAVES;
            textureTypeSide = TextureType::OAK_LOG_LEAVES;
            textureTypeBottom = TextureType::OAK_LOG_LEAVES;
        }

        // 每个面包含4个顶点，每个顶点的数据包括：
        // 位置 (x, y, z)，纹理坐标 (u, v)，材质类型
        std::vector<float> cubeVertices = {
            // Front face
            x,     y,     z,     0.0f, 0.0f, float(textureTypeSide),
            x + 1, y,     z,     1.0f, 0.0f, float(textureTypeSide),
            x + 1, y + 1, z,     1.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z,     0.0f, 1.0f, float(textureTypeSide),

            // Back face
            x,     y,     z + 1, 0.0f, 0.0f, float(textureTypeSide),
            x + 1, y,     z + 1, 1.0f, 0.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z + 1, 0.0f, 1.0f, float(textureTypeSide),

            // Left face
            x,     y,     z,     0.0f, 0.0f, float(textureTypeSide),
            x,     y + 1, z,     0.0f, 1.0f, float(textureTypeSide),
            x,     y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x,     y,     z + 1, 1.0f, 0.0f, float(textureTypeSide),

            // Right face
            x + 1, y,     z,     0.0f, 0.0f, float(textureTypeSide),
            x + 1, y + 1, z,     0.0f, 1.0f, float(textureTypeSide),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeSide),
            x + 1, y,     z + 1, 1.0f, 0.0f, float(textureTypeSide),

            // Top face
            x,     y + 1, z,     0.0f, 0.0f, float(textureTypeTop),
            x + 1, y + 1, z,     1.0f, 0.0f, float(textureTypeTop),
            x + 1, y + 1, z + 1, 1.0f, 1.0f, float(textureTypeTop),
            x,     y + 1, z + 1, 0.0f, 1.0f, float(textureTypeTop),

            // Bottom face
            x,     y,     z,     0.0f, 0.0f, float(textureTypeBottom),
            x + 1, y,     z,     1.0f, 0.0f, float(textureTypeBottom),
            x + 1, y,     z + 1, 1.0f, 1.0f, float(textureTypeBottom),
            x,     y,     z + 1, 0.0f, 1.0f, float(textureTypeBottom),
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
        树的总高度: 5/6/7
        树干高度为树的总高度 - 1
    */
    void placeTree(int x, int baseHeight, int z) {
        int treeHeight = 5 + rand() % 3; // 树高度随机在 5 到 7 之间
        for (int y = baseHeight; y < baseHeight + treeHeight - 1 && y < worldHeight-1; ++y) {
            setBlock(x, y, z, BlockType::OAK_LOG); // 树干用类型 2 表示
        }

        if (treeHeight >=6){
            for (int y = baseHeight + treeHeight -1; y > baseHeight && y> baseHeight + treeHeight - 5 && y < worldHeight; y--){
                // 顶层树叶 
                if (y == baseHeight + treeHeight - 1){
                    for (int dx = x - 1; dx <= x + 1; dx++){
                        for (int dz = z - 1; dz <= z + 1; dz++){
                            if (dx == x || dz == z){
                                setBlock(dx, y, dz, BlockType::OAK_LEAVES);
                            }
                        }
                    }
                }
                // 2层
                else if (y == baseHeight + treeHeight - 2){
                    for (int dx = x - 1; dx <= x + 1; dx++){
                        for(int dz= z - 1; dz <= z + 1; dz++){
                            // 躲避树干
                            if (dx != x || dz != z){
                                setBlock(dx, y, dz, BlockType::OAK_LEAVES);
                            }
                        }
                    }
                }
                // 34层
                else if (y < baseHeight + treeHeight - 2){
                    for (int dx = x - 2; dx <= x + 2; dx++){
                        for(int dz= z - 2; dz <= z + 2; dz++){
                            if (dx != x || dz != z){
                                setBlock(dx, y, dz, BlockType::OAK_LEAVES);
                            }
                        }
                    }
                }
            }
        }
        else if (treeHeight == 5) { 
            for (int y = baseHeight + treeHeight -1; y > baseHeight && y> baseHeight + treeHeight - 4 && y < worldHeight; y--){
                // 顶层树叶 
                if (y == baseHeight + treeHeight - 1){
                    for (int dx = x - 1; dx <= x + 1; dx++){
                        for (int dz = z - 1; dz <= z + 1; dz++){
                            if (dx == x || dz == z){
                                setBlock(dx, y, dz, BlockType::OAK_LEAVES);
                            }
                        }
                    }
                }
                // 23层
                else if (y <= baseHeight + treeHeight - 2){
                    for (int dx = x - 2; dx <= x + 2; dx++){
                        for(int dz= z - 2; dz <= z + 2; dz++){
                            // 躲避树干
                            if (dx != x || dz != z){
                                setBlock(dx, y, dz, BlockType::OAK_LEAVES);
                            }
                        }
                    }
                }
            }
        }
    }

    // 渲染地图
    void render(const glm::mat4& view, const glm::mat4& projection) {
        world_shader.use();

        // 设置视图和投影矩阵
        world_shader.setUniformMatrix4fv("view", glm::value_ptr(view));
        world_shader.setUniformMatrix4fv("projection", glm::value_ptr(projection));
        world_shader.setUniform1f("dayNightBlendFactor", DayTime::getDayNightBlendFactor());

        glBindVertexArray(VAO);

        // 使用三角形模式绘制
        glDrawElements(GL_TRIANGLES, blockIndices.size() * worldWidth * worldHeight * worldDepth, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    // 检测选中的方块
    // blockHit: 返回选中的方块的位置
    bool detectSelectedBlock(const glm::vec3& playerPos, const glm::vec3& rayDir, glm::vec3& blockHit) {
        float maxDistance = 7.0f; // 最大检测距离
        float step = 0.1f;          // 每步的移动距离

        for (float distance = 0.0f; distance < maxDistance; distance += step) {
            glm::vec3 currentPos = playerPos + distance * rayDir;
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

    // 寻找放置方块的位置(选中的非空白方块 前的最后一个 空白方块)
    bool findLastAirBlock(const glm::vec3& playerPos, const glm::vec3& rayDir, glm::vec3& blockHit) {
        float maxDistance = 7.0f; // 最大检测距离
        float step = 0.1f;          // 每步的移动距离

        for (float distance = 0.0f; distance < maxDistance; distance += step) {
            glm::vec3 currentPos = playerPos + distance * rayDir;
            int x = static_cast<int>(currentPos.x);
            int y = static_cast<int>(currentPos.y);
            int z = static_cast<int>(currentPos.z);

            if (getBlock(x, y, z) > 0) {
                currentPos = currentPos - step * rayDir;
                x = static_cast<int>(currentPos.x);
                y = static_cast<int>(currentPos.y);
                z = static_cast<int>(currentPos.z);
                blockHit = glm::vec3(x, y, z);
                return true;
            }
        }
        return false;
    }

    void updateBlock(int x, int y, int z, BlockType type) {
        BlockType oldType = getBlock(x, y, z);
        if(oldType != BlockType::BLOCK_AIR) {
            // 销毁方块时产生粒子
            particleSystem.emit(glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f), oldType);
        }
        setBlock(x, y, z, type);
        std::vector<float> blockVertices = getCubeVertices(x, y, z, type);

        int blockIndex = x * worldHeight * worldDepth + y * worldDepth + z; // 方块索引
        int offset = blockIndex * blockStride * sizeof(float); // 偏移字节数
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferSubData(GL_ARRAY_BUFFER, offset, blockVertices.size() * sizeof(float), blockVertices.data());
        glBindVertexArray(0);
    }

    // 检测两个三维坐标形成的体积是否与方块碰撞
    bool isColliding(const glm::vec3& minBound, const glm::vec3& maxBound) {
        // 步进大小
        const float step = 0.3f;
        const float eps = 0.0001f; // 阈值，避免检测非常小的范围

        // 遍历检测碰撞范围内的点
        if (maxBound.x - minBound.x > eps) {
            for (float x = minBound.x; x < maxBound.x; x += step) {
                for (float y = minBound.y; y < maxBound.y; y += step) {
                    for (float z = minBound.z; z < maxBound.z; z += step) {
                        int blockX = static_cast<int>(std::floor(x));
                        int blockY = static_cast<int>(std::floor(y));
                        int blockZ = static_cast<int>(std::floor(z));

                        if (getBlock(blockX, blockY, blockZ) != BlockType::BLOCK_AIR) {
                            return true; // 如果有非空气方块，发生碰撞
                        }
                    }
                }
            }
        }

        // maxBound.y 检查顶部边缘
        if (maxBound.y - minBound.y > eps) {
            for (float x = minBound.x; x <= maxBound.x; x += step) {
                for (float z = minBound.z; z <= maxBound.z; z += step) {
                    if (getBlock(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(maxBound.y)), static_cast<int>(std::floor(z))) != BlockType::BLOCK_AIR) {
                        return true;
                    }
                }
            }
        }

        // maxBound.z 检查深度方向的边缘
        if (maxBound.z - minBound.z > eps) {
            for (float x = minBound.x; x <= maxBound.x; x += step) {
                for (float y = minBound.y; y <= maxBound.y; y += step) {
                    if (getBlock(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(maxBound.z))) != BlockType::BLOCK_AIR) {
                        return true;
                    }
                }
            }
        }

        // maxBound.x 检查宽度方向的边缘
        if (maxBound.x - minBound.x > eps) {
            for (float y = minBound.y; y <= maxBound.y; y += step) {
                for (float z = minBound.z; z <= maxBound.z; z += step) {
                    if (getBlock(static_cast<int>(std::floor(maxBound.x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(z))) != BlockType::BLOCK_AIR) {
                        return true;
                    }
                }
            }
        }

        return false; // 无碰撞
    }

    // 检测两个三维坐标形成的体积是否与目标方块碰撞
    bool isCollidingWith(const glm::vec3& minBound, const glm::vec3& maxBound, int targetBlockX, int targetBlockY, int targetBlockZ) {
        const float step = 0.3f;
        const float eps = 0.0001f; // 阈值，避免检测非常小的范围

        // 检查目标方块是否在碰撞范围内
        auto isTargetBlock = [&](int x, int y, int z) {
            return x == targetBlockX && y == targetBlockY && z == targetBlockZ;
        };

        // 遍历检测碰撞范围内的点
        if (maxBound.x - minBound.x > eps) {
            for (float x = minBound.x; x < maxBound.x; x += step) {
                for (float y = minBound.y; y < maxBound.y; y += step) {
                    for (float z = minBound.z; z < maxBound.z; z += step) {
                        int blockX = static_cast<int>(std::floor(x));
                        int blockY = static_cast<int>(std::floor(y));
                        int blockZ = static_cast<int>(std::floor(z));

                        if (isTargetBlock(blockX, blockY, blockZ)) {
                            return true; // 如果目标方块在范围内，发生碰撞
                        }
                    }
                }
            }
        }

        // maxBound.y 检查顶部边缘
        if (maxBound.y - minBound.y > eps) {
            for (float x = minBound.x; x <= maxBound.x; x += step) {
                for (float z = minBound.z; z <= maxBound.z; z += step) {
                    int blockX = static_cast<int>(std::floor(x));
                    int blockY = static_cast<int>(std::floor(maxBound.y));
                    int blockZ = static_cast<int>(std::floor(z));

                    if (isTargetBlock(blockX, blockY, blockZ)) {
                        return true;
                    }
                }
            }
        }

        // maxBound.z 检查深度方向的边缘
        if (maxBound.z - minBound.z > eps) {
            for (float x = minBound.x; x <= maxBound.x; x += step) {
                for (float y = minBound.y; y <= maxBound.y; y += step) {
                    int blockX = static_cast<int>(std::floor(x));
                    int blockY = static_cast<int>(std::floor(y));
                    int blockZ = static_cast<int>(std::floor(maxBound.z));

                    if (isTargetBlock(blockX, blockY, blockZ)) {
                        return true;
                    }
                }
            }
        }

        // maxBound.x 检查宽度方向的边缘
        if (maxBound.x - minBound.x > eps) {
            for (float y = minBound.y; y <= maxBound.y; y += step) {
                for (float z = minBound.z; z <= maxBound.z; z += step) {
                    int blockX = static_cast<int>(std::floor(maxBound.x));
                    int blockY = static_cast<int>(std::floor(y));
                    int blockZ = static_cast<int>(std::floor(z));

                    if (isTargetBlock(blockX, blockY, blockZ)) {
                        return true;
                    }
                }
            }
        }

        return false; // 无碰撞
    }
    
    void renderWireframe(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& blockPos) {
        wireframe.render(view, projection, blockPos);
    }
};