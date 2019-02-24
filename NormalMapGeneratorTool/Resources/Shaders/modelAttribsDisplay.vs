#version 150
in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;
in vec3 aTangent;
in vec3 aBitangent;

out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 _aPos = aPos + aNormal * 0.01;
    FragPos = (model * vec4(_aPos, 1.0)).xyz;
    Normal =  (model * view * vec4(aNormal,0.0)).xyz;

    vec3 T = (model * vec4(aTangent,0.0)).xyz;
    vec3 N = (model * vec4(aNormal, 0.0)).xyz;
    T = normalize(T - dot(T, N) * N);
    vec3 B = (model * vec4(aBitangent, 0.0)).xyz;

    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}