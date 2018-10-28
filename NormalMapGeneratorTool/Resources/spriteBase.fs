#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform sampler2D textureOne;
uniform vec3 lightDir;
uniform float _HeightmapStrength;
uniform float _HeightmapDimX;
uniform float _HeightmapDimY;
uniform float _Specularity;
uniform float _LightIntensity;
uniform int _normalMapModeOn;
uniform bool _flipX_Ydir;
uniform int _Channel_R;
uniform int _Channel_G;
uniform int _Channel_B;

void main()
{
    if(_normalMapModeOn == 1 || _normalMapModeOn == 2)
    {
        vec3 normal = vec3(0.5,0.5,1.0);
        float me = texture(textureOne,textureUV).x;
        float n = texture(textureOne,vec2(textureUV.x,textureUV.y + 1.0/_HeightmapDimY)).x;
        float s = texture(textureOne,vec2(textureUV.x, textureUV.y - 1.0/_HeightmapDimY)).x;
        float e = texture(textureOne,vec2(textureUV.x - 1.0/_HeightmapDimX, textureUV.y)).x;
        float w = texture(textureOne,vec2(textureUV.x + 1.0/_HeightmapDimX, textureUV.y)).x;

        vec3 norm = normal;
        vec3 temp = norm;
        if(norm.x==1) temp.y += 0.5;
        else temp.x += 0.5;
        //form a basis with norm being one of the axes:
        vec3 perp1 = normalize(cross(norm,temp));
        vec3 perp2 = normalize(cross(norm,perp1));
        //use the basis to move the normal in its own space by the offset
        vec3 normalOffset = -_HeightmapStrength * ( ( (n-me) - (s-me) ) * perp1 + ( ( e - me ) - ( w - me ) ) * perp2 );
        norm += normalOffset;
        norm = normalize(norm);
		if(_flipX_Ydir == true)
			norm = norm.grb;
		norm = (2.0 * norm) - 1.0;
        float light = (dot(norm, lightDir) + 1.0) * 0.5 * _LightIntensity;
        vec3 LightReflect = normalize(reflect(lightDir, norm));
        vec3 worldEyePos = worldPos - vec3(0.0,0.0,10.0);
        float SpecularFactor = (dot(worldEyePos, LightReflect) + 1.0)*0.5 * _Specularity;

        if(_normalMapModeOn == 2)
		{
			if(SpecularFactor > 0)
				SpecularFactor *= SpecularFactor;
			else
				SpecularFactor = 0;
			float gammaCorrected = pow(SpecularFactor + light, 1.0/2.4);
            color = vec4(gammaCorrected, gammaCorrected, gammaCorrected, 1.0);
		}
        else
		{
            color = vec4((norm+1.0)*0.5,1.0);
			color.r *= _Channel_R;
			color.g *= _Channel_G;
			color.b *= _Channel_B;
			//color = vec4(color.r, color.g, 0, 1);
		}
    }
    else
    {
        color = texture(textureOne,textureUV);
    }
}