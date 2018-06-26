#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform sampler2D textureOne;
uniform vec3 _chromeColour;

void main()
{
	vec4 texCol = texture(textureOne, textureUV);
    color = texCol + vec4(_chromeColour.rgb, 1.0);
}