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
    const int vertexCountPerBlock = 36;
    const int attributesPerVertex = 6;
    const int blockStride = vertexCountPerBlock * attributesPerVertex;
    const int maxTreeHeight = 7; // 树木最大高度
    const int GPUBufferLimit = 1024; // GPU缓冲区大小限制 MB
    int worldWidth, worldHeight, worldDepth; // 地图的最大尺寸
    int worldSeed; // 地图种子
    ParticleSystem particleSystem; // 粒子系统
    TextureManager textureManager; // 纹理管理器
    std::vector<std::vector<std::vector<int>>> map; // 方块类型的3D数组
    std::vector<std::vector<std::vector<bool>>> visibleBlocks;

    GLuint VAO, VBO;  
    Shader world_shader;    // 着色器

    Wireframe wireframe;

    std::vector<int> blockVAOIndices;  // 方块在 VAO 中的位置
    std::vector<int> airIndices;      // 空气方块的 VAO 索引池
    int vaoSize = 0;                  // 当前 VAO 中的有效方块数量

    const int dirs[6][3] = {
        { 1,  0,  0},  // +x
        {-1,  0,  0},  // -x
        { 0,  1,  0},  // +y
        { 0, -1,  0},  // -y
        { 0,  0,  1},  // +z
        { 0,  0, -1},  // -z
    };

    World(int w, int h, int d) : worldWidth(w), worldHeight(h), worldDepth(d),particleSystem(textureManager) {
        map.resize(worldWidth, std::vector<std::vector<int>>(worldHeight, std::vector<int>(worldDepth, 0)));
        visibleBlocks.resize(worldWidth, std::vector<std::vector<bool>>(worldHeight, std::vector<bool>(worldDepth, false)));

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
        std::cout << "[INFO] World Seed: " << worldSeed << std::endl;

        blockVAOIndices.resize(worldWidth * worldHeight * worldDepth, -1); 
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

    void generateWorldMap() {
        FastNoiseLite terrainNoise;
        terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        terrainNoise.SetFrequency(0.03f);
        int terrainSeed = rand32();
        terrainNoise.SetSeed(terrainSeed);

        FastNoiseLite biomeNoise;
        biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        biomeNoise.SetFrequency(0.01f);
        biomeNoise.SetSeed(rand32());

        FastNoiseLite dirtNoise;
        dirtNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        dirtNoise.SetFrequency(0.15f);
        dirtNoise.SetSeed(rand32());

        // 初始化高度数组
        std::vector<std::vector<int>> heightMap(worldWidth, std::vector<int>(worldDepth, 0));

        // 第一步：生成原始地形高度
        for (int x = 0; x < worldWidth; ++x) {
            for (int z = 0; z < worldDepth; ++z) {
                float terrainValue = terrainNoise.GetNoise((float)x, (float)z);
                float normalizedTerrain = (terrainValue + 1.0f) / 2.0f;

                float biomeValue = biomeNoise.GetNoise((float)x, (float)z);
                bool isBasin = biomeValue > 0.2f;

                int terrainHeight = isBasin
                    ? (int)(normalizedTerrain * (worldHeight / 4 - 1)) + 1 // [1, worldHeight/4]
                    : (int)(normalizedTerrain * (worldHeight - 1)) + 1;    // [1, worldHeight]

                terrainHeight = std::max(terrainHeight, 1);
                heightMap[x][z] = terrainHeight;
            }
        }

        // 第二步：平滑地形高度
        const int maxDelta = 2; // 相邻高度的最大差值
        const int iterations = 4; // 平滑扫描次数

        for (int iter = 0; iter < iterations; ++iter) {
            std::vector<std::vector<int>> newHeightMap = heightMap; // 临时存储新的高度值

            for (int x = 1; x < worldWidth - 1; ++x) {
                for (int z = 1; z < worldDepth - 1; ++z) {
                    int& currentHeight = heightMap[x][z];

                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dz = -1; dz <= 1; ++dz) {
                            if (dx == 0 && dz == 0) continue;

                            int neighborHeight = heightMap[x + dx][z + dz];
                            if (currentHeight < neighborHeight - maxDelta) {
                                // 只调整较低点，提升到允许范围内
                                newHeightMap[x][z] = currentHeight + (neighborHeight - currentHeight) / 2;
                            }
                        }
                    }
                }
            }

            // 更新原始高度图
            heightMap = newHeightMap;
        }

        // 第三步：生成方块和树
        for (int x = 0; x < worldWidth; ++x) {
            for (int z = 0; z < worldDepth; ++z) {
                int terrainHeight = heightMap[x][z];
                float dirtValue = dirtNoise.GetNoise((float)x, (float)z);
                int dirtDepth = (int)((dirtValue + 1.0f) / 2.0f * terrainHeight / 2) + maxDelta; // [maxDelta, terrainHeight/2 + maxDelta]

                for (int y = 0; y < worldHeight; ++y) {
                    if (y < terrainHeight) {
                        if (y == terrainHeight - 1) {
                            setBlock(x, y, z, BlockType::GRASS_BLOCK);
                        } else if (y >= terrainHeight - dirtDepth) {
                            setBlock(x, y, z, BlockType::DIRT_BLOCK);
                        } else {
                            setBlock(x, y, z, BlockType::STONE_BLOCK);
                        }
                    }
                }

                float biomeValue = biomeNoise.GetNoise((float)x, (float)z);
                bool isBasin = biomeValue > 0.2f;
                if (!isBasin) {
                    const float treeDensity = 0.001f;
                    if (rand32() % 10000 < treeDensity * 10000) {
                        if (canPlaceTree(x, z, terrainHeight)) {
                            placeTree(x, terrainHeight, z);
                        }
                    }
                }
            }
        }

        setupBuffers();
    }


    void setupBuffers() {
        std::cout << "[INFO] Setting up buffers..." << std::endl;
        const int maxVertices = worldWidth * worldHeight * worldDepth * vertexCountPerBlock * attributesPerVertex;
        std::vector<float> vertices; // 用于存储可见方块的顶点数据（临时使用）

        // 遍历方块，生成可见方块的顶点数据
        for (int x = 0; x < worldWidth; ++x) {
            for (int y = 0; y < worldHeight; ++y) {
                for (int z = 0; z < worldDepth; ++z) {
                    if (isBlockVisible(x, y, z)) {
                        visibleBlocks[x][y][z] = true;

                        // 添加方块顶点数据
                        BlockType blockType = getBlock(x, y, z);
                        std::vector<float> cubeVertices = getCubeVertices(x, y, z, blockType);
                        vertices.insert(vertices.end(), cubeVertices.begin(), cubeVertices.end());

                        // 更新 VAO 索引
                        int blockIndex = x * worldHeight * worldDepth + y * worldDepth + z;
                        blockVAOIndices[blockIndex] = vaoSize++;
                    } else {
                        visibleBlocks[x][y][z] = false;
                    }
                }
            }
        }

        // 创建 VAO 和 VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        // 在 GPU 上分配缓冲区大小，但不从 CPU 上传所有数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, GPUBufferLimit * 1024 * 1024, nullptr, GL_STATIC_DRAW);
        
        // 如果有初始数据，仅上传当前已有的部分
        if (!vertices.empty()) {
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
        }
        std::cout << "[INFO] GPU Buffer Limit: " << GPUBufferLimit << " MB" << std::endl;
        std::cout << "[INFO] GPU Current Buffer size: " << vertices.size() * sizeof(float) / 1024.0 / 1024.0 << " MB" << std::endl;
        // 设置顶点属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        std::cout << "[INFO] Buffers set up successfully." << std::endl;
    }


    std::vector<float> getCubeVertices(float x, float y, float z, int blockType) {
        std::vector<float> cubeVertices(blockStride);

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
        }else if (blockType == BlockType::STONE_BLOCK) { // 石头方块
            textureTypeTop = TextureType::TEXTURE_STONE;
            textureTypeSide = TextureType::TEXTURE_STONE;
            textureTypeBottom = TextureType::TEXTURE_STONE;
        }else if (blockType == BlockType::DIRT_BLOCK) { // 泥土方块
            textureTypeTop = TextureType::TEXTURE_DIRT;
            textureTypeSide = TextureType::TEXTURE_DIRT;
            textureTypeBottom = TextureType::TEXTURE_DIRT;
        }else if (blockType == BlockType::SAND_BLOCK){ // 沙子方块
            textureTypeTop = TextureType::TEXTURE_SAND;
            textureTypeSide = TextureType::TEXTURE_SAND;
            textureTypeBottom = TextureType::TEXTURE_SAND;
        }else if (blockType == BlockType::GLASS_BLOCK) { // 玻璃方块
            textureTypeTop = TextureType::TEXTURE_GLASS;
            textureTypeSide = TextureType::TEXTURE_GLASS;
            textureTypeBottom = TextureType::TEXTURE_GLASS;
        }else if(blockType == BlockType::OAK_PLANKS) { // 橡木板方块
            textureTypeTop = TextureType::TEXTURE_OAK_PLANKS;
            textureTypeSide = TextureType::TEXTURE_OAK_PLANKS;
            textureTypeBottom = TextureType::TEXTURE_OAK_PLANKS;
        }else if(blockType == BlockType::STONE_BRICKS) { // 石砖方块
            textureTypeTop = TextureType::TEXTURE_STONE_BRICKS;
            textureTypeSide = TextureType::TEXTURE_STONE_BRICKS;
            textureTypeBottom = TextureType::TEXTURE_STONE_BRICKS;
        }

        // 每个面由两个三角形组成，总共36个顶点，每个顶点包含位置(0-2)、纹理坐标(3-4)和材质信息(5)
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

    bool canPlaceTree(int x, int z, int terrainHeight) {
        const int treeSpacing = 5; // 树木间隔

        // 如果树的顶部超出世界高度，返回 false
        if (terrainHeight + maxTreeHeight >= worldHeight) {
            return false;
        }

        // 遍历以 (x, z) 为中心的间隔范围，检查树木间隔
        for (int dx = -treeSpacing; dx <= treeSpacing; ++dx) {
            for (int dz = -treeSpacing; dz <= treeSpacing; ++dz) {
                if (x + dx >= 0 && x + dx < worldWidth && z + dz >= 0 && z + dz < worldDepth) {
                    // 检查区域内是否已经有树
                    for (int y = 0; y < worldHeight; ++y) {
                        BlockType blockType = getBlock(x + dx, y, z + dz);
                        if (blockType == BlockType::OAK_LOG || blockType == BlockType::OAK_LEAVES) {
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
        glDrawArrays(GL_TRIANGLES, 0, vaoSize * vertexCountPerBlock); // 每个方块有6*6=36个顶点

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

    void addBlock(int x, int y, int z, BlockType type) {
        setBlock(x, y, z, type);

        // 更新当前方块和周围邻居的可见性
        updateVisibility(x, y, z);
        for (auto& dir : dirs) {
            updateVisibility(x + dir[0], y + dir[1], z + dir[2]);
        }
    }

    void removeBlock(int x, int y, int z) {
        // 检查边界
        if (x < 0 || x >= worldWidth || y < 0 || y >= worldHeight || z < 0 || z >= worldDepth) {
            return;
        }

        // 获取当前方块类型
        BlockType currentType = getBlock(x, y, z);
        if (currentType == BlockType::BLOCK_AIR) {
            return; // 空气方块无需移除
        }

        // 生成粒子效果
        glm::vec3 blockCenter(x + 0.5f, y + 0.5f, z + 0.5f);
        particleSystem.emit(blockCenter, currentType);

        // 移除方块数据
        setBlock(x, y, z, BlockType::BLOCK_AIR);
        updateVisibility(x, y, z);
        for (auto& dir : dirs) {
            updateVisibility(x + dir[0], y + dir[1], z + dir[2]);
        }
    }


    void updateBlock(int x, int y, int z, BlockType type) {
        BlockType currentType = getBlock(x, y, z);
        if (currentType == type) return;
        if (currentType == BlockType::BLOCK_AIR) {
            // 空气变为其他方块
            addBlock(x, y, z, type);
        } else if (type == BlockType::BLOCK_AIR) {
            // 非空气变为空气
            removeBlock(x, y, z);
        } else {
            std::cerr << "ERROR: update but not add/remove NOT IMPLEMENTED" << std::endl;
        }
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

    void addBlockToGPU(int x, int y, int z) {
        int blockIndex = x * worldHeight * worldDepth + y * worldDepth + z;

        // 如果方块已存在于 GPU 中，则直接返回
        if (blockVAOIndices[blockIndex] != -1) return;

        // 获取当前方块的顶点数据
        std::vector<float> blockVertices = getCubeVertices(x, y, z, getBlock(x, y, z));

        int vaoIndex;
        if (!airIndices.empty()) {
            // 如果有可复用的 VAO 索引，从空气池中取出
            vaoIndex = airIndices.back();
            airIndices.pop_back();

            // 更新 VBO 数据
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, vaoIndex * blockStride * sizeof(float), blockVertices.size() * sizeof(float), blockVertices.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else {
            // 如果没有可复用的 VAO 索引，则追加到当前 VAO 末尾
            vaoIndex = vaoSize++;

            // 更新 VBO 数据
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, vaoIndex * blockStride * sizeof(float), blockVertices.size() * sizeof(float), blockVertices.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // 更新 VAO 索引映射
        blockVAOIndices[blockIndex] = vaoIndex;
    }


    void removeBlockFromGPU(int x, int y, int z) {
        int blockIndex = x * worldHeight * worldDepth + y * worldDepth + z;
        int vaoIndex = blockVAOIndices[blockIndex];

        if (vaoIndex == -1) return; // 不在 GPU 中，无需移除

        // 更新 VAO 数据为空气方块
        std::vector<float> airVertices = getCubeVertices(x, y, z, BlockType::BLOCK_AIR);
        int offset = vaoIndex * blockStride * sizeof(float);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, offset, airVertices.size() * sizeof(float), airVertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 回收 VAO 索引并更新映射
        airIndices.push_back(vaoIndex);
        blockVAOIndices[blockIndex] = -1;
    }


    // 方块是否未被遮挡
    bool isBlockVisible(int x, int y, int z) {
        BlockType type = getBlock(x, y, z);
        // 空气直接不可见
        if (type == BlockType::BLOCK_AIR) {
            return false;
        }

        // 检查 6 个方向
        // 如果越界、或者空气、或者是透明方块，就认为此方向是“敞开”的
        // 只要有 1 个方向敞开，就可见
        

        for (auto &dir : dirs) {
            int nx = x + dir[0];
            int ny = y + dir[1];
            int nz = z + dir[2];
            // 越界算可见
            if (nx < 0 || nx >= worldWidth  ||
                ny < 0 || ny >= worldHeight ||
                nz < 0 || nz >= worldDepth) {
                return true;
            }

            BlockType neighborType = getBlock(nx, ny, nz);
            if (isTransparent(neighborType)) {
                return true;
            }
        }

        // 如果六面都被不透明方块包住，则不可见
        return false;
    }

    void updateVisibility(int x, int y, int z) {
        // 越界检查
        if (x < 0 || x >= worldWidth || y < 0 || y >= worldHeight || z < 0 || z >= worldDepth) {
            return;
        }
        // 更新当前方块的可见性
        bool visible = isBlockVisible(x, y, z);

        if (visible && !visibleBlocks[x][y][z]) {
            // 方块从不可见变为可见，加入 GPU
            addBlockToGPU(x, y, z);
            visibleBlocks[x][y][z] = true;
        } else if (!visible && visibleBlocks[x][y][z]) {
            // 方块从可见变为不可见，从 GPU 移除
            removeBlockFromGPU(x, y, z);
            visibleBlocks[x][y][z] = false;
        }
    }
    
    void renderWireframe(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& blockPos) {
        wireframe.render(view, projection, blockPos);
    }
};