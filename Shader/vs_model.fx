//----------------------------------------------------------------------------
//!	@file	vs_model.fx
//!	@brief	DxLib MV1モデル頂点シェーダー
//----------------------------------------------------------------------------
#include "dxlib_vs_model.h"

// 頂点シェーダーの出力
struct VS_OUTPUT_MODEL
{
	float4	position_       : SV_Position;		//!< 座標       (スクリーン空間)
    float4  curr_position_  : CURR_POSITION;    //!< 現在の座標 (スクリーン空間)
	float3	world_position_ : WORLD_POSITION;	//!< ワールド座標
	float3	normal_         : NORMAL0;			//!< 法線
	float4	diffuse_        : COLOR0;			//!< Diffuseカラー
	float2	uv0_            : TEXCOORD0;		//!< テクスチャ座標
};

//----------------------------------------------------------------------------
//	メイン関数
//----------------------------------------------------------------------------
VS_OUTPUT_MODEL main(VS_INPUT_MV1 input)
{
	VS_OUTPUT_MODEL output;

	//----------------------------------------------------------
	// 頂点座標変換
	//----------------------------------------------------------
	float3		localPosition = input.position_;
	float3x4	matWorld      = DxLib_WorldMatrix(input);

	float3  worldPosition = mul(matWorld                , float4(localPosition, 1.0));  // スキニング計算。ワールド空間へ変換
    float3  viewPosition  = mul(DxLib_ViewMatrix()      , float4(worldPosition, 1.0));  // ビュー空間へ変換
    output.position_      = mul(DxLib_ProjectionMatrix(), float4(viewPosition , 1.0));  // スクリーン空間へ変換

	output.world_position_ = worldPosition;

	//------------------------------
	// 現在の座標を別パラメーターで出力
	// (SV_Positionはピクセルシェーダーでは値の意味が変化するため)
	output.curr_position_ = output.position_;

	//----------------------------------------------------------
	// 出力パラメータ
	//----------------------------------------------------------
	output.normal_  = mul(matWorld, float4(input.normal_, 0.0));	// 法線をワールド空間へ変換
	output.diffuse_ = input.diffuse_;								// Diffuseカラー
	output.uv0_     = input.uv0_.xy;								// テクスチャ座標
	
	//----------------------------------------------------------
	// 出力パラメータを返す
	//----------------------------------------------------------
	return output;
}
