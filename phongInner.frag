#version 330 core
#define NR_POINT_LIGHTS 1

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
    vec3 Ks;
};

struct Material {
    sampler2D Kd;
    sampler2D Ks;
    float Shininess;
};

uniform PointLight uPointLights[NR_POINT_LIGHTS];

uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;
uniform bool isColor;
uniform vec3 uColor;
uniform bool uTransp;
uniform float uAlpha;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
    vec3 MaterialColor = isColor ? uColor : vec3(texture(uMaterial.Kd, UV));
    vec3 MaterialSpecualr = isColor ? vec3(0.4,0.4,0.4) : vec3(texture(uMaterial.Ks, UV));

    vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);
    vec3 DirLightVector = normalize(-uDirLight.Direction);
    float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
    vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
    float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);

    vec3 DirAmbientColor = uDirLight.Ka * MaterialColor;
    vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * MaterialSpecualr;
    vec3 DirColor = DirAmbientColor + DirSpecularColor;

    vec3 PtLightsColor = vec3(0);
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        vec3 PtLightVector = normalize(uPointLights[i].Position - vWorldSpaceFragment);
        float PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
        vec3 PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
        float PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

        vec3 PtAmbientColor = uPointLights[i].Ka * MaterialColor;
        vec3 PtDiffuseColor = PtDiffuse * uPointLights[i].Kd * MaterialColor;
        vec3 PtSpecularColor = PtSpecular * uPointLights[i].Ks * MaterialSpecualr;

        float PtLightDistance = length(uPointLights[i].Position - vWorldSpaceFragment);
        float PtAttenuation = 1.0f / (uPointLights[i].Kc + uPointLights[i].Kl * PtLightDistance + uPointLights[i].Kq * (PtLightDistance * PtLightDistance));
        PtLightsColor += PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);
    }

    vec3 FinalColor = DirColor + PtLightsColor;
    float alpha = 1.0f;
    if(uTransp) {
        alpha = uAlpha;
    }
    if(!isColor) {
        vec4 col = texture(uMaterial.Kd, UV);
        FragColor = vec4(FinalColor, col.w);
    } else {
        FragColor = vec4(FinalColor, alpha);
    }
}
