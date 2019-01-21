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
uniform int _MethodIndex; // 0 - Method 1, 1 - Method 2

void main()
{
    if(_normalMapModeOn == 1 || _normalMapModeOn == 2)
    {
        float currentPx = texture(textureOne,textureUV).x;
		float xOffset = 1.0/_HeightmapDimX;
		float yOffset = 1.0/_HeightmapDimY;

        float n = texture(textureOne,vec2(textureUV.x, textureUV.y + yOffset)).r;
        float s = texture(textureOne,vec2(textureUV.x, textureUV.y - yOffset)).r;
        float e = texture(textureOne,vec2(textureUV.x - xOffset, textureUV.y)).r;
        float w = texture(textureOne,vec2(textureUV.x + xOffset, textureUV.y)).r;

        vec3 norm;

		if(_MethodIndex == 0) //For method 1
		{
			n *= _HeightmapStrength * 0.01;
			s *= _HeightmapStrength * 0.01;
			e *= _HeightmapStrength * 0.01;
			w *= _HeightmapStrength * 0.01;
			//Point 1 north
			vec3 point1 = vec3(0, n, 0);
			point1.xz = vec2(textureUV.x, textureUV.y + yOffset);
			//Point 2 west
			vec3 point2 = vec3(0, w, 0);
			point2.xz = vec2(textureUV.x + xOffset, textureUV.y);
			//Point 3 center
			vec3 point3 = vec3(0, currentPx, 0);
			point3.xz = textureUV;
			//Point 4 south
			vec3 point4 = vec3(0, s, 0);
			point4.xz = vec2(textureUV.x, textureUV.y - yOffset);
			//Point 5 east
			vec3 point5 = vec3(0, e, 0);
			point5.xz = vec2(textureUV.x - xOffset, textureUV.y);

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
			float ne = texture(textureOne,vec2(textureUV.x - xOffset, textureUV.y + yOffset)).r;
			float nw = texture(textureOne,vec2(textureUV.x + xOffset, textureUV.y + yOffset)).r;
			float se = texture(textureOne,vec2(textureUV.x - xOffset, textureUV.y - yOffset)).r;
			float sw = texture(textureOne,vec2(textureUV.x + xOffset, textureUV.y - yOffset)).r;
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