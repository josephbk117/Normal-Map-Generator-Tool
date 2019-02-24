#version 150
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in vec3 FragPos[];
in vec3 Normal[];
in mat3 TBN[];

out vec3 AttribColour;

uniform bool _ShowNormals;
const float MAGNITUDE = 0.25;

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);

	vec4 midPoint = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position)/3.0;
	vec3 avgNormal = normalize(cross(a, b)) * ((_ShowNormals)? 1.0 : 0.0);

	AttribColour = vec3(0,1,0);

	gl_Position = midPoint;
	EmitVertex();
	gl_Position = midPoint + vec4(avgNormal, 0.0) * MAGNITUDE;
	EmitVertex();
	EndPrimitive();

	/*AttribColour = vec3(0,1,0);
	gl_Position = midPoint;
	EmitVertex();
	gl_Position = midPoint + vec4(TBN[0][0],0.0) * MAGNITUDE;
	EmitVertex();
	EndPrimitive();*/

}