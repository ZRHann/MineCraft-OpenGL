#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#define PI 3.14159265358979323846

namespace DayTime {
    static float currentTime = 0.0f;   // 当前时间
    const float dayLength = 240.0f;     // 一天的长度(s)

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
}
