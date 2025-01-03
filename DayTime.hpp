#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#define PI 3.14159265358979323846

namespace DayTime {
    const float dayLength = 200.0f;     // 一天的长度(s)
    static float currentTime = dayLength / 2;   // 当前时间
    // 更新时间
    void update(float deltaTime) {
        currentTime += deltaTime;
        if (currentTime > dayLength) {
            currentTime -= dayLength;
        }
    }

    // 获取当前时间
    float getCurrentTime() {
        return currentTime;
    }

    float getDayLength() {
        return dayLength;
    }

    float getDayNightBlendFactor() {
        // 将当前时间映射到 0~1 的周期
        float timeRatio = currentTime / dayLength;

        // 基于正弦函数的平滑过渡，调整输出到 [0.1, 1.0]
        return 0.1f + 0.9f * (0.5f * (1.0f + sin(2.0f * PI * timeRatio - PI / 2.0f)));
    }
}
