#version 330 core

struct Material {
    sampler2D Kd;
};

uniform Material uMaterial;
uniform bool isColor;
uniform vec3 uColor;
uniform bool uTransp;
uniform float uAlpha;

in vec2 UV;

out vec4 FragColor;

void main() {
    vec3 MaterialColor = isColor ? uColor : vec3(texture(uMaterial.Kd, UV));
    float alpha = 1.0f;
    if(uTransp) {
        alpha = uAlpha;
    }
    FragColor = vec4(MaterialColor, alpha);
}
