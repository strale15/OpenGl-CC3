#version 330 core
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1

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
    float InnerCutOff;
    float OuterCutOff;
    float Kc;
    float Kl;
    float Kq;
};

struct Material {
    sampler2D Kd;
    sampler2D Ks;
    float Shininess;
};

uniform PointLight uPointLights[NR_POINT_LIGHTS];
uniform DirectionalLight uSpotlights[NR_SPOT_LIGHTS];

uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;
uniform bool isColor;
uniform vec3 uColor;
uniform bool uTransp;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
    vec3 MaterialColor = isColor ? uColor : vec3(texture(uMaterial.Kd, UV));
    vec3 MaterialSpecualr = isColor ? vec3(0.6,0.6,0.6) : vec3(texture(uMaterial.Ks, UV));

    vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);
    vec3 DirLightVector = normalize(-uDirLight.Direction);
    float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
    vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
    float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);

    vec3 DirAmbientColor = uDirLight.Ka * MaterialColor;
    vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse * MaterialColor;
    vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * MaterialSpecualr;
    vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

    vec3 PtLightsColor;
    bool first = true;
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

        if (first) {
            PtLightsColor = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);
            first = false;
        } else {
            PtLightsColor += PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);
        }
    }

    vec3 SptLightsColor;
    first = true;
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        vec3 SpotlightVector = normalize(uSpotlights[i].Position - vWorldSpaceFragment);

        float SpotDiffuse = max(dot(vWorldSpaceNormal, SpotlightVector), 0.0f);
        vec3 SpotReflectDirection = reflect(-SpotlightVector, vWorldSpaceNormal);
        float SpotSpecular = pow(max(dot(ViewDirection, SpotReflectDirection), 0.0f), uMaterial.Shininess);

        vec3 SpotAmbientColor = uSpotlights[i].Ka * MaterialColor;
        vec3 SpotDiffuseColor = SpotDiffuse * uSpotlights[i].Kd * MaterialColor;
        vec3 SpotSpecularColor = SpotSpecular * uSpotlights[i].Ks * MaterialSpecualr;

        float SpotlightDistance = length(uSpotlights[i].Position - vWorldSpaceFragment);
        float SpotAttenuation = 1.0f / (uSpotlights[i].Kc + uSpotlights[i].Kl * SpotlightDistance + uSpotlights[i].Kq * (SpotlightDistance * SpotlightDistance));

        float Theta = dot(SpotlightVector, normalize(-uSpotlights[i].Direction));
        float Epsilon = uSpotlights[i].InnerCutOff - uSpotlights[i].OuterCutOff;
        float SpotIntensity = clamp((Theta - uSpotlights[i].OuterCutOff) / Epsilon, 0.0f, 1.0f);
        vec3 SpotColor = SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);

        if (first) {
            SptLightsColor = SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
            first = false;
        } else {
            SptLightsColor += SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
        }
    }

    vec3 FinalColor = DirColor + PtLightsColor + SptLightsColor;
    float alpha = 1.0f;
    if(uTransp) {
        alpha = 0.4f;
    }
    FragColor = vec4(FinalColor, alpha);
}
