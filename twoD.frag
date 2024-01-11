#version 330 core

uniform sampler2D uTex;
uniform bool isColor;
uniform vec3 uColor;
uniform bool isTransp;

in vec2 UV;

out vec4 FragColor;

void main() {
    vec4 MaterialColor = isColor ? vec4(uColor, 1.0f) : texture(uTex, UV);

    if(isTransp) {
        MaterialColor.w = 0.3; 
    }
    FragColor = MaterialColor;
}
