struct VertexOut
{
	float4 PositionH : SV_POSITION;
	float4 PositionW : POSITION;
	float4 Color : COLOR;
};

float4 main(VertexOut pin) : SV_TARGET
{
	clip(pin.Color.a - 0.1f);
	return pin.Color;
}