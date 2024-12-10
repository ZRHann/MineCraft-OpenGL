#version 460 core

in vec2 TexCoord;
flat in int TextureType;

out vec4 FragColor;

uniform sampler2DArray textureArray; // 使用纹理数组

void main() {
    if (TextureType >= 0) {
        FragColor = texture(textureArray, vec3(TexCoord, float(TextureType))); // 访问纹理数组
    } else {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Debug color for invalid textureType
    }
}
