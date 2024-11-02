//----------------------------------------------------------------------------
//!	@file	gs_model_streamout_position.fx
//!	@brief	頂点座標 StreamOut ジオメトリ頂点シェーダー
//----------------------------------------------------------------------------

//---------------------------------------------------------------
// 頂点シェーダー出力(ジオメトリシェーダー入力)
//---------------------------------------------------------------
struct VS_OUTPUT_MODEL
{
	float4	position_       : SV_Position;		//!< 座標       (スクリーン空間)
    float4  curr_position_  : CURR_POSITION;    //!< 現在の座標 (スクリーン空間)
    float4  prev_position_  : PREV_POSITION;    //!< 1フレーム前の座標 (スクリーン空間)
	float3	world_position_ : WORLD_POSITION;	//!< ワールド座標
	float3	normal_         : NORMAL0;			//!< 法線
	float4	diffuse_        : COLOR0;			//!< Diffuseカラー
	float2	uv0_            : TEXCOORD0;		//!< テクスチャ座標
};

struct GS_STREAMOUT
{
	float4 position_ : POSITION;
};

//----------------------------------------------------------------------------
//	メイン関数
//----------------------------------------------------------------------------
[maxvertexcount(3)]
void main(triangle VS_OUTPUT_MODEL input[3], inout TriangleStream<GS_STREAMOUT> outputStream)
{
	GS_STREAMOUT	output;

	output.position_ = input[0].curr_position_;
	outputStream.Append(output);

	output.position_ = input[1].curr_position_;
	outputStream.Append(output);

	output.position_ = input[2].curr_position_;
	outputStream.Append(output);
}
