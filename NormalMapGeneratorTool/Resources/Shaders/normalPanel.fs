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
uniform float _SpecularStrength;
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
        float currentPx = texture(textureOne,textureUV).x;
        float n = texture(textureOne,vec2(textureUV.x, textureUV.y + 1.0/_HeightmapDimY)).r;
        float s = texture(textureOne,vec2(textureUV.x, textureUV.y - 1.0/_HeightmapDimY)).r;
        float e = texture(textureOne,vec2(textureUV.x - 1.0/_HeightmapDimX, textureUV.y)).r;
        float w = texture(textureOne,vec2(textureUV.x + 1.0/_HeightmapDimX, textureUV.y)).r;

        vec3 norm = normalize(vec3( 0.5, 0.5, 1.0));
        vec3 temp = vec3(1.0, 0, 0);
        //form a basis with norm being one of the axes:
        vec3 perp1 = normalize(cross(norm, temp));
        vec3 perp2 = normalize(cross(norm, perp1));
        //use the basis to move the normal in its own space by the offset
        vec3 normalOffset = -_HeightmapStrength * currentPx * ( ( (n-currentPx) - (s-currentPx) ) * perp1 + ( ( e - currentPx ) - ( w - currentPx ) ) * perp2 );
        norm += normalOffset;
        norm = normalize(norm);
		if(_flipX_Ydir == true)
			norm = norm.grb;
		norm = (2.0 * norm) - 1.0; // from -1.0 to +1.0 range
        float light = max((dot(norm, lightDir) + 1.0) * 0.5 * _LightIntensity, 0.0);
        vec3 LightReflect = normalize(reflect(lightDir, norm));
        vec3 worldEyePos = worldPos - vec3(0.0,0.0,10.0);
        float SpecularFactor = (dot(worldEyePos, LightReflect) + 1.0)*0.5 * _SpecularStrength;

        if(_normalMapModeOn == 2)
		{
			if(SpecularFactor > 0)
				SpecularFactor = pow(SpecularFactor, _Specularity);
			else
				SpecularFactor = 0;
			float gammaCorrected = min(pow(SpecularFactor + light, 1.0/2.4), 1.0);
            color = vec4(gammaCorrected, gammaCorrected, gammaCorrected, 1.0);
		}
        else
		{
            color = vec4((norm+1.0)*0.5,1.0);
			color.rgb *= vec3(_Channel_R, _Channel_G, _Channel_B);
		}
    }
    else
    {
        color = texture(textureOne,textureUV);
    }
}