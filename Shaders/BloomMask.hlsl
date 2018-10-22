#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VOut VS(VIn vIn)
{
    VOut output;

	output.position = float4(vIn.position, 1.0f);
    output.uv = vIn.uv;
    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
	float4 diffuse = DiffuseTexture.Sample(DefaultSampler, pIn.uv);
	float brightness = max(max(diffuse.x, diffuse.y), diffuse.z);
	diffuse.a = 1.0;
	diffuse.xyz = brightness >= 0.8 ? diffuse.xyz : float3(0.0, 0.0, 0.0);
	return diffuse;
}
