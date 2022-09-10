cbuffer _
{
	float4x4 mvp;
};

float4 main(float3 pos : Position) : SV_Position
{
	return mul(float4(pos, 1.0f), mvp);
}