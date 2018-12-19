#version 140
in vec2 vertexPosition;
in vec2 texCoords;
out vec2 textureUV;
out vec3 worldPos;
uniform mat4 model;
void main()
{
	gl_Position =  model * vec4(vertexPosition.xy,0,1.0);
	worldPos = gl_Position.xyz;
	gl_Position.z = 0;
	gl_Position.w = 1.0;
	textureUV= texCoords;
}