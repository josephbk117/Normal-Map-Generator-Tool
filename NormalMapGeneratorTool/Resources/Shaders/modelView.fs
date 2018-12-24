#version 140
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
uniform sampler2D inTexture;
uniform sampler2D inTexture2;
uniform samplerCube skybox;
uniform vec3 diffuseColour;
uniform vec3 ambientColour;
uniform vec3 lightColour;
uniform vec3 lightDir;
uniform float _CameraZoom;
uniform float _HeightmapStrength;
uniform float _HeightmapDimX;
uniform float _HeightmapDimY;
uniform float _Specularity;
uniform float _SpecularStrength;
uniform float _Roughness;
uniform float _Reflectivity;
uniform float _LightIntensity;
uniform int _normalMapModeOn;
uniform bool _flipX_Ydir;

void main()
{
	 if(_normalMapModeOn == 1 || _normalMapModeOn == 2 || _normalMapModeOn == 4)
    {
		vec3 norm = vec3(0.5,0.5,1.0);
		float currentPx = texture(inTexture,TexCoords).x;
		float n = texture(inTexture,vec2(TexCoords.x,TexCoords.y + 1.0/512.0/*_HeightmapDimY*/)).x;
		float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - 1.0/512.0/*_HeightmapDimY*/)).x;
		float e = texture(inTexture,vec2(TexCoords.x - 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;
		float w = texture(inTexture,vec2(TexCoords.x + 1.0/512.0/*_HeightmapDimX*/, TexCoords.y)).x;

		vec3 temp = norm;
		if(norm.x==1) temp.y += 0.5;
		else temp.x += 0.5;
		temp = normalize(temp);
		//form a basis with norm being one of the axes:
		vec3 perp1 = normalize(cross(norm,temp));
		vec3 perp2 = normalize(cross(norm,perp1));
		//use the basis to move the normal in its own space by the offset
		vec3 normalOffset = -_HeightmapStrength * currentPx * ( ( (n-currentPx) - (s-currentPx) ) * perp1 + ( ( e - currentPx ) - ( w - currentPx ) ) * perp2 );
		norm += normalOffset;
		norm = normalize(norm);
		if(_flipX_Ydir == true)
			norm = norm.grb;
        if(_normalMapModeOn == 2 || _normalMapModeOn == 4)
		{
			norm = (2.0 * norm) - 1.0;
			vec3 _norm = normalize(Normal * norm);
			float lightDot = max(dot( _norm, lightDir), 0.0);

			//Object colour
			vec3 objectColour = diffuseColour;
			if(_normalMapModeOn == 4)
				objectColour *= texture(inTexture2,TexCoords).rgb;

			// diffuse
			vec3 diffuse = lightColour * lightDot * _LightIntensity;

			//Reflection
			vec3 I = normalize(FragPos - vec3(0, 0, _CameraZoom));
			vec3 R = reflect(I, _norm);
			vec3 reflectionCol = textureLod(skybox, R, _Roughness).rgb;
    
			// specular
			vec3 viewDir = normalize(FragPos - vec3( 0, 0, _CameraZoom));
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = pow(max(dot(_norm, halfwayDir), 0.0), _Specularity);

			vec3 specular = _SpecularStrength * spec * lightColour;  

			//final colour
			vec3 result = (ambientColour + diffuse + specular) * mix(objectColour, reflectionCol, _Reflectivity);
			result = pow(result, vec3(1.0/2.2));
			FragColor = vec4(result, 1.0);
		}
        else
		{
            FragColor = vec4(norm,1.0);
		}
    }
    else if(_normalMapModeOn == 3)
    {
        FragColor = texture(inTexture, TexCoords);
    }
}