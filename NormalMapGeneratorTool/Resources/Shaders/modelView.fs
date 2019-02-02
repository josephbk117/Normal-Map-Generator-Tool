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
uniform vec3 _CameraPosition;
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
uniform int _MethodIndex; // 0 - Method 1, 1 - Method 2

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
vec4 PBR_Colour(vec3 Normal, vec3 camPos, vec3 WorldPos, vec3 albedo, float metallic, float roughness, vec3 lightPositions)
{	
	vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = normalize(lightPositions - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPositions - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = vec3(1,1,1) * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
       
    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * 1.0;//ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    return  vec4(color, 1.0);
}


void main()
{
	if(_normalMapModeOn == 1 || _normalMapModeOn == 2 || _normalMapModeOn == 4)
    {
		float currentPx = texture(inTexture,TexCoords).x;
		float xOffset = 1.0/_HeightmapDimX;
		float yOffset = 1.0/_HeightmapDimY;

        float n = texture(inTexture,vec2(TexCoords.x, TexCoords.y + yOffset)).r;
        float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - yOffset)).r;
        float e = texture(inTexture,vec2(TexCoords.x - xOffset, TexCoords.y)).r;
        float w = texture(inTexture,vec2(TexCoords.x + xOffset, TexCoords.y)).r;

        vec3 norm;

		if(_MethodIndex == 0) //For method 1
		{
			n *= _HeightmapStrength * 0.01;
			s *= _HeightmapStrength * 0.01;
			e *= _HeightmapStrength * 0.01;
			w *= _HeightmapStrength * 0.01;

			yOffset = xOffset = 0.002;//Uniform distance for triangle points
			//Point 1 north
			vec3 point1 = vec3(0, n, 0);
			point1.xz = vec2(TexCoords.x, TexCoords.y + yOffset);
			//Point 2 west
			vec3 point2 = vec3(0, w, 0);
			point2.xz = vec2(TexCoords.x + xOffset, TexCoords.y);
			//Point 3 center
			vec3 point3 = vec3(0, currentPx, 0);
			point3.xz = TexCoords;
			//Point 4 south
			vec3 point4 = vec3(0, s, 0);
			point4.xz = vec2(TexCoords.x, TexCoords.y - yOffset);
			//Point 5 east
			vec3 point5 = vec3(0, e, 0);
			point5.xz = vec2(TexCoords.x - xOffset, TexCoords.y);

			vec3 v1 = point1 - point3;
			vec3 v2 = point2 - point3;

			vec3 v3 = point4 - point3;
			vec3 v4 = point5 - point3;

			vec3 v5 = point5 - point3;
			vec3 v6 = point1 - point3;

			vec3 v7 = point2 - point3;
			vec3 v8 = point4 - point3;

			vec3 frNorm = normalize(cross(v1, v2));
			vec3 secNorm = normalize(cross(v3, v4));
			vec3 thrNorm = normalize(cross(v5, v6));
			vec3 fourNorm = normalize(cross(v7, v8));

			norm = frNorm + secNorm + thrNorm + fourNorm;
			norm.rgb = norm.rbg;
			norm.rg = -norm.rg;
		}
		else //For method 2
		{
			float ne = texture(inTexture,vec2(TexCoords.x - xOffset, TexCoords.y + yOffset)).r;
			float nw = texture(inTexture,vec2(TexCoords.x + xOffset, TexCoords.y + yOffset)).r;
			float se = texture(inTexture,vec2(TexCoords.x - xOffset, TexCoords.y - yOffset)).r;
			float sw = texture(inTexture,vec2(TexCoords.x + xOffset, TexCoords.y - yOffset)).r;
			//           -1 0 1
			//           -2 0 2
			//           -1 0 1
			float dX = nw + 2*w + sw -ne - 2*e - se;
			//           -1-2-1
			//            0 0 0
			//            1 2 1
			float dY = se + 2*s + sw -ne - 2*n - nw;
			dX *= _HeightmapStrength * currentPx;
			dY *= _HeightmapStrength * currentPx;
			norm = vec3(dX, dY, 1.0);
			norm.g = -norm.g;
		}
		norm = normalize(norm);
		if(_flipX_Ydir == true)
			norm = norm.grb;

        if(_normalMapModeOn == 2 || _normalMapModeOn == 4)
		{
			norm = (2.0 * norm) - 1.0;
			vec3 Normal = TBN * Normal;
			//Normal = TBN * Normal;
			norm  = TBN * norm;
			vec3 lightDir = TBN * lightDir;
			vec3 _norm = normalize(Normal * norm);
			vec3 _CameraPosition = TBN * _CameraPosition;
			float lightDot = max(dot( _norm, lightDir), 0.0);

			//Object colour
			vec3 objectColour = diffuseColour;
			if(_normalMapModeOn == 4)
				objectColour *= texture(inTexture2,TexCoords).rgb;

			// diffuse
			vec3 diffuse = lightColour * lightDot * _LightIntensity;

			//Reflection
			vec3 I = normalize(FragPos - vec3(0, 0, -_CameraPosition.z));//_CameraPosition);
			vec3 R = reflect(I, _norm);
			vec3 reflectionCol = textureLod(skybox, R, _Roughness).rgb;
    
			// specular
			vec3 viewDir = normalize(FragPos -vec3(0, 0, -_CameraPosition.z));// _CameraPosition);
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = pow(max(dot(_norm, halfwayDir), 0.0), _Specularity);

			vec3 specular = _SpecularStrength * spec * lightColour;  

			//final colour
			vec3 result = (ambientColour + diffuse + specular) * mix(objectColour, reflectionCol, _Reflectivity);
			result = pow(result, vec3(1.0/2.2));
			FragColor = vec4(result, 1.0);//PBR_Colour(Normal, vec3(0.0, 0.0, -_CameraZoom), FragPos, result, _Reflectivity, _Roughness ,vec3(1,0,2));//
		}
        else
		{
			//norm = TBN * norm;
            FragColor = vec4(norm * 0.5 + 0.5,1.0);
		}
    }
    else if(_normalMapModeOn == 3)
    {
		//vec4 col = texture(inTexture, TexCoords);
		//vec2 parallaxTexCoord = TexCoords;
		//vec3 dirToEye = normalize(vec3(0, 0, _CameraZoom) - FragPos);
		//parallaxTexCoord += (dirToEye * TBN[2]).xy * col.r;
		
        //FragColor = texture(inTexture, parallaxTexCoord);
		FragColor = texture(inTexture, TexCoords);
    }
}