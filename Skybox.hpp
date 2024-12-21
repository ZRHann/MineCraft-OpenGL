#pragma once
#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include "Shader.hpp"

class Skybox {
private:
    GLuint skyboxVAO, skyboxVBO;
    Shader skyboxShader;
    
    float dayTime = 0.0f;
    const float DAY_LENGTH = 300.0f;  // 5分钟一个游戏日
    
    glm::vec3 sunPosition;
    glm::vec3 moonPosition;

    // 天空盒顶点数据
    const float skyboxVertices[108] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

   
    GLuint programID; 
public:
    Skybox() {
        setupSkyboxMesh();
        loadShaders();
        updateCelestialBodies();
    }

    ~Skybox() {
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);
    }
    
    // 更新天空盒状态
    void update(float deltaTime) {
        dayTime += deltaTime / DAY_LENGTH * 24.0f;
        if(dayTime >= 24.0f) dayTime -= 24.0f;
        updateCelestialBodies();
    }
    
    // 渲染天空盒
    void render(const glm::mat4& view, const glm::mat4& projection) {
        glDepthFunc(GL_LEQUAL);
        
        skyboxShader.use();
        
        // 移除观察矩阵的平移部分，保持天空盒固定在相机位置
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        
        skyboxShader.setUniformMatrix4fv("view", glm::value_ptr(skyView));
        skyboxShader.setUniformMatrix4fv("projection", glm::value_ptr(projection));
        skyboxShader.setUniform1f("dayTime", dayTime);

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glDepthFunc(GL_LESS);
    }

    // 获取当前游戏时间（0-24小时）
    float getGameTime() const {
        return dayTime;
    }

private:
    // 设置天空盒网格
    void setupSkyboxMesh() {
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    // 加载着色器
    void loadShaders() {
        std::string vertPath = "shaders/skybox.vert";
        std::string fragPath = "shaders/skybox.frag";
        skyboxShader.createProgram(vertPath, fragPath);
    }

    // 更新天体位置
    void updateCelestialBodies() {
        float sunAngle = (dayTime / 24.0f) * 2.0f * M_PI;
        float radius = 90.0f;
        sunPosition = glm::vec3(
            cos(sunAngle) * radius,
            sin(sunAngle) * radius,
            0.0f
        );
        moonPosition = -sunPosition;
    }

};