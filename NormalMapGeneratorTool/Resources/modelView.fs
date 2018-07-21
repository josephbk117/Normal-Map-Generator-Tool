#version 140
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
uniform sampler2D inTexture;

void main()
{
    FragColor = texture(inTexture, TexCoords);
	//color.a = 1;
}