#version 330 core

#define NR_POINT_LIGHTS 0
#define NR_SPOT_LIGHTS 0

struct PointLight {
    vec3 Position;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Kc;
    float Kl;
    float Kq;
};

struct DirectionalLight {
    vec3 Position;
    vec3 Direction;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
};

struct SpotLight {
    vec3 Position;
    vec3 Direction;
    float InnerCutOff;
    float OuterCutOff;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Kc;
    float Kl;
    float Kq;
};

struct Material {
    sampler2D Kd;
    sampler2D Ks;
    float Shininess;
};

uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
    vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);

    // Directional light
    vec3 DirLightVector = normalize(-uDirLight.Direction);
    float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
    vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
    float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);

    vec3 DirAmbientColor = uDirLight.Ka * vec3(texture(uMaterial.Kd, UV));
    vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse * vec3(texture(uMaterial.Kd, UV));
    vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * vec3(texture(uMaterial.Ks, UV));
    vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

    vec3 FinalColor = DirColor;
    FragColor = vec4(FinalColor, 1.0f);
}
