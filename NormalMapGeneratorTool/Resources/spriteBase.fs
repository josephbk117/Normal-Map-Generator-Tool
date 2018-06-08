#version 140
in vec2 textureUV;
in vec3 worldPos;
out vec4 color;
uniform sampler2D textureOne;
uniform float _HeightmapStrength;
uniform int _normalMapModeOn;
/*void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord)
{
	float w = 1.0 / 512;
	float h = 1.0 / 512;

	n[0] = texture2D(tex, coord + vec2( -w, -h));
	n[1] = texture2D(tex, coord + vec2(0.0, -h));
	n[2] = texture2D(tex, coord + vec2(  w, -h));
	n[3] = texture2D(tex, coord + vec2( -w, 0.0));
	n[4] = texture2D(tex, coord);
	n[5] = texture2D(tex, coord + vec2(  w, 0.0));
	n[6] = texture2D(tex, coord + vec2( -w, h));
	n[7] = texture2D(tex, coord + vec2(0.0, h));
	n[8] = texture2D(tex, coord + vec2(  w, h));
}
vec4 getSobel(in vec2 uv)
{
    vec4 n[9];
	make_kernel( n, textureOne, uv );

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));
    return 1.0 - sobel;
}*/
void main()
{
    if(_normalMapModeOn == 1 || _normalMapModeOn == 2)
    {
        float _HeightmapDimX = 512;
        float _HeightmapDimY = 512;
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
        vec3 lightDir = normalize(vec3(0.0,0.0,1.0));
        float light = (dot(norm, lightDir) + 1.0) * 0.5;
        vec3 LightReflect = normalize(reflect(lightDir, norm));
        vec3 worldEyePos = worldPos - vec3(0,0.0, - 0.00001);
        float SpecularFactor = dot(worldEyePos, LightReflect);
        if(SpecularFactor > 0)
            SpecularFactor = pow(SpecularFactor, 2);
        if(_normalMapModeOn == 2)
            color = vec4(light,light,light,1.0) + clamp(SpecularFactor,0,1);
        else
            color = vec4(norm,1.0);
    }
    else
    {
        color = texture(textureOne,textureUV);        
    }
}