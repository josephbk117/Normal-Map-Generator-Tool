#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform sampler2D textureOne;

void main()
{
	float v = 0;
	if(distance(vec2(0.5,0.5), textureUV) > 0.45 && distance(vec2(0.5,0.5), textureUV) < 0.5)
		v = 1;
	color = mix(texture(textureOne, textureUV), vec4(1,1,1,0.5),v);
}