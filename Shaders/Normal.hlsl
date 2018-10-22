#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
    float2 uv : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION0;
    float4 normal : NORMAL0;
	float4 tangent : TANGENT0;
    float2 uv : TEXCOORD0;
};

VOut VS(VIn vIn)
{
    VOut output;

    // transform input position from model to world space
    output.worldPos = mul(float4(vIn.position, 1.0), c_modelToWorld);
    // transform position from world to projection space
    output.position = mul(output.worldPos, c_viewProj);

    // transform input tangent from model to world space
    output.normal = mul(float4(vIn.normal, 0.0), c_modelToWorld);
	// transform input normal from model to world space
	output.tangent = mul(float4(vIn.tangent, 0.0), c_modelToWorld);

    output.uv = vIn.uv;
    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(DefaultSampler, pIn.uv);

	// Tangent space
	float3 n = normalize(pIn.normal.xyz);
	float3 t = normalize(pIn.tangent.xyz);
	float3 b = cross(pIn.normal, pIn.tangent).xyz;
	b = normalize(b);
	float3x3 TBN = float3x3(t, b, n);
	// Read the normal map
	n = NormalTexture.Sample(DefaultSampler, pIn.uv).xyz;
	n = 2.0 * n - 1.0; // bias the normal into -1 to +1 range
	n = mul(n, TBN); // tranform from tangent space to world space
	n = normalize(n); // make sure it�s normalized

    // do the lighting
    float3 lightColor = c_ambient;
    float3 v = normalize(c_cameraPosition - pIn.worldPos);
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (c_pointLight[i].isEnabled)
        {
            float3 l = c_pointLight[i].position - pIn.worldPos.xyz;
            float dist = length(l);
            if (dist > 0.0)
            {
                l = l / dist;
                float falloff = smoothstep(c_pointLight[i].outerRadius, c_pointLight[i].innerRadius, dist);
                float3 d = falloff * c_pointLight[i].diffuseColor * max(0.0, dot(l, n));
                lightColor += d;

                float3 r = -reflect(l, n);
                float3 s = falloff * c_pointLight[i].specularColor * pow(max(0.0, dot(r, v)), c_pointLight[i].specularPower);
                lightColor += s;
            }
        }
    }

    float4 finalColor = diffuse * float4(lightColor, 1.0);
    return finalColor;
}
