#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;
uniform float dayTime;
uniform float dayLength;

void main() 
{
    // 计算一个从 0～24 的周期映射到 0～1 的时间比
    float timeRatio = dayTime / dayLength;  

    // 定义不同时间段的颜色
    vec3 morningColor = vec3(0.8, 0.5, 0.3);   // 清晨：橙色调
    vec3 dayColor = vec3(1.0, 1.0, 1.0);       // 白天：明亮的白色
    vec3 eveningColor = vec3(0.8, 0.3, 0.1);   // 傍晚：橙红色调
    vec3 nightColor = vec3(0.1, 0.1, 0.1);     // 夜晚：黑色

    // 根据时间比率确定当前的颜色
    vec3 currentColor;
    if (timeRatio < 0.125) 
    {
        // 0:00 - 3:00，夜晚到清晨渐变
        float t = timeRatio / 0.125;
        currentColor = mix(nightColor, morningColor, t);
    }
    else if (timeRatio < 0.375) 
    {
        // 3:00 - 9:00，清晨到白天渐变
        float t = (timeRatio - 0.125) / 0.25;
        currentColor = mix(morningColor, dayColor, t);
    }
    else if (timeRatio < 0.625) 
    {
        // 9:00 - 15:00，白天到傍晚渐变
        float t = (timeRatio - 0.375) / 0.25;
        currentColor = mix(dayColor, eveningColor, t);
    }
    else if (timeRatio < 0.75) 
    {
        // 15:00 - 18:00，傍晚到夜晚渐变
        float t = (timeRatio - 0.625) / 0.125;
        currentColor = mix(eveningColor, nightColor, t);
    }
    else 
    {
        // 18:00 - 24:00，夜晚持续
        currentColor = nightColor;
    }

    // 取样天空盒纹理
    vec4 color = texture(skybox, TexCoords);

    // 将当前时间段的颜色乘到纹理上
    FragColor = vec4(color.rgb * currentColor, color.a);
}
