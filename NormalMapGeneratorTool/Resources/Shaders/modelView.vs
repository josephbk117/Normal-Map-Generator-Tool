#version 140
in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;
in vec3 aTangent;
in vec3 aBitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = (model * vec4(aPos, 1.0)).xyz;
    Normal =  (model * view * vec4(aNormal,0.0)).xyz; //mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    vec3 T = (model * vec4(aTangent,0.0)).xyz;
    vec3 N = (model * vec4(aNormal, 0.0)).xyz;
    T = normalize(T - dot(T, N) * N);
    vec3 B = (model * vec4(aBitangent, 0.0)).xyz;//cross(N, T);

    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}