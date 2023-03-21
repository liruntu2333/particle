struct VertexOut
{
	float4 PositionH : SV_POSITION;
	float4 PositionW : POSITION;
	float4 Color : COLOR;
};

// 0------ 1
// |     / |
// |    /  |
// |   /   |
// |  /    |
// | /     |
// 2------ 3
// 0, 1, 2, 2, 1, 3

VertexOut main(uint vertexId : SV_VertexID)
{
	VertexOut vout;
	vout.PositionH = float4(0, 0, 0, 1);
	return vout;
}