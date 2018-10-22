#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION0;
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
	const float offset[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
const float weight[] = {
	0.2270270270, 0.1945945946, 0.1216216216,
	0.0540540541, 0.0162162162
};
float4 diffuse = DiffuseTexture.Sample(DefaultSampler, pIn.uv) * weight[0];
float4 FragmentColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

for (int i = 1; i < 5; i++) {
	FragmentColor +=
		DiffuseTexture.Sample(DefaultSampler, pIn.uv + float2(0.0, offset[i]))*weight[i] +
		DiffuseTexture.Sample(DefaultSampler, pIn.uv - float2(0.0, offset[i]))*weight[i];
}
diffuse += FragmentColor;
return diffuse;
}
