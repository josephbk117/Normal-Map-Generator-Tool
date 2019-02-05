#version 140
in vec2 textureUV;
out vec4 color;

uniform sampler2D _BrushTexture;
uniform float _BrushOffset;
uniform float _BrushStrength;
uniform vec3 _BrushColour;
uniform int _UseTexture;

void main()
{
	if(_UseTexture == 0)
	{
		float val = texture(_BrushTexture, textureUV).r;

		float rim = step(sin(textureUV.x * 3.141), 0.15);
		rim += step(sin(textureUV.y * 3.141), 0.15);

		if(rim > 0)
		{
			color = vec4(1.0 -_BrushColour,0.4);
		}
		else
		{
			val = pow(val, _BrushOffset * 0.1);
			color = vec4(val * _BrushColour, (rim > 0 ? rim : val) * _BrushStrength);
		}
	}
	else
	{
		float dist = distance(vec2(0.5, 0.5), textureUV);
		float t = 1.0 - clamp( dist * 2, 0.0, 1.0);
		t = clamp(t * _BrushOffset, 0.0, 1.0);
		vec3 col2 = vec3(1,0,1);
		color = vec4(_BrushColour, t * _BrushStrength);
		color = mix(color, vec4(1.0 - color.rgb, 0.2), step(dist,0.5) - step(dist,0.45));
	}
}