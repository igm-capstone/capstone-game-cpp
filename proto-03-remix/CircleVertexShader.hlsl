struct Vertex
{
	float3		mPosition		: POSITION;
	float4x4	mWorld			: WORLD;
	float		mColorswitch	: BLENDWEIGHT;
	//	uint		mInstanceID	: SV_InstanceID;
};

struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float4 mColor		: COLOR;
};

cbuffer transform : register(b0)
{
	matrix view;
	matrix projection;
	float4 color;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(vertex.mWorld, view), projection);

	Pixel pixel;
	pixel.mPositionH = mul(float4(vertex.mPosition, 1.0f), clip);
	pixel.mColor = color * vertex.mColorswitch;

	return pixel;
}