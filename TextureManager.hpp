#define STB_IMAGE_IMPLEMENTATION
#include <unordered_map>
#include <string>
#include <glad.h>
#include <stb_image.h>
#include <iostream>
#include <vector>
enum TextureType {
    GRASS_BLOCK_TOP,
    GRASS_BLOCK_SIDE,
    GRASS_BLOCK_BOTTOM,
    TEXTURE_COUNT
};

class TextureManager {
public:
    GLuint textureArrayID;
    std::vector<std::string> texture_imgs = {
        "assets/grass_block_top.png",
        "assets/grass_block_side.png",
        "assets/grass_block_bottom.png"
    };

    TextureManager() : textureArrayID(0) {}

    ~TextureManager() {
        if (textureArrayID != 0) {
            glDeleteTextures(1, &textureArrayID);
        }
    }

    // 加载纹理数组
    bool loadTextureArray() {
        glGenTextures(1, &textureArrayID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

        int width = 0, height = 0, nrChannels = 0;

        // 预加载第一张纹理以确定尺寸
        unsigned char* data = stbi_load(texture_imgs[0].c_str(), &width, &height, &nrChannels, 4); // 强制加载为RGBA
        if (!data) {
            std::cerr << "Failed to load texture: " << texture_imgs[0] << std::endl;
            return false;
        }

        // 分配纹理数组内存
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, texture_imgs.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // 加载所有纹理
        for (size_t i = 0; i < texture_imgs.size(); ++i) {
            stbi_set_flip_vertically_on_load(true);
            data = stbi_load(texture_imgs[i].c_str(), &width, &height, &nrChannels, 4); // 强制加载为RGBA
            if (data) {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cerr << "ERROR: Failed to load texture: " << texture_imgs[i] << std::endl;
            }
        }

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return true;
    }


    // 获取纹理数组ID
    GLuint getTextureArrayID() const {
        return textureArrayID;
    }
};