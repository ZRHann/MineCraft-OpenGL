#version 330 core
in float vTextureLayer;
out vec4 FragColor;

uniform sampler2DArray textureArray;
uniform float dayNightBlendFactor;

void main() {
    vec2 texCoord = gl_PointCoord;
    vec3 dayNightBlendColor = vec3(dayNightBlendFactor);
    vec4 color = texture(textureArray, vec3(texCoord, vTextureLayer));
    FragColor = vec4(color.rgb * dayNightBlendColor, color.a);
}