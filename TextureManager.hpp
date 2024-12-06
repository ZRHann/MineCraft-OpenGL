#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_map>
#include <glad.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iostream>
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