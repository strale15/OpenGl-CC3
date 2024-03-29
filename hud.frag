#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main() {
    vec4 sampledColor = texture(textureSampler, TexCoord);
    
    if (sampledColor.x <= 0.1 && sampledColor.y <= 0.1 && sampledColor.y <= 0.1) {
        discard;
    }

    sampledColor.w = 0.5;
    FragColor = sampledColor;
}
