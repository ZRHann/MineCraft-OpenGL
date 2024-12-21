#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform float dayTime;

// 随机数函数
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// 噪声函数
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

// FBM函数
float fbm(vec2 st) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;

    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(st * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

// 星星闪烁效果
float starTwinkle(vec2 st, float time) {
    return random(st) * 0.5 * (sin(time * 2.0) * 0.5 + 0.5);
}

// 生成星星
float stars(vec3 position) {
    vec2 st = vec2(
        atan(position.z, position.x),
        acos(position.y / length(position))
    );
    
    st *= 25.0; // 增加星星密度
    
    vec2 ipos = floor(st);
    float star = 0.0;
    
    float rand = random(ipos);
    if(rand > 0.98) { // 控制星星数量
        vec2 fpos = fract(st);
        float dist = length(fpos - 0.5);
        star = 1.0 - smoothstep(0.0, 0.03, dist); // 调整星星大小
        
        // 添加闪烁效果
        star *= (0.8 + 0.2 * starTwinkle(ipos, dayTime * 5.0));
    }
    
    return star;
}

void main() {
    // 基础天空颜色
    vec3 dayColor = vec3(0.5, 0.7, 1.0);
    vec3 nightColor = vec3(0.0, 0.0, 0.1);
    vec3 sunriseColor = vec3(0.8, 0.6, 0.4);

    float t = (dayTime - 6.0) / 12.0;

    vec3 skyColor;
    if (t < 0.0) {
        skyColor = mix(nightColor, sunriseColor, t + 0.5);
    } else if (t < 1.0) {
        skyColor = mix(sunriseColor, dayColor, t);
    } else {
        skyColor = mix(dayColor, nightColor, t - 1.0);
    }

    // 根据高度增加渐变
    float height = TexCoords.y * 0.5 + 0.5;
    skyColor = mix(skyColor * 0.8, skyColor, height);

    // 添加星星
    float starIntensity = 0.0;
    if(t < 0.0 || t > 1.0) { // 夜晚显示星星
        float starVisibility = 1.0;
        if(t < 0.0) starVisibility = 1.0 - (t + 0.5) * 2.0; // 日出时淡出
        if(t > 1.0) starVisibility = (t - 1.0) * 2.0;       // 日落时淡入
        
        starIntensity = stars(normalize(TexCoords)) * starVisibility;
        // 在有云的地方减弱星星亮度
        skyColor += vec3(0.9, 0.95, 1.0) * starIntensity;
    }

    // 云层生成
    vec2 cloudUV = TexCoords.xz;

    // 改进云的运动和扰动
    float timeOffset = dayTime * 0.05;
    vec2 cloudOffset1 = vec2(noise(vec2(timeOffset * 0.5, 0.0)), noise(vec2(0.0, timeOffset * 0.5))) * 0.5;
    vec2 cloudOffset2 = vec2(noise(vec2(timeOffset * 0.25 + 10.0, 0.0)), noise(vec2(0.0, timeOffset * 0.25 + 10.0))) * 0.25;
    cloudUV *= 2.0;
    cloudUV += cloudOffset1 + cloudOffset2;
    cloudUV += vec2(timeOffset * 0.1, 0.0);

    float cloudNoise = fbm(cloudUV);

    // 改进云的密度和形状
    float cloudDensity = smoothstep(0.1, 0.9, cloudNoise);
    cloudDensity *= smoothstep(-0.4, 0.4, TexCoords.y);
    cloudDensity *= (1.0 - noise(cloudUV * 5.0) * 0.3);

    // 模拟云层遮挡
    float occlusion = fbm(cloudUV * 4.0) * 0.4;
    cloudDensity -= occlusion;
    cloudDensity = max(0.0, cloudDensity);

    // 云层颜色
    vec3 cloudColor = vec3(1.0);
    float cloudBrightness = 1.0;
    if(t < 0.0 || t > 1.0) {
        cloudColor = vec3(0.2, 0.2, 0.3);
        cloudBrightness = 0.5;
    }
    cloudColor *= cloudBrightness;

    // 最终颜色混合
    cloudDensity = pow(cloudDensity, 1.5);
    vec3 finalColor = mix(skyColor, cloudColor, cloudDensity);

    FragColor = vec4(finalColor, 1.0);
}