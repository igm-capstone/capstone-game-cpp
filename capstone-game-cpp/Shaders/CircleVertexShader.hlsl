struct Vertex
{
	float4		mPosition		: POSITION;
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
	pixel.mPositionH = mul(float4(vertex.mPosition.xy, 14.0f, 1.0f), clip);
	pixel.mColor = color * (vertex.mColorswitch+ 1.0f)/2;

	return pixel;
}