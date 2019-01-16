#version 140
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float Depth;

uniform float _CameraZoom;

void main()
{	
	vec2 coord = TexCoords * 100;
	vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord * 10);
	float line = 1.0 - min(grid.x, grid.y);
	FragColor = vec4(line, line, line, (1.0 - (Depth * 0.02)) * line);
}