#version 330 core
layout (lines) in;             // 输入图元类型：线段
layout (triangle_strip, max_vertices = 4) out; // 输出图元类型：三角形带，最多 4 个顶点

uniform float lineWidth;        // 线宽

void main() {
    // 获取线段的两个端点（已经在顶点着色器中乘以了MVP矩阵）
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    // 计算线段的方向向量
    vec4 diff = p2 - p1;

    // 计算垂直于线段的向量（用于偏移，在裁剪空间进行）
    vec2 offset = normalize(vec2(-diff.y, diff.x)) * lineWidth * 0.5;

    // 生成四边形的四个顶点
    gl_Position = p1 + vec4(offset, 0.0, 0.0);
    EmitVertex(); // 发射第一个顶点

    gl_Position = p1 - vec4(offset, 0.0, 0.0);
    EmitVertex(); // 发射第二个顶点

    gl_Position = p2 + vec4(offset, 0.0, 0.0);
    EmitVertex(); // 发射第三个顶点

    gl_Position = p2 - vec4(offset, 0.0, 0.0);
    EmitVertex(); // 发射第四个顶点

    EndPrimitive(); // 结束当前图元
}