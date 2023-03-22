struct VertexOut
{
	float4 PositionH : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	int TextureId : TEXCOORD1;
};

Texture2D g_Texture[5] : register(t0);
SamplerState g_Sampler : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
	const float4 albedo = g_Texture[0].Sample(g_Sampler, pin.TexCoord);
	float4 color = albedo * pin.Color;
	clip(color.a - 0.1f);
	return color;
}