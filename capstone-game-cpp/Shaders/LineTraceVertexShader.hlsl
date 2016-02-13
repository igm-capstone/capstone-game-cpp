struct Vertex
{
	float4		mColor		: COLOR;
	float3		mPosition	: POSITION;
};

struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float4 mColor		: COLOR;
};

cbuffer transform : register(b0)
{
	matrix projection;
	matrix view;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(view, projection);

	Pixel pixel;
	pixel.mPositionH = mul(float4(vertex.mPosition, 1.0f), clip);
	pixel.mColor = vertex.mColor;

	return pixel;
}