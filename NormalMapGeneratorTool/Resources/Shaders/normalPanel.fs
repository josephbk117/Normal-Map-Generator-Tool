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
        float diffuse = max(dot(norm, lightDir) * 0.5 + 0.5, 0.0) * _LightIntensity;

        if(_normalMapModeOn == 2)
		{
			// specular
			vec3 viewDir = normalize(- vec3( 0, 0, 5.0));
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = pow(max(dot(norm, halfwayDir), 0.0), _Specularity) * _SpecularStrength;
			float gammaCorrected = min(pow(spec + diffuse, 1.0/2.2), 1.0);
            color = vec4(gammaCorrected, gammaCorrected, gammaCorrected, 1.0);
		}
        else
		{
            color = vec4(norm*0.5 + 0.5,1.0);
			color.rgb *= vec3(_Channel_R, _Channel_G, _Channel_B);
		}
    }
    else
    {
        color = texture(textureOne,textureUV);
    }
}