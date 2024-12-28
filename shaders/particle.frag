#version 330 core
in float vTextureLayer;
out vec4 FragColor;

uniform sampler2DArray textureArray;

void main() {
    vec2 texCoord = gl_PointCoord;
    FragColor = texture(textureArray, vec3(texCoord, vTextureLayer));
}