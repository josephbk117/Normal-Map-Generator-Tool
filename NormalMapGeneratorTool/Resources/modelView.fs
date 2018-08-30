#version 140
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
uniform sampler2D inTexture;
uniform vec3 diffuseColour;
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
		vec3 norm = vec3(0.5,0.5,1.0);
		float me = texture(inTexture,TexCoords).x;
		float n = texture(inTexture,vec2(TexCoords.x,TexCoords.y + 1.0/512.0/*_HeightmapDimY*/)).x;
		float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - 1.0/512.0/*_HeightmapDimY*/)).x;
		float e = texture(inTexture,vec2(TexCoords.x - 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;
		float w = texture(inTexture,vec2(TexCoords.x + 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;

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
        if(_normalMapModeOn == 2)
		{
			// diffuse 
			norm = (2.0 * norm)-1.0;
			vec3 _norm = normalize(Normal*norm);
			vec3 _lightDir = normalize(lightDir);
			float diff = max(dot(_norm, _lightDir), 0.0);
			vec3 diffuse = _LightIntensity * (diff * diffuseColour);
    
			// specular
			vec3 viewDir = normalize(vec3(0,0,-2.4) - FragPos);
			vec3 reflectDir = reflect(-_lightDir, _norm);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 20);
			vec3 specular = vec3(1.0,1.0,1.0) * (spec * _Specularity);
        
			vec3 result = vec3(0.14,0.14,0.14) + diffuse + specular;
			FragColor = vec4(result, 1.0);
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