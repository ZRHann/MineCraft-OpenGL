#version 460 core

in vec2 TexCoord;          // 从顶点着色器传来的纹理坐标
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 设为红色
}