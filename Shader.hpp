#include <glad.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
class Shader {
public:
    GLuint programID;

    Shader() : programID(0) {}

    // 加载着色器文件
    std::string loadShaderSource(const std::string& filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        if (file.is_open()) {
            buffer << file.rdbuf();
            file.close();
        } else {
            std::cerr << "Error: Unable to open shader file " << filepath << std::endl;
        }
        return buffer.str();
    }

    // 编译着色器
    GLuint compileShader(const std::string& source, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        // 检查编译是否成功
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Error compiling shader: " << infoLog << std::endl;
        }
        return shader;
    }

    // 创建着色器程序
    void createProgram(const std::string& vertexPath, const std::string& fragmentPath) {
        // 加载顶点着色器和片段着色器的源代码
        std::string vertexSource = loadShaderSource(vertexPath);
        std::string fragmentSource = loadShaderSource(fragmentPath);

        // 编译顶点着色器和片段着色器
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

        // 创建着色器程序并链接着色器
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);

        // 检查程序是否链接成功
        GLint success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(programID, 512, nullptr, infoLog);
            std::cerr << "Error linking program: " << infoLog << std::endl;
        }

        // 删除着色器，因为它们已经被链接到程序中
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // 使用着色器程序
    void use() {
        glUseProgram(programID);
    }

    // 获取uniform变量的位置
    GLint getUniformLocation(const std::string& name) {
        return glGetUniformLocation(programID, name.c_str());
    }

    // 设置uniform变量
    void setUniform1i(const std::string& name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }

    void setUniform1f(const std::string& name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }

    void setUniform3fv(const std::string& name, const GLfloat* value) {
        glUniform3fv(getUniformLocation(name), 1, value);
    }

    void setUniformMatrix4fv(const std::string& name, const GLfloat* matrix) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
    }

    ~Shader() {
        if (programID) {
            glDeleteProgram(programID);
        }
    }
};