#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "Shader.hpp"
#include "Block.hpp"
#include "TextureManager.hpp"
#include "DayTime.hpp"

class ParticleSystem {
private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float life;
        float size;
        int textureLayer; // 新增:用于存储纹理数组层索引
    };

    std::vector<Particle> particles;
    GLuint VAO, VBO;
    Shader particleShader;
    TextureManager& textureManager; // 引用纹理管理器

public:
    ParticleSystem(TextureManager& tm,int maxParticles = 1000) : textureManager(tm) {
        particles.reserve(maxParticles);
        setupParticleBuffers();
        particleShader.createProgram("shaders/particle.vert", "shaders/particle.frag");
    }

    void emit(const glm::vec3& position, BlockType blockType);
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void setupParticleBuffers();
};

// Add helper function to map BlockType to texture layer


void ParticleSystem::emit(const glm::vec3& position, BlockType blockType) {
    
    for(int i = 0; i < 10; i++) { // 每次产生10个粒子
        Particle p;
        p.position = position;
        
        // 随机速度
        float randomAngle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
        float randomSpeed = static_cast<float>(rand()) / RAND_MAX * 2.0f + 1.0f;
        p.velocity = glm::vec3(
            cos(randomAngle) * randomSpeed,
            static_cast<float>(rand()) / RAND_MAX * 2.0f + 2.0f, // 向上的速度
            sin(randomAngle) * randomSpeed
        );
        
        p.life = 1.0f;
        p.size = 80.0f;
        p.textureLayer = getTextureLayer(blockType);
        particles.push_back(p);
    }
}

void ParticleSystem::update(float deltaTime) {
    for(auto it = particles.begin(); it != particles.end();) {
        it->position += it->velocity * deltaTime;
        it->velocity.y -= 9.81f * deltaTime; // 重力
        it->life -= deltaTime;
        it->size -= deltaTime * 2.0f;
        
        if(it->life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::setupParticleBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Position (3) + Size (1) + TextureLayer (1) = 5 floats per vertex
    glBufferData(GL_ARRAY_BUFFER, 1000 * 5 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Size attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture layer attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    if (particles.empty()) return;
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    particleShader.use();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager.getTextureArrayID());
    particleShader.setUniform1i("textureArray", 0);
    particleShader.setUniformMatrix4fv("view", glm::value_ptr(view));
    particleShader.setUniformMatrix4fv("projection", glm::value_ptr(projection));
    particleShader.setUniform1f("dayNightBlendFactor", DayTime::getDayNightBlendFactor());
    
    std::vector<float> particleData;
    for (const auto& particle : particles) {
        if(particle.life > 0.0f) {
            // Position
            particleData.push_back(particle.position.x);
            particleData.push_back(particle.position.y);
            particleData.push_back(particle.position.z);
            // Size
            particleData.push_back(particle.size);
            // Texture layer
            particleData.push_back(static_cast<float>(particle.textureLayer));
        }
    }
    
    if(!particleData.empty()) {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particleData.size() * sizeof(float), particleData.data());
        glDrawArrays(GL_POINTS, 0, particleData.size() / 5);
        glBindVertexArray(0);
    }
    
    glDisable(GL_BLEND);
    glDisable(GL_PROGRAM_POINT_SIZE);
}