#version 140
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float Depth;

uniform float _CameraZoom;

void main()
{
	float valX = TexCoords.x * 1000.0;
	float valY = TexCoords.y * 1000.0;
	float val = 0.0;
	if(mod(valX, 10) >= 9 || mod(valY, 10) >= 9)
		val = 1.0;
	FragColor = vec4(val,val,val, (1.0 - (Depth * 0.02)) * val);
}