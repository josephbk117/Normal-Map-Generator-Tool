#version 150
out vec4 FragColor;

in vec3 AttribColour;

void main()
{
	FragColor = vec4(AttribColour.rgb, 1.0);
}