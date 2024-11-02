//----------------------------------------------------------------------------
//!	@file	gs_model_composite_prev_position.fx
//!	@brief	頂点座標 StreamOut ジオメトリ頂点シェーダー
//----------------------------------------------------------------------------

//---------------------------------------------------------------
// 頂点シェーダー出力(ジオメトリシェーダー入力)
//---------------------------------------------------------------
struct VS_OUTPUT_MODEL
{
	float4	position_       : SV_Position;		//!< 座標       (スクリーン空間)
    float4  curr_position_  : CURR_POSITION;    //!< 現在の座標 (スクリーン空間)
	float3	world_position_ : WORLD_POSITION;	//!< ワールド座標
	float3	normal_         : NORMAL0;			//!< 法線
	float4	diffuse_        : COLOR0;			//!< Diffuseカラー
	float2	uv0_            : TEXCOORD0;		//!< テクスチャ座標
};

struct GS_OUTPUT_MODEL
{
	VS_OUTPUT_MODEL	vs_input;

    //! 1フレーム前の座標 (スクリーン空間) ■これを頂点に合成
    float4	prev_position_ : PREV_POSITION;
};


//----------------------------------------------------------------------------
// ByteAddressBuffer
//----------------------------------------------------------------------------
// 1フレーム前の座標情報
// float4 * 頂点数
ByteAddressBuffer ScreenPosition_Prev : register(t0);

// 1フレーム前の座標を取得
//!	@param	[in]	頂点番号
float4 DxLib_ScreenPosition_Prev(int vertexIndex)
{
	return asfloat(ScreenPosition_Prev.Load4(vertexIndex * 16));
}

//----------------------------------------------------------------------------
//	メイン関数
//----------------------------------------------------------------------------
[maxvertexcount(3)]
void main(triangle VS_OUTPUT_MODEL input[3], uint primitiveId: SV_PrimitiveID,  inout TriangleStream<GS_OUTPUT_MODEL> outputStream)
{
	GS_OUTPUT_MODEL	output;

	output.vs_input       = input[0];
	output.prev_position_ = DxLib_ScreenPosition_Prev(primitiveId * 3 + 0);
	outputStream.Append(output);

	output.vs_input = input[1];
	output.prev_position_ = DxLib_ScreenPosition_Prev(primitiveId * 3 + 1);
	outputStream.Append(output);

	output.vs_input = input[2];
	output.prev_position_ = DxLib_ScreenPosition_Prev(primitiveId * 3 + 2);
	outputStream.Append(output);
}
