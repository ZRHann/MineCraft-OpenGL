#pragma once
#include "Shader.hpp"

class GeometryShader : public Shader {
public:
    GeometryShader() : Shader() {}

    // 创建着色器程序，包括几何着色器
    void createProgram(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
        // 加载顶点着色器、片段着色器和几何着色器的源代码
        std::string vertexSource = loadShaderSource(vertexPath);
        std::string fragmentSource = loadShaderSource(fragmentPath);
        std::string geometrySource = loadShaderSource(geometryPath); // 加载几何着色器源代码

        // 编译顶点着色器、片段着色器和几何着色器
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
        GLuint geometryShader = compileShader(geometrySource, GL_GEOMETRY_SHADER); // 编译几何着色器

        // 创建着色器程序并链接着色器
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glAttachShader(programID, geometryShader); // 附加几何着色器
        glLinkProgram(programID);

        // 检查程序是否链接成功
        GLint success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(programID, 512, nullptr, infoLog);
            std::cerr << "Error linking geometry program: " << infoLog << std::endl;
        }

        // 删除着色器，因为它们已经被链接到程序中
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(geometryShader); // 删除几何着色器
    }

    // 不使用几何着色器时，调用父类的createProgram
    void createProgram(const std::string& vertexPath, const std::string& fragmentPath){
        Shader::createProgram(vertexPath,fragmentPath);
    }
};