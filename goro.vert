#version 330 core
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

struct PositionalLight {
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

uniform PositionalLight uPointLights[NR_POINT_LIGHTS];
uniform DirectionalLight uSpotlights[NR_SPOT_LIGHTS];
uniform DirectionalLight uDirLight;
uniform vec3 uViewPos;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out vec3 vCol;
out vec2 TexCoord;

void main() {
	TexCoord = aUV;

	vec3 WorldSpaceVertex = vec3(uModel * vec4(aPos, 1.0f));
	vec3 WorldSpaceNormal = normalize(mat3(transpose(inverse(uModel))) * aNormal);
	vec3 ViewDirection = normalize(uViewPos - WorldSpaceVertex);

	vec3 DirLightVector = normalize(-uDirLight.Direction);
	float DirDiffuse = max(dot(WorldSpaceNormal, DirLightVector), 0.0f);
	vec3 DirReflectDirection = reflect(-DirLightVector, WorldSpaceNormal);
	float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), 32.0f);

	vec3 DirAmbientColor = uDirLight.Ka;
	vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse;
	vec3 DirSpecularColor = uDirLight.Ks * DirSpecular;
	vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

	vec3 PtColor;
	bool first = true;
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		vec3 PtLightVector = normalize(uPointLights[i].Position - WorldSpaceVertex);
		float PtDiffuse = max(dot(WorldSpaceNormal, PtLightVector), 0.0f);
		vec3 PtReflectDirection = reflect(-PtLightVector, WorldSpaceNormal);
		float PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), 32.0f);

		vec3 PtAmbientColor = uPointLights[i].Ka;
		vec3 PtDiffuseColor = PtDiffuse * uPointLights[i].Kd;
		vec3 PtSpecularColor = PtSpecular * uPointLights[i].Ks;

		float PtLightDistance = length(uPointLights[i].Position - WorldSpaceVertex);
		float PtAttenuation = 1.0f / (uPointLights[i].Kc + uPointLights[i].Kl * PtLightDistance + uPointLights[i].Kq * (PtLightDistance * PtLightDistance));
		if(first) {
			PtColor = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);
			first = false;
		} else {
			PtColor += PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);
		}
	}
	

	vec3 SpotColor;
	first = true;
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
		vec3 SpotlightVector = normalize(uSpotlights[i].Position - WorldSpaceVertex);

		float SpotDiffuse = max(dot(WorldSpaceNormal, SpotlightVector), 0.0f);
		vec3 SpotReflectDirection = reflect(-SpotlightVector, WorldSpaceNormal);
		float SpotSpecular = pow(max(dot(ViewDirection, SpotReflectDirection), 0.0f), 32.0f);

		vec3 SpotAmbientColor = uSpotlights[i].Ka;
		vec3 SpotDiffuseColor = SpotDiffuse * uSpotlights[i].Kd;
		vec3 SpotSpecularColor = SpotSpecular * uSpotlights[i].Ks;

		float SpotlightDistance = length(uSpotlights[i].Position - WorldSpaceVertex);
		float SpotAttenuation = 1.0f / (uSpotlights[i].Kc + uSpotlights[i].Kl * SpotlightDistance + uSpotlights[i].Kq * (SpotlightDistance * SpotlightDistance));

		float Theta = dot(SpotlightVector, normalize(-uSpotlights[i].Direction));
		float Epsilon = uSpotlights[i].InnerCutOff - uSpotlights[i].OuterCutOff;
		float SpotIntensity = clamp((Theta - uSpotlights[i].OuterCutOff) / Epsilon, 0.0f, 1.0f);
		if(first) {
			SpotColor = SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
			first = false;
		} else {
			SpotColor += SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
		}
	}

	vCol = DirColor + PtColor + SpotColor;
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0f);
}