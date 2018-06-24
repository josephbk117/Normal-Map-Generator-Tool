#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform vec3 _chromeColour;

void main()
{
    color = vec4(_chromeColour.rgb,1.0);
}