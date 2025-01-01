#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;
uniform float dayNightBlendFactor;

void main() 
{
    vec3 dayNightBlendColor = vec3(dayNightBlendFactor);
    // 取样天空盒纹理
    vec4 color = texture(skybox, TexCoords);

    // 将当前时间段的颜色乘到纹理上
    FragColor = vec4(color.rgb * dayNightBlendColor, color.a);
}
