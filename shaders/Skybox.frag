#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform float dayTime;

void main() {
    // 计算天空颜色
    vec3 dayColor = vec3(0.5, 0.7, 1.0);
    vec3 nightColor = vec3(0.0, 0.0, 0.1);
    vec3 sunriseColor = vec3(0.8, 0.6, 0.4);
    
    float t = (dayTime - 6.0) / 12.0; // 将时间映射到[-0.5, 1.5]范围
    
    vec3 skyColor;
    if(t < 0.0) { // 夜晚
        skyColor = mix(nightColor, sunriseColor, t + 0.5);
    } else if(t < 1.0) { // 白天
        skyColor = mix(sunriseColor, dayColor, t);
    } else { // 傍晚
        skyColor = mix(dayColor, nightColor, t - 1.0);
    }
    
    // 根据高度增加渐变效果
    float height = TexCoords.y * 0.5 + 0.5;
    skyColor = mix(skyColor * 0.8, skyColor, height);
    
    FragColor = vec4(skyColor, 1.0);
}