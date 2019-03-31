#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform sampler2D textureOne;
uniform sampler2D textureTwo;
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
uniform int _MethodIndex; // 0 - Method 1, 1 - Method 2
uniform int _UseNormalInput; // 1 - true, 0 - false;

// Inputs will be in -1.0 to +1.0 range for blending methods
vec3 blend_rnm(vec3 n1, vec3 n2);
vec3 blend_udn(vec3 n1, vec3 n2);
vec3 blend_pd(vec3 n1, vec3 n2);
vec3 TriSample(sampler2D inTexture ,vec2 TexCoords, float xOffset, float yOffset);
vec3 SobelNormal(sampler2D inTexture, vec2 TexCoords, float xOffset, float yOffset);

void main()
{
	if(_UseNormalInput == 1)
	{
		vec3 n1 = texture(textureOne,textureUV).rgb * 2.0 - 1.0;
		vec3 n2 = texture(textureTwo,textureUV).rgb * 2.0 - 1.0;
		vec3 norm = blend_rnm(n1, n2);
		color = vec4(norm * 0.5 + 0.5, 1.0);
	}
	else
	{
		if(_normalMapModeOn == 1 || _normalMapModeOn == 2)
		{
			float xOffset = 1.0/_HeightmapDimX;
			float yOffset = 1.0/_HeightmapDimY;
			vec3 norm;
			if(_UseNormalInput == 2)
				norm = texture(textureOne,textureUV).rgb*2.0 - 1.0;
			else if(_UseNormalInput == 3)
			{
				if(_MethodIndex == 0) //For method 1
					norm = TriSample(textureOne, textureUV, xOffset, yOffset);
				else //For method 2
					norm = SobelNormal(textureOne, textureUV, xOffset, yOffset);
			}
			norm = normalize(norm);
			if(_flipX_Ydir == true)
				norm = norm.grb;

			if(_normalMapModeOn == 2)
			{
				// diffuse
				float diffuse = max(dot(norm, lightDir) * 0.5 + 0.5, 0.0) * _LightIntensity;
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
}

vec3 blend_rnm(vec3 n1, vec3 n2)
{
	 n1 = n1 * 0.5 + 0.5;
	 n2 = n2 * 0.5 + 0.5;
     n1 = n1*vec3( 2,  2, 2) + vec3(-1, -1,  0);
     n2 = n2*vec3(-2, -2, 2) + vec3( 1,  1, -1);
     return n1*dot(n1, n2)/n1.z - n2;
}
vec3 blend_udn(vec3 n1, vec3 n2)
{
	return normalize(vec3(n1.xy + n2.xy, n1.z));
}
vec3 blend_pd(vec3 n1, vec3 n2)
{
	return normalize(vec3(n1.xy*n2.z + n2.xy*n1.z, n1.z*n2.z));
}
vec3 TriSample(sampler2D inTexture ,vec2 TexCoords, float xOffset, float yOffset)
{
	float currentPx = texture(inTexture,TexCoords).x;

    float n = texture(inTexture,vec2(TexCoords.x, TexCoords.y + yOffset)).r;
    float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - yOffset)).r;
    float e = texture(inTexture,vec2(TexCoords.x - xOffset, TexCoords.y)).r;
    float w = texture(inTexture,vec2(TexCoords.x + xOffset, TexCoords.y)).r;

     vec3 norm;

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

	return norm;
}
vec3 SobelNormal(sampler2D inTexture, vec2 TexCoords, float xOffset, float yOffset)
{
	vec3 norm;
	float currentPx = texture(inTexture,TexCoords).x;
	float n = texture(inTexture,vec2(TexCoords.x, TexCoords.y + yOffset)).r;
    float s = texture(inTexture,vec2(TexCoords.x, TexCoords.y - yOffset)).r;
    float e = texture(inTexture,vec2(TexCoords.x - xOffset, TexCoords.y)).r;
    float w = texture(inTexture,vec2(TexCoords.x + xOffset, TexCoords.y)).r;

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

	return norm;
}