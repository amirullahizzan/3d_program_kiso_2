//----------------------------------------------------------------------------
//!	@file	vs_3d.fx
//!	@brief	3Dプリミティブ頂点シェーダー
//----------------------------------------------------------------------------
#include "dxlib_vs.h"

// 頂点シェーダーの出力
struct VS_OUTPUT_3D
{
	float4	position_      : SV_Position;		//!< 座標 (スクリーン空間)
	float3	worldPosition_ : WORLD_POSITION;	//!< ワールド座標
	float3	normal_        : NORMAL0;			//!< 法線
	float4	diffuse_       : COLOR0;			//!< Diffuseカラー
	float2	uv0_           : TEXCOORD0;			//!< テクスチャ座標
};

//----------------------------------------------------------------------------
//	メイン関数
//----------------------------------------------------------------------------
VS_OUTPUT_3D main(VS_INPUT_3D input)
{
	VS_OUTPUT_3D output;

	//----------------------------------------------------------
	// 頂点座標変換
	//----------------------------------------------------------
	float3		localPosition = input.position_;
	float3x4	matWorld = DxLib_Base.WorldMatrix;

	float3	worldPosition = mul(matWorld              , float4(localPosition, 1.0));	// ワールド空間へ変換
	float3	viewPosition  = mul(DxLib_Base.ViewMatrix , float4(worldPosition, 1.0));	// ビュー空間へ変換
	output.position_ = mul(DxLib_Base.ProjectionMatrix, float4(viewPosition , 1.0));	// スクリーン空間へ変換

	output.worldPosition_ = worldPosition;	// ワールド座標

	//----------------------------------------------------------
	// 出力パラメータ
	//----------------------------------------------------------
	output.normal_  = mul(matWorld, float4(input.normal_, 0.0));	// 法線をワールド空間へ変換
	output.diffuse_ = input.diffuse_;								// Diffuseカラー
	output.uv0_     = input.uv0_;									// テクスチャ座標

	//----------------------------------------------------------
	// 出力パラメータを返す
	//----------------------------------------------------------
	return output;
}