#version 140
in vec2 textureUV;
out vec4 color;
uniform sampler2D textureOne;
uniform float _HeightmapStrength;
uniform int _normalMapModeOn;
uniform int colourFlip;
void main()
{
    if(_normalMapModeOn == 1)
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
        if(norm.x==1) temp.y+=0.5;
        else temp.x+=0.5;
        //form a basis with norm being one of the axes:
        vec3 perp1 = normalize(cross(norm,temp));
        vec3 perp2 = normalize(cross(norm,perp1));
        //use the basis to move the normal in its own space by the offset
        vec3 normalOffset = -_HeightmapStrength * ( ( (n-me) - (s-me) ) * perp1 + ( ( e - me ) - ( w - me ) ) * perp2 );
        norm += normalOffset;
        norm = normalize(norm);
        color = vec4(norm,1.0);
    }
    else
    {
        color = texture(textureOne,textureUV);        
    }
}