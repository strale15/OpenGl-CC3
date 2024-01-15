#version 330 core

struct Material {
    sampler2D Kd;
    sampler2D Ks;
    float Shininess;
};

in vec3 vCol;
in vec2 TexCoord;
out vec4 FragColor;

uniform Material uMaterial;
uniform bool isColor;
uniform vec3 uColor;
uniform bool uTransp;
uniform float uAlpha;


void main() {
	vec3 MaterialColor = isColor ? uColor : vec3(texture(uMaterial.Kd, TexCoord));
	vec3 finalColor = vCol * MaterialColor;

	float alpha = 1.0f;
    if(uTransp) {
        alpha = uAlpha;
    }
	FragColor = vec4(finalColor, alpha);
}