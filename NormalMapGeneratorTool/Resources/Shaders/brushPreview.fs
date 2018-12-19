#version 140
in vec2 textureUV;
out vec4 color;

uniform float _BrushOffset;
uniform float _BrushStrength;
uniform vec3 _BrushColour;

void main()
{
	float dist = distance(vec2(0.5, 0.5), textureUV);
	float t = 1.0 - clamp( dist * 2, 0.0, 1.0);
	t = clamp(t * _BrushOffset, 0.0, 1.0);
	vec3 col2 = vec3(1,0,1);//*step(t, 0.6) * step(1.0 - t, 0.5)
	color = vec4(_BrushColour, t * _BrushStrength);
	//color.r = step(dist,0.5) - step(dist,0.45);
	//color.a *= step(dist,0.5) - step(dist,0.45);
	color = mix(color, vec4(1.0 - color.rgb, 0.2), step(dist,0.5) - step(dist,0.45));
}