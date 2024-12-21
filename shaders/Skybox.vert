#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos * 100.0, 1.0); // 放大天空盒
    gl_Position = pos.xyww; // 确保天空盒始终在最远处
}