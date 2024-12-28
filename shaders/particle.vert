#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;
layout (location = 2) in float aTextureLayer;

out float vTextureLayer;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vTextureLayer = aTextureLayer;
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aSize;
}