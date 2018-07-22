#version 140
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
uniform sampler2D inTexture;
uniform vec3 lightDir;
uniform float _HeightmapStrength;
uniform float _HeightmapDimX;
uniform float _HeightmapDimY;
uniform float _Specularity;
uniform float _LightIntensity;
uniform int _normalMapModeOn;
uniform bool _flipX_Ydir;

void main()
{
	 if(_normalMapModeOn == 1 || _normalMapModeOn == 2)
    {
        vec3 normal = vec3(0.5,0.5,1.0);
        float me = texture(inTexture,TexCoords).x;
        float n = texture(inTexture,vec2(TexCoords.x,TexCoords.y + 1.0/512.0/*_HeightmapDimY*/)).x;
        float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - 1.0/512.0/*_HeightmapDimY*/)).x;
        float e = texture(inTexture,vec2(TexCoords.x - 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;
        float w = texture(inTexture,vec2(TexCoords.x + 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;

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
		vec3 _lightDir = normalize(vec3(0.3,0.9,0.4));
        float light = (dot(norm, _lightDir) + 1.0) * 0.5 * _LightIntensity;
        vec3 LightReflect = normalize(reflect(_lightDir, norm));
        vec3 worldEyePos = FragPos - vec3(0,0.0,-3.0);
        float SpecularFactor = (dot(worldEyePos, LightReflect) + 1.0)*0.5 * _Specularity;
        if(SpecularFactor > 0)
           SpecularFactor *= SpecularFactor;
        if(_normalMapModeOn == 2)
		{
            FragColor = (vec4(light,light,light,1.0) + clamp(SpecularFactor,0,1)) * ((dot(Normal, _lightDir)+1.0)*0.5);
			FragColor.a = 1;
		}
        else
		{
            FragColor = vec4(norm,1.0);
		}
    }
    else
    {
        FragColor = texture(inTexture,TexCoords);
    }
}