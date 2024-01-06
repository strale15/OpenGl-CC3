#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

out vec3 chFragPos;
out vec3 chNormal;
out vec2 chUV;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    chUV = inUV;
    chFragPos = vec3(uM * vec4(inPos, 1.0));
    chNormal = mat3(transpose(inverse(uM))) * inNormal;  
    
    gl_Position = uP * uV * vec4(chFragPos, 1.0);
}

