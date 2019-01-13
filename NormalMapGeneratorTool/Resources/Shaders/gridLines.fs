#version 140
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

uniform float _CameraZoom;

void main()
{
	float valX = TexCoords.x * 400.0;
	float valY = TexCoords.y * 400.0;
	float val = 0.0;
	if(mod(valX, 10) >= 9 || mod(valY, 10) >= 9)
		val = 1.0;
	FragColor = vec4(val,val,val,val);
}