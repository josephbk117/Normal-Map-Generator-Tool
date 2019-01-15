#version 140
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float Depth;

uniform float _CameraZoom;

void main()
{
	float valX = TexCoords.x * 1500.0;
	float valY = TexCoords.y * 1500.0;
	float val = pow(sin(smoothstep(8.5, 10.0, mod(valX, 10)) * 3.1415926), 8);
	val += pow(sin(smoothstep(8.5, 10.0, mod(valY, 10)) * 3.1415926), 8);
	FragColor = vec4(val,val,val, (1.0 - (Depth * 0.02)) * val);
}