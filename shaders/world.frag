#version 330 core

in vec2 TexCoord;
flat in int TextureType;

out vec4 FragColor;

uniform sampler2DArray textureArray; // 使用纹理数组

void main() {
    if (TextureType == 0) {
        discard; // 丢弃片元
    }
    FragColor = texture(textureArray, vec3(TexCoord, float(TextureType))); // 访问纹理数组

    // 支持透明
    if (FragColor.a < 0.01) {
        discard;
    }
}
