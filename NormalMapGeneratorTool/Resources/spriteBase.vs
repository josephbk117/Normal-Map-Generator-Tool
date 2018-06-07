#version 140
in vec2 vertexPosition;
in vec2 texCoords;
out vec2 textureUV;
void main()
{
	gl_Position.xyz =  vec3(vertexPosition.x, vertexPosition.y,0.0);
	gl_Position.w = 1.0;
	textureUV= texCoords;
}