#version 330 core

uniform sampler2D uTex;
uniform bool isColor;
uniform vec3 uColor;
uniform float uA;
uniform bool uT;

in vec2 UV;

out vec4 FragColor;

void main() {
    vec4 MaterialColor = isColor ? vec4(uColor, 1.0f) : texture(uTex, UV);

    if(uT) {
        MaterialColor.w = uA;
    }
    FragColor = MaterialColor;
}
