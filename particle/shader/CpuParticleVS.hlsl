cbuffer PassConstants : register(b0)
{
	float4x4 g_ViewProj;
	float3 g_EyePos;
	float g_Dt;
}

StructuredBuffer<float> g_ParticleData[9] : register(t0);

struct VertexOut
{
	float4 PositionH : SV_Position;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	int TextureId : TEXCOORD1;
};

// 0------ 1
// |     / |
// |    /  |
// |   /   |
// |  /    |
// | /     |
// 2------ 3
// 0, 1, 2, 2, 1, 3

static const float2 offset[] =
{
	float2(-0.5f, +0.5f),
	float2(+0.5f, +0.5f),
	float2(-0.5f, -0.5f),
	float2(+0.5f, -0.5f),
};

static const float2 texCoord[] =
{
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
};

VertexOut main(uint vertexId : SV_VertexID)
{
	VertexOut vout;

	const uint i = vertexId / 4;
	const uint j = vertexId % 4;
	const float3 center = float3(g_ParticleData[0][i], g_ParticleData[1][i], g_ParticleData[2][i]);
	const float4 color = float4(g_ParticleData[3][i], g_ParticleData[4][i], g_ParticleData[5][i], g_ParticleData[6][i]);
	float2 size = float2(g_ParticleData[7][i], g_ParticleData[8][i]);

	const float3 Z = normalize(g_EyePos - center);
	const float3 X = normalize(cross(float3(0.0f, 1.0f, 0.0f), Z));
	const float3 Y = normalize(cross(Z, X));
	const float4 v = float4(center + offset[j].x * size.x * X + offset[j].y * size.y * Y, 1.0f);

	vout.PositionH = mul(v, g_ViewProj);
	vout.TexCoord = texCoord[j];
	vout.Color = color;
	vout.TextureId = i % 5;

	return vout;
}