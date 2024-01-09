#version 330 core

uniform sampler2D uTex;
uniform bool isColor;
uniform vec3 uColor;

in vec2 UV;

out vec4 FragColor;

void main() {
    vec3 MaterialColor = isColor ? uColor : vec3(texture(uTex, UV));

    FragColor = vec4(MaterialColor, 1.0f);
    //FragColor = vec4(1,0,0,1);
}
